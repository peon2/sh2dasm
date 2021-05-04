// code taken from the MAME project

#include "cps3dec.h"

inline void
swp(UINT8 *a, UINT8 *b)
{
	*a^=*b;
	*b^=*a;
	*a^=*b;
}

static UINT16
lrot(UINT16 v, UINT8 n)
{
	UINT32 a = v>>(16-n);
	return ((v<<n)|a)%0x10000;
}

static UINT16
rotxor(UINT16 v, UINT16 x)
{
	UINT16 r = v+lrot(v,2);
	return lrot(r,4)^(r&(v^x));
}

static UINT16
cps3mask(UINT32 addr, UINT32 k1, UINT32 k2)
{
	UINT16 v;

	addr ^= k1;
	v = (addr&0xffff)^0xffff;
	v = rotxor(v, k2&0xffff);
	v ^= (addr>>16)^0xffff;
	v = rotxor(v, k2>>16);
	v ^= (addr&0xffff)^(k2&0xffff);
	return v;
}

UINT8
cps3decrypt(struct cps3decryption *cpdec)
{

	if (cpdec->type != CPS3) return 0;

	if (cpdec->gameid != NULL) {
		if (*cpdec->gameid >= CPS3KEYSLEN/2) return 0; // error
		cpdec->k1 = cps3keys[*cpdec->gameid*2];
		cpdec->k2 = cps3keys[1+*cpdec->gameid*2];
	}

	UINT16 mask = cps3mask(*cpdec->addr, cpdec->k1, cpdec->k2); // generate mask
	
	cpdec->val[0]^=mask; // apply mask
	cpdec->val[1]^=mask; // apply mask
	return 1;
}
