#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef SH2DASM_H
#define SH2DASM_H

typedef unsigned char UINT8; // unsigned 8 bit
typedef char INT8; // signed 8 bit
typedef unsigned short UINT16; // unsigned 16 bit
typedef short INT16; // signed 16 bit

#if UINT_MAX == 4294967295U // 32bit vs 64bit
typedef unsigned int UINT32; // unsigned 32 bit
typedef int INT32; // signed 32 bit
#else
typedef unsigned long UINT32; // unsigned 32 bit
typedef long INT32; // signed 32 bit
#endif

typedef UINT8 bool;

enum {false, true}; // bool

enum {NODECRYPT, CPS3};

#include "opcodes.h"
#include "cps3dec.h"

struct arguments{
	char **inputs; // files read in
	bool simm; // are we reading simm files in	
	UINT32 pcoffset; // pc offset
	UINT32 fileoffset; // disassembling offset
	UINT32 filelength;
	void *decryption;	
	// void pointer to struct for decryption struct
	char *output; // output for the program
};

#endif // SH2DASM_H
