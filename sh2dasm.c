#include "sh2dasm.h"
#include "argp.h"

// TODO exit codes

const char *argp_program_version = "<SuperH-2 Disassembler v1.0>";

static char args_doc[] = "[FILE ...]\n";

static char doc[] = "Disassembles SuperH-2 binaries.\nReads in as many binaries as given and outputs as assembly sequentially.\n\vDisassembles using Mame sh2dasm, primarily used for the cps3 arcade system.\nIf - is given for input, reads stdin. If - is given for output, print to stdout.\n";

static struct argp_option options[] = {
	{"simm", 's', 0, 0, "Expects input to be in a simm format, meaning groups of 4."},
	{"pcOffset", 'p', "ADDRESS", 0, "Changes the program counter to start from this address. Otherwise, no effect on disassembly. (Hexadecimal)"},
	{"fileOffset", 'g', "OFFSET", 0, "Start reading a file OFFSET bytes ahead of the start. (Hexadecimal)"},
	{"range", 'r', "RANGE", 0, "How much of a file to read in. (HEXADECIMAL)"},
	{"decryption", 'd', "ENCRYPTION ARGUMENTS...", 0, "Decrypt the file as its read in then disassemble. Takes an encryption and option arguments."},
	{"output", 'o', "FILE", 0, "Outputs the disassembly to specified FILE"},
	{0, 1, 0, 0, ""},
	{"help", '?', 0, 0, "Give this help list"},
	{"usage", ARGP_HELP_USAGE, 0, 0, "Give a short usage message"},
	{"version", 'V', 0, 0, "Print program version"},
	{ 0 }
};

#define ERR_OUT(A) {fputs(A, stderr); argp_usage(state, stderr);}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;
	char *endptr, message[30];

	switch(key) {
		case ARGP_HELP_USAGE:
			argp_usage(state, stdout);
			break;
		case 'V':
			argp_version();
			break;
		case '?':
			argp_help(state, stdout);
			break;
		case 's':
			arguments->simm = true;
			break;
		case 'p': 
			arguments->pcoffset = strtol(arg, &endptr, 16);
			if (*endptr) ERR_OUT("Bad PC Offset read.\n"); // needs to parse proper
			state->next++;
			break;
		case 'g':
			arguments->fileoffset = strtol(arg, &endptr, 16);
			if (*endptr) ERR_OUT("Bad File Offset read.\n"); // needs to parse proper
			state->next++;
			break;
		case 'r':
			arguments->filelength = strtol(arg, &endptr, 16);
			if (*endptr) ERR_OUT("Bad Length read.\n"); // needs to parse proper
			state->next++;
			break;
		case 'd':
			if (!strcmp(arg, "none")) { 
				UINT8 *decryption = malloc(sizeof(UINT8));
				*decryption = NODECRYPT;
				arguments->decryption = decryption;
				state->next++;
				break;
			}

			if (!strcmp(arg, "cps3")) { // cps3 rom
				struct cps3decryption *decryption = malloc(sizeof(struct cps3decryption));
				arguments->decryption = decryption;
				decryption->type = CPS3;
				state->next+=2; // inc counter
				char *k1 = state->argv[state->next];
				for (int i = 0; i < CPS3ROMMAPLEN; i++) // try to match
					if (!strcmp(k1, cps3rommap[i].name)) {
						// found a match, don't need to read keys
						decryption->gameid = &cps3rommap[i].id;
						break;
					}

				if (decryption->gameid==NULL) { // no match was found
					// if no match to found check if the next two arguments are valid keys (UINT32 strings with no letters)
					// if not throw error
					char *endptr;
					decryption->k1 = strtol(k1, &endptr, 16);
					if (!*endptr) {
						char *k2 = state->argv[++state->next];
						decryption->k2 = strtol(k2, &endptr, 16);
						if (!*endptr) break; // good parse
					} 

					free(decryption);
					decryption = NULL;
					fputs("CPS3 decryption takes one of these games:\n", stderr);
					for (int i = 0; i < CPS3ROMMAPLEN; i++) {
						fputs(cps3rommap[i].name, stderr);
						fputc(' ', stderr);
					}
					fputs("\nOr a pair of 32-bit hexadecimal keys.\n", stderr);
					exit(1);
				}
			}
			
			if (arguments->decryption==NULL) {
				fputs("-d currently only supports:\nnone cps3", stderr);
				exit(1);
			}
			break;
		case 'o':
			arguments->output = arg;
			state->next++;
			break;

		case ARGP_KEY_ARG:
			arguments->inputs[state->arg_num-1] = arg;
			state->arg_num++;
			break;
		default:
			strcpy(message, "Bad argument: ");
			#define MESSAGE_LEN strlen("Bad argument: ")
			if (arg!=NULL) {
				strncat(message, arg, 28-MESSAGE_LEN);
			} else {
				message[MESSAGE_LEN] = '-';
				message[MESSAGE_LEN+1] = key;
			}
			#undef MESSAGE_LEN
			strncat(message, "\n", 2);
			ERR_OUT(message);
			break;
		}
	return 0;
}

#undef ERR_OUT

static struct argp argp = {options, parse_opt, args_doc, doc};

//TODO if an argument is given and its not a file and a valid number, disassemble that?

static UINT32
fileLength(FILE *fp, UINT32 offset)
{
	if (fp==NULL) return 0;
	fseek(fp, 0, SEEK_END);
	UINT32 file_size = ftell(fp);
	fseek(fp, offset, SEEK_SET);
	file_size-=ftell(fp); // relative length
	return file_size;
}

static inline void
blockRead(FILE *fp, UINT8 *buffer, UINT8 blocksize)
{
	for (int i = 0; i<blocksize; i++) buffer[i] = fgetc(fp);
}


static void
opcodePrint(UINT16 *opbuffer, UINT8 op_buffer_size, UINT32 *pc, FILE *out)
{
	#define BUFF_SIZE 30
	char buffer[BUFF_SIZE];
	for (int j = 0; j<op_buffer_size; j++) { // increment through the buffer of opcodes in sets of 2 (SH-2 opcode size)
		memset(buffer, 0, BUFF_SIZE);
		DasmSH2(buffer, *pc, opbuffer[j]);
		fprintf(out,"0x%lx: \t%-23s %04x\n", *pc, buffer, opbuffer[j]);
		*pc+=2;
	}
	#undef BUFF_SIZE
}

int 
main(int argc, char **argv)
{
	struct arguments arguments;

	char *inputs[argc-1];
	for (int i = 0; i < argc-1; i++) inputs[i] = NULL;

	arguments.inputs = inputs;
	arguments.simm = false;
	arguments.pcoffset = 0;
	arguments.fileoffset = 0;
	arguments.filelength = ((UINT32) -1)/2;
	arguments.decryption = NULL;
	arguments.output = NULL;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	if (inputs[0]==NULL) exit(0); // nothing to do

	UINT8 op_buffer_size = 1, read_in_buffer_size; // how many words are handled at a time and how many words are read from a file at a time

	if (arguments.simm) 
		op_buffer_size = 2; // need to read in sets of 2 for simm format
	

	UINT8 decryption = NODECRYPT;
	if (arguments.decryption != NULL) {
		// figure out what kind of decryption it is
		decryption = *(UINT8 *) arguments.decryption;
		switch (decryption) {
			//decryption (cps3) sets of 2
			case CPS3:
				op_buffer_size = 2;
				break;
		}
	}


	FILE *fp, *out, *f2, *f3, *f4;
	UINT32 file_size; 
	UINT32 pc = (arguments.pcoffset+arguments.fileoffset>0?arguments.pcoffset+arguments.fileoffset:0); // should be positive or 0
	UINT32 filelength = arguments.filelength+pc;

	UINT16 opbuffer[op_buffer_size];

	switch (decryption) {
		case CPS3:
			((struct cps3decryption*)arguments.decryption)->val = opbuffer; // sets up pointer to values
			((struct cps3decryption*)arguments.decryption)->addr = &pc; // sets up pointer to values
	}

	if (arguments.output!=NULL && strcmp(arguments.output, "-")) {
		out = fopen(arguments.output, "w");
	} else	out = stdout;


	read_in_buffer_size = arguments.simm?op_buffer_size/2:op_buffer_size;

	for (int i = 0; i<argc-1 && inputs[i] != NULL; i++) {
		
		// - implentation if file_size is 0
		// fix reading in from stdin, linebreaks, read in as hex not ascii etc.

		if (!strcmp(inputs[i], "-")) {
			char buff[op_buffer_size*4], buff2[4]; // 4 characters per UINT16 in hex
			char c;

			UINT32 index = 0;
			while ((c=getchar())!=EOF && c!='q') {
				if (isspace(c) || !isxdigit(c)) continue;
				buff[index%(op_buffer_size*4)] = c;
				if (index%(op_buffer_size*4)==(op_buffer_size*4)-1) {
					for (int i=0; i<op_buffer_size; i++) {
						buff2[0] = buff[i*2+0];
						buff2[1] = buff[i*2+1];
						buff2[2] = buff[i*2+2];
						buff2[3] = buff[i*2+3];
						opbuffer[i] = strtol(buff2, NULL, 16);
					}
					
					switch (decryption) {
						case CPS3: // need 4 bytes + address + xorkeys to decrypt
							cps3decrypt((struct cps3decryption*)arguments.decryption);
					}

					opcodePrint(opbuffer, op_buffer_size, &pc, out);
				}
				index++;
			}
			continue;
		}
	
		#define ERR_FP(A) {fputs("Bad read on file ", stderr);fputs(A,stderr);fputs(" make sure the file exists and the size is correct.\n", stderr);exit(1);}
		fp = fopen(inputs[i], "rb");
		file_size = fileLength(fp, arguments.fileoffset);
		if (file_size==0) {
			// check if its a num
			
			UINT16 opcode;
			char *endptr;
			opcode = strtol(inputs[i], &endptr, 16);
			if (*endptr) ERR_FP(inputs[i]);
			opbuffer[0] = opcode;

			switch (decryption) {
				case CPS3: // need 4 bytes + address + xorkeys to decrypt
					cps3decrypt((struct cps3decryption*)arguments.decryption);
			}

			opcodePrint(opbuffer, op_buffer_size, &pc, out);
			if (pc >= filelength) {
				i+=argc;
				break;
			}

			pc+=2;
			
			continue;
		}
		if (file_size%read_in_buffer_size) ERR_FP(inputs[i]); // test file size against buffer size
		
		if (arguments.simm) {
			f2 = fopen(inputs[i+1], "rb");
			if (fileLength(f2, arguments.fileoffset)!=file_size) ERR_FP(inputs[i+1]);
			f3 = fopen(inputs[i+2], "rb");
			if (fileLength(f3, arguments.fileoffset)!=file_size) ERR_FP(inputs[i+2]);
			f4 = fopen(inputs[i+3], "rb");
			if (fileLength(f4, arguments.fileoffset)!=file_size) ERR_FP(inputs[i+3]);
		}
		#undef ERR_FP

		for (int _ = 0; _<file_size/read_in_buffer_size; _++) {
			if (arguments.simm) { // arrange in simm format
				for (int j = 0; j<op_buffer_size; j+=2) { // reads in 4 bytes at a time
					blockRead(fp, (UINT8 *)(j+opbuffer)+1, read_in_buffer_size);
					blockRead(f2, (UINT8 *)(j+opbuffer)+0, read_in_buffer_size);
					blockRead(f3, (UINT8 *)(j+opbuffer)+3, read_in_buffer_size);
					blockRead(f4, (UINT8 *)(j+opbuffer)+2, read_in_buffer_size); // fill in the buffer
				}
			} else {
				blockRead(fp, (UINT8 *)opbuffer, read_in_buffer_size*2); // read it normally
				for (int j = 0; j < read_in_buffer_size; j++) swp((UINT8 *)opbuffer+j*2, 1+(UINT8 *)opbuffer+j*2);
			}

			// if decrypting decrypt here. 
			switch (decryption) {
				case CPS3: // need 4 bytes + address + xorkeys to decrypt
					cps3decrypt((struct cps3decryption*)arguments.decryption);
			}

			opcodePrint(opbuffer, op_buffer_size, &pc, out);
			if (pc >= filelength) {
				i+=argc;
				break;
			}
		}
		
		fclose(fp);

		if (arguments.simm) {
			fclose(f2);
			fclose(f3);
			fclose(f4);
			i+=3;
		}
	}
	if (out!=stdout) fclose(out);

	if (arguments.decryption != NULL) free(arguments.decryption);

	return 0;
}
