#include "argp.h"

error_t 
argp_parse (const struct argp *_argp, int _argc, char **_argv, unsigned _flags, int *argv, void *_input) 
{
	char *str;
	struct argp_state _argp_state;
	_argp_state.argc = _argc;
	_argp_state.argv = _argv;
	_argp_state.input = _input;
	_argp_state.arg_num = 1;
	_argp_state.root_argp = _argp;

	for (int i = 1; i<_argc; i++) {
		_argp_state.next = i;
		if(_argv[i][0] == '-' && strlen(_argv[i])>1) {
			if(_argv[i][1] == '-') { 
				str = _argv[i]+2;
				int match = 1;
				for (int j=0; _argp->options[j].name != NULL || _argp->options[j].key != 0; j++) {
					if (_argp->options[j].name != NULL && strcmp(str, _argp->options[j].name)==0) {
						_argp->parser( _argp->options[j].key, _argv[i+1], &_argp_state );
						match = 0;
					}
				}
				if (match) argp_usage(&_argp_state, stderr);
			} else {
				int pos = 1;
				while (_argv[i][pos]) _argp->parser( _argv[i][pos++], _argv[i+1], &_argp_state );
			}
		} else {
			_argp->parser( ARGP_KEY_ARG, _argv[i], &_argp_state );
		}
		if (_argp_state.next>i) i = _argp_state.next;
	}
	return 0;
}

void
argp_usage (const struct argp_state *__restrict __state, FILE *s) 
{
	const struct argp *__argp = __state->root_argp;
	fprintf(s, "Usage: %s [-", basename(__state->argv[0]));
	for (int i=0; __argp->options[i].key != 0 || __argp->options[i].name != NULL; i++) 
		if (__argp->options[i].key>' ' && !__argp->options[i].arg) fputc(__argp->options[i].key, s);
	fputs("] ", s);
	for (int i=0; __argp->options[i].key != 0; i++) if (__argp->options[i].key>' ' && __argp->options[i].arg) fprintf(s, "[-%c %s] ", (char) __argp->options[i].key, __argp->options[i].arg);
	for (int i=0; __argp->options[i].name != NULL; i++) {
		if (__argp->options[i].arg) {
			fprintf(s, "[--%s=%s] ", __argp->options[i].name, __argp->options[i].arg);
		} else	fprintf(s, "[--%s] ", __argp->options[i].name);
	}
	fputs(__argp->args_doc, s);
	if (s==stderr) exit(1);
	exit(0);
}

void 
argp_help (const struct argp_state *__restrict __state, FILE *s)
{	
	const struct argp *__argp = __state->root_argp;
	char doc[strlen(__argp->doc)];
	strcpy(doc, __argp->doc);
	if (__state->argv[0][0]=='.'&&__state->argv[0][1]=='/') {
		fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0]+2, __argp->args_doc);
	} else 	fprintf(s, "Usage: %s [OPTIONS...] %s\n", __state->argv[0], __argp->args_doc);
	
	
	char *secondhalf = strchr(doc, '\v');
	if (secondhalf!=NULL) (secondhalf++)[0] = '\0';

	const unsigned char NAMELEN = 50;	
	char key[4] = {'-', 0, ',', 0};
	char name[NAMELEN];

	fputs(doc, s);
	fputc('\n',s);
	for (int i=0; __argp->options[i].name != NULL || __argp->options[i].key != 0; i++) {
		key[0] = '-';
		key[1] = __argp->options[i].key;
		if (key[1]<=' ') key[0] = 0;

		if (__argp->options[i].name != NULL) {
			name[0] = '-';
			name[1] = '-';
			strncpy(name+2, __argp->options[i].name, 25);
		} else	name[0]=0;

		if (__argp->options[i].arg!=NULL) {
			strncat(name, "=", 2);
			strncat(name, __argp->options[i].arg, 25);
		}
		fprintf(s, "%5s %-50s%s\n", key, name, __argp->options[i].doc);	
	}
	fputc('\n',s);

	fputs(secondhalf, s);

	
	exit(0);
}

void
argp_version ()
{
	puts(argp_program_version);
	exit(0);
}
