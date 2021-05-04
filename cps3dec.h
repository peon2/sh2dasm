#ifndef CPS3DEC_H

#define CPS3DEC_H

#include "sh2dasm.h"

enum {REDEARTH, SFIII, SFIII2, JOJO, SFIII3, JOJOBA}; // cps3 roms

struct {
	const char name[sizeof("redearthr1")];
	const UINT8 id;
	// add a bool for being a simm rom to force the flag being set?
} static const cps3rommap[] =
{
	{"redearth", REDEARTH},
	{"redearthr1", REDEARTH},
	{"redeartn", REDEARTH},
	{"warzard", REDEARTH},
	{"warzardr1", REDEARTH},
	{"warzardn", REDEARTH},
	{"sfiii", SFIII},
	{"sfiiia", SFIII},
	{"sfiiin", SFIII},
	{"sfiiina", SFIII},
	{"sfiiih", SFIII},
	{"sfiiij", SFIII},
	{"sfiiiu", SFIII},
	{"sfiii2", SFIII2},
	{"sfiii2n", SFIII2},
	{"sfiii2j", SFIII2},
	{"jojoaltn", JOJO},
	{"jojon", JOJO},
	{"jojonr1", JOJO},
	{"jojonr2", JOJO},
	{"sfiii3", SFIII3},
	{"sfiii3r1", SFIII3},
	{"sfiii3jr1", SFIII3},
	{"sfiii3nr1", SFIII3},
	{"sfiii3j", SFIII3},
	{"sfiii3a", SFIII3},
	{"sfiii3an", SFIII3},
	{"sfiii3n", SFIII3},
	{"sfiii3nr1", SFIII3},
	{"sfiii3u", SFIII3},
	{"sfiii3ur1", SFIII3},
	{"sfiii4n", SFIII3},
	{"jojoba", JOJOBA},
	{"jojoban", JOJOBA},
	{"jojobane", JOJOBA},
	{"jojobanr1", JOJOBA},
	{"jojobaner1", JOJOBA},
};

#define CPS3ROMMAPLEN (sizeof(cps3rommap)/sizeof(cps3rommap[0]))

static const UINT32 cps3keys[] =
{
	0x9E300AB1, 0xA175B82C, // red earth
	0xB5FE053E, 0xFC03925A, // sfiii:NG
	0x00000000, 0x00000000, // sfiii:2i
	0x02203EE3, 0x01301972, // jojo
	0xA55432B4, 0x0C129981, // sfiii:3s
	0x23323EE3, 0x03021972  // jojo HFTF
};

#define CPS3KEYSLEN (sizeof(cps3keys)/sizeof(UINT32))

struct cps3decryption {
	UINT8 type; // type of decryption, should always be set to CPS3
	UINT32 *addr; // address is needed to decrypt
	UINT16 *val; // value to decrypt, cps3 works in 32 bit
	const UINT8 *gameid; 
	UINT32 k1;
	UINT32 k2; // either the gameid must be give or both keys must be given
};

UINT8 cps3decrypt(struct cps3decryption *cpdec);
void swp(UINT8 *a, UINT8 *b);

#endif // CPS3DEC_H
