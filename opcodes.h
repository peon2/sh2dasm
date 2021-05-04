#ifndef OPCODES_H

#define OPCODES_H

#include "sh2dasm.h"


#define SIGNX8(x)	(((INT32)(x) << 24) >> 24)
#define SIGNX12(x)	(((INT32)(x) << 20) >> 20)

#define Rn ((opcode >> 8) & 15)
#define Rm ((opcode >> 4) & 15)

static const char *const regname[16] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
	"R8", "R9", "R10","R11","R12","R13","R14","SP"
};

UINT32 DasmSH2(char *buffer, unsigned pc, UINT16 opcode);

#endif // OPCODES_H
