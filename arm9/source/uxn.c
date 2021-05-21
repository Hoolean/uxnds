#include <stdio.h>
#include "../../include/uxn.h"

#ifndef CPU_ERROR_CHECKING
#define NO_STACK_CHECKS
#endif

/*
Copyright (u) 2021 Devine Lu Linvega
Copyright (u) 2021 Andrew Alderwick

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

/* clang-format off */
static inline void   devpoke8(Device *d, Uint8 a, Uint8 b) { d->dat[a & 0xf] = b; d->talk(d, a & 0x0f, 1); }
static inline Uint8  devpeek8(Device *d, Uint8 a) { d->talk(d, a & 0x0f, 0); return d->dat[a & 0xf];  }
static inline void   devpoke16(Device *d, Uint8 a, Uint16 b) { devpoke8(d, a, b >> 8); devpoke8(d, a + 1, b); }
static inline Uint16 devpeek16(Device *d, Uint16 a) { return (devpeek8(d, a) << 8) + devpeek8(d, a + 1); }
/* clang-format on */

ITCM_ARM_CODE
int
evaluxn(Uxn *u, Uint16 vec)
{
	Uint8 instr;
	u->ram.ptr = vec;
	while(u->ram.ptr) {
		instr = u->ram.dat[u->ram.ptr++];
		switch(instr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
		case 0x00: /* BRK */
		case 0x20: /* BRK2 */
		case 0x40: /* BRKr */
		case 0x60: /* BRK2r */
		case 0x80: /* BRKk */
		case 0xa0: /* BRK2k */
		case 0xc0: /* BRKkr */
		case 0xe0: /* BRK2kr */
		{
			u->ram.ptr = 0;
			break;
		}
		case 0x01: /* LIT */
		case 0x81: /* LITk */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x02: /* NOP */
		case 0x22: /* NOP2 */
		case 0x42: /* NOPr */
		case 0x62: /* NOP2r */
		case 0x82: /* NOPk */
		case 0xa2: /* NOP2k */
		case 0xc2: /* NOPkr */
		case 0xe2: /* NOP2kr */
		{
			(void)u;
			break;
		}
		case 0x03: /* POP */
		{
			u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x04: /* DUP */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = a;
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x05: /* SWP */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = a;
			u->wst.dat[u->wst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x06: /* OVR */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b;
			u->wst.dat[u->wst.ptr - 1] = a;
			u->wst.dat[u->wst.ptr] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x07: /* ROT */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr - 3] = b;
			u->wst.dat[u->wst.ptr - 2] = a;
			u->wst.dat[u->wst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x08: /* EQU */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x09: /* NEQ */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x0a: /* GTH */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x0b: /* LTH */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x0c: /* JMP */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x0d: /* JCN */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x0e: /* JSR */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x0f: /* STH */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x10: /* LDZ */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x11: /* STZ */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x12: /* LDR */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x13: /* STR */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x14: /* LDA */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x15: /* STA */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint8 b = u->wst.dat[u->wst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x16: /* DEI */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x17: /* DEO */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x18: /* ADD */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x19: /* SUB */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1a: /* MUL */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1b: /* DIV */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1c: /* AND */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1d: /* ORA */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1e: /* EOR */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x1f: /* SFT */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 1;
			break;
		}
		case 0x21: /* LIT2 */
		case 0xa1: /* LIT2k */
		{
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x23: /* POP2 */
		{
			(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x24: /* DUP2 */
		{
			u->wst.dat[u->wst.ptr] = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr + 1] = u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x25: /* SWP2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 4];
			Uint8 b = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr - 4] = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr - 3] = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 2] = a;
			u->wst.dat[u->wst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x26: /* OVR2 */
		{
			u->wst.dat[u->wst.ptr] = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr + 1] = u->wst.dat[u->wst.ptr - 3];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x27: /* ROT2 */
		{
			Uint8 f = u->wst.dat[u->wst.ptr - 6];
			Uint8 e = u->wst.dat[u->wst.ptr - 5];
			Uint8 d = u->wst.dat[u->wst.ptr - 4];
			Uint8 c = u->wst.dat[u->wst.ptr - 3];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 6] = d;
			u->wst.dat[u->wst.ptr - 5] = c;
			u->wst.dat[u->wst.ptr - 4] = b;
			u->wst.dat[u->wst.ptr - 3] = a;
			u->wst.dat[u->wst.ptr - 2] = f;
			u->wst.dat[u->wst.ptr - 1] = e;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 6) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x28: /* EQU2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x29: /* NEQ2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x2a: /* GTH2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x2b: /* LTH2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x2c: /* JMP2 */
		{
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x2d: /* JCN2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x2e: /* JSR2 */
		{
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x2f: /* STH2 */
		{
			u->rst.dat[u->rst.ptr] = u->wst.dat[u->wst.ptr - 2];
			u->rst.dat[u->rst.ptr + 1] = u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x30: /* LDZ2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x31: /* STZ2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x32: /* LDR2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x33: /* STR2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x34: /* LDA2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr - 2] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x35: /* STA2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 4;
			break;
		}
		case 0x36: /* DEI2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x37: /* DEO2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 3;
			break;
		}
		case 0x38: /* ADD2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b + a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x39: /* SUB2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b - a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3a: /* MUL2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b * a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3b: /* DIV2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b / a) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3c: /* AND2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d & b;
			u->wst.dat[u->wst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3d: /* ORA2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d | b;
			u->wst.dat[u->wst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3e: /* EOR2 */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr - 4] = d ^ b;
			u->wst.dat[u->wst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x3f: /* SFT2 */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr - 4] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->wst.dat[u->wst.ptr - 3] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			u->wst.ptr -= 2;
			break;
		}
		case 0x41: /* LITr */
		case 0xc1: /* LITkr */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x43: /* POPr */
		{
			u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x44: /* DUPr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = a;
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x45: /* SWPr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = a;
			u->rst.dat[u->rst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x46: /* OVRr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b;
			u->rst.dat[u->rst.ptr - 1] = a;
			u->rst.dat[u->rst.ptr] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x47: /* ROTr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr - 3] = b;
			u->rst.dat[u->rst.ptr - 2] = a;
			u->rst.dat[u->rst.ptr - 1] = c;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x48: /* EQUr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x49: /* NEQr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x4a: /* GTHr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x4b: /* LTHr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x4c: /* JMPr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x4d: /* JCNr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x4e: /* JSRr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x4f: /* STHr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x50: /* LDZr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x51: /* STZr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x52: /* LDRr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x53: /* STRr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x54: /* LDAr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x55: /* STAr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint8 b = u->rst.dat[u->rst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x56: /* DEIr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x57: /* DEOr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x58: /* ADDr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x59: /* SUBr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5a: /* MULr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5b: /* DIVr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5c: /* ANDr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5d: /* ORAr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5e: /* EORr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x5f: /* SFTr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 2] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 1;
			break;
		}
		case 0x61: /* LIT2r */
		case 0xe1: /* LIT2kr */
		{
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr++);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr++);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x63: /* POP2r */
		{
			(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x64: /* DUP2r */
		{
			u->rst.dat[u->rst.ptr] = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr + 1] = u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x65: /* SWP2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 4];
			Uint8 b = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr - 4] = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr - 3] = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 2] = a;
			u->rst.dat[u->rst.ptr - 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x66: /* OVR2r */
		{
			u->rst.dat[u->rst.ptr] = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr + 1] = u->rst.dat[u->rst.ptr - 3];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x67: /* ROT2r */
		{
			Uint8 f = u->rst.dat[u->rst.ptr - 6];
			Uint8 e = u->rst.dat[u->rst.ptr - 5];
			Uint8 d = u->rst.dat[u->rst.ptr - 4];
			Uint8 c = u->rst.dat[u->rst.ptr - 3];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 6] = d;
			u->rst.dat[u->rst.ptr - 5] = c;
			u->rst.dat[u->rst.ptr - 4] = b;
			u->rst.dat[u->rst.ptr - 3] = a;
			u->rst.dat[u->rst.ptr - 2] = f;
			u->rst.dat[u->rst.ptr - 1] = e;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 6) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x68: /* EQU2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x69: /* NEQ2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x6a: /* GTH2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x6b: /* LTH2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x6c: /* JMP2r */
		{
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x6d: /* JCN2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x6e: /* JSR2r */
		{
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x6f: /* STH2r */
		{
			u->wst.dat[u->wst.ptr] = u->rst.dat[u->rst.ptr - 2];
			u->wst.dat[u->wst.ptr + 1] = u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x70: /* LDZ2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x71: /* STZ2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x72: /* LDR2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x73: /* STR2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x74: /* LDA2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr - 2] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr - 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x75: /* STA2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 4;
			break;
		}
		case 0x76: /* DEI2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr - 1] = devpeek8(&u->dev[a >> 4], a);
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x77: /* DEO2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 3;
			break;
		}
		case 0x78: /* ADD2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b + a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x79: /* SUB2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b - a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7a: /* MUL2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b * a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7b: /* DIV2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b / a) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7c: /* AND2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d & b;
			u->rst.dat[u->rst.ptr - 3] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7d: /* ORA2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d | b;
			u->rst.dat[u->rst.ptr - 3] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7e: /* EOR2r */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr - 4] = d ^ b;
			u->rst.dat[u->rst.ptr - 3] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x7f: /* SFT2r */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr - 4] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->rst.dat[u->rst.ptr - 3] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			u->rst.ptr -= 2;
			break;
		}
		case 0x83: /* POPk */
		{
			u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x84: /* DUPk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
			u->wst.dat[u->wst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x85: /* SWPk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = a;
			u->wst.dat[u->wst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0x86: /* OVRk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b;
			u->wst.dat[u->wst.ptr + 1] = a;
			u->wst.dat[u->wst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 252) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 3;
			break;
		}
		case 0x87: /* ROTk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr] = b;
			u->wst.dat[u->wst.ptr + 1] = a;
			u->wst.dat[u->wst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 252) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 3;
			break;
		}
		case 0x88: /* EQUk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x89: /* NEQk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x8a: /* GTHk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x8b: /* LTHk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x8c: /* JMPk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x8d: /* JCNk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			if(u->wst.dat[u->wst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x8e: /* JSRk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0x8f: /* STHk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0x90: /* LDZk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x91: /* STZk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x92: /* LDRk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x93: /* STRk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x94: /* LDAk */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x95: /* STAk */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint8 b = u->wst.dat[u->wst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x96: /* DEIk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x97: /* DEOk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0x98: /* ADDk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x99: /* SUBk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9a: /* MULk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9b: /* DIVk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9c: /* ANDk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9d: /* ORAk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9e: /* EORk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0x9f: /* SFTk */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xa3: /* POP2k */
		{
			(u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xa4: /* DUP2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 2];
			Uint8 b = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
			u->wst.dat[u->wst.ptr + 1] = b;
			u->wst.dat[u->wst.ptr + 2] = a;
			u->wst.dat[u->wst.ptr + 3] = b;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 251) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 4;
			break;
		}
		case 0xa5: /* SWP2k */
		{
			u->wst.dat[u->wst.ptr] = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr + 1] = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr + 2] = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr + 3] = u->wst.dat[u->wst.ptr - 3];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 251) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 4;
			break;
		}
		case 0xa6: /* OVR2k */
		{
			Uint8 d = u->wst.dat[u->wst.ptr - 4];
			Uint8 c = u->wst.dat[u->wst.ptr - 3];
			Uint8 b = u->wst.dat[u->wst.ptr - 2];
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = d;
			u->wst.dat[u->wst.ptr + 1] = c;
			u->wst.dat[u->wst.ptr + 2] = b;
			u->wst.dat[u->wst.ptr + 3] = a;
			u->wst.dat[u->wst.ptr + 4] = d;
			u->wst.dat[u->wst.ptr + 5] = c;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 249) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 6;
			break;
		}
		case 0xa7: /* ROT2k */
		{
			u->wst.dat[u->wst.ptr] = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr + 1] = u->wst.dat[u->wst.ptr - 3];
			u->wst.dat[u->wst.ptr + 2] = u->wst.dat[u->wst.ptr - 2];
			u->wst.dat[u->wst.ptr + 3] = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr + 4] = u->wst.dat[u->wst.ptr - 6];
			u->wst.dat[u->wst.ptr + 5] = u->wst.dat[u->wst.ptr - 5];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 6) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 249) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 6;
			break;
		}
		case 0xa8: /* EQU2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xa9: /* NEQ2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xaa: /* GTH2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xab: /* LTH2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xac: /* JMP2k */
		{
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xad: /* JCN2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			if(u->wst.dat[u->wst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xae: /* JSR2k */
		{
			u->rst.dat[u->rst.ptr] = u->ram.ptr >> 8;
			u->rst.dat[u->rst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xaf: /* STH2k */
		{
			u->rst.dat[u->rst.ptr] = u->wst.dat[u->wst.ptr - 2];
			u->rst.dat[u->rst.ptr + 1] = u->wst.dat[u->wst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xb0: /* LDZ2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xb1: /* STZ2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xb2: /* LDR2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xb3: /* STR2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xb4: /* LDA2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			u->wst.dat[u->wst.ptr] = mempeek8(u->ram.dat, a);
			u->wst.dat[u->wst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 2) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xb5: /* STA2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8));
			Uint16 b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xb6: /* DEI2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			u->wst.dat[u->wst.ptr] = devpeek8(&u->dev[a >> 4], a);
			u->wst.dat[u->wst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 1) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xb7: /* DEO2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1];
			Uint16 b = (u->wst.dat[u->wst.ptr - 2] | (u->wst.dat[u->wst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 3) {
				u->wst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xb8: /* ADD2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b + a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xb9: /* SUB2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b - a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xba: /* MUL2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b * a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xbb: /* DIV2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b / a) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xbc: /* AND2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d & b;
			u->wst.dat[u->wst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xbd: /* ORA2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d | b;
			u->wst.dat[u->wst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xbe: /* EOR2k */
		{
			Uint8 a = u->wst.dat[u->wst.ptr - 1], b = u->wst.dat[u->wst.ptr - 2], c = u->wst.dat[u->wst.ptr - 3], d = u->wst.dat[u->wst.ptr - 4];
			u->wst.dat[u->wst.ptr] = d ^ b;
			u->wst.dat[u->wst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xbf: /* SFT2k */
		{
			Uint16 a = (u->wst.dat[u->wst.ptr - 1] | (u->wst.dat[u->wst.ptr - 2] << 8)), b = (u->wst.dat[u->wst.ptr - 3] | (u->wst.dat[u->wst.ptr - 4] << 8));
			u->wst.dat[u->wst.ptr] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->wst.dat[u->wst.ptr + 1] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr < 4) {
				u->wst.error = 1;
				goto error;
			}
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xc3: /* POPkr */
		{
			u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xc4: /* DUPkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
			u->rst.dat[u->rst.ptr + 1] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xc5: /* SWPkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = a;
			u->rst.dat[u->rst.ptr + 1] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xc6: /* OVRkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b;
			u->rst.dat[u->rst.ptr + 1] = a;
			u->rst.dat[u->rst.ptr + 2] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 252) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 3;
			break;
		}
		case 0xc7: /* ROTkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr] = b;
			u->rst.dat[u->rst.ptr + 1] = a;
			u->rst.dat[u->rst.ptr + 2] = c;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 252) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 3;
			break;
		}
		case 0xc8: /* EQUkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xc9: /* NEQkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xca: /* GTHkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xcb: /* LTHkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xcc: /* JMPkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xcd: /* JCNkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			if(u->rst.dat[u->rst.ptr - 2]) u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xce: /* JSRkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr += (Sint8)a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xcf: /* STHkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->wst.dat[u->wst.ptr] = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 254) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 1;
			break;
		}
		case 0xd0: /* LDZkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xd1: /* STZkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd2: /* LDRkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xd3: /* STRkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			mempoke8(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd4: /* LDAkr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xd5: /* STAkr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint8 b = u->rst.dat[u->rst.ptr - 3];
			mempoke8(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd6: /* DEIkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xd7: /* DEOkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			devpoke8(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xd8: /* ADDkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b + a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xd9: /* SUBkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b - a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xda: /* MULkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b * a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xdb: /* DIVkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b / a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xdc: /* ANDkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xdd: /* ORAkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xde: /* EORkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xdf: /* SFTkr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr] = b >> (a & 0x07) << ((a & 0x70) >> 4);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xe3: /* POP2kr */
		{
			(u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xe4: /* DUP2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 2];
			Uint8 b = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = a;
			u->rst.dat[u->rst.ptr + 1] = b;
			u->rst.dat[u->rst.ptr + 2] = a;
			u->rst.dat[u->rst.ptr + 3] = b;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 251) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 4;
			break;
		}
		case 0xe5: /* SWP2kr */
		{
			u->rst.dat[u->rst.ptr] = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr + 1] = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr + 2] = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr + 3] = u->rst.dat[u->rst.ptr - 3];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 251) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 4;
			break;
		}
		case 0xe6: /* OVR2kr */
		{
			Uint8 d = u->rst.dat[u->rst.ptr - 4];
			Uint8 c = u->rst.dat[u->rst.ptr - 3];
			Uint8 b = u->rst.dat[u->rst.ptr - 2];
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = d;
			u->rst.dat[u->rst.ptr + 1] = c;
			u->rst.dat[u->rst.ptr + 2] = b;
			u->rst.dat[u->rst.ptr + 3] = a;
			u->rst.dat[u->rst.ptr + 4] = d;
			u->rst.dat[u->rst.ptr + 5] = c;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 249) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 6;
			break;
		}
		case 0xe7: /* ROT2kr */
		{
			u->rst.dat[u->rst.ptr] = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr + 1] = u->rst.dat[u->rst.ptr - 3];
			u->rst.dat[u->rst.ptr + 2] = u->rst.dat[u->rst.ptr - 2];
			u->rst.dat[u->rst.ptr + 3] = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr + 4] = u->rst.dat[u->rst.ptr - 6];
			u->rst.dat[u->rst.ptr + 5] = u->rst.dat[u->rst.ptr - 5];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 6) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 249) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 6;
			break;
		}
		case 0xe8: /* EQU2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b == a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xe9: /* NEQ2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b != a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xea: /* GTH2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b > a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xeb: /* LTH2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = b < a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 254) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 1;
			break;
		}
		case 0xec: /* JMP2kr */
		{
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xed: /* JCN2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			if(u->rst.dat[u->rst.ptr - 3]) u->ram.ptr = a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xee: /* JSR2kr */
		{
			u->wst.dat[u->wst.ptr] = u->ram.ptr >> 8;
			u->wst.dat[u->wst.ptr + 1] = u->ram.ptr & 0xff;
			u->ram.ptr = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xef: /* STH2kr */
		{
			u->wst.dat[u->wst.ptr] = u->rst.dat[u->rst.ptr - 2];
			u->wst.dat[u->wst.ptr + 1] = u->rst.dat[u->rst.ptr - 1];
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
#endif
#ifndef NO_STACK_CHECKS
			if(u->wst.ptr > 253) {
				u->wst.error = 2;
				goto error;
			}
#endif
			u->wst.ptr += 2;
			break;
		}
		case 0xf0: /* LDZ2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xf1: /* STZ2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf2: /* LDR2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, u->ram.ptr + (Sint8)a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xf3: /* STR2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			mempoke16(u->ram.dat, u->ram.ptr + (Sint8)a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf4: /* LDA2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			u->rst.dat[u->rst.ptr] = mempeek8(u->ram.dat, a);
			u->rst.dat[u->rst.ptr + 1] = mempeek8(u->ram.dat, a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 2) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xf5: /* STA2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8));
			Uint16 b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			mempoke16(u->ram.dat, a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf6: /* DEI2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			u->rst.dat[u->rst.ptr] = devpeek8(&u->dev[a >> 4], a);
			u->rst.dat[u->rst.ptr + 1] = devpeek8(&u->dev[a >> 4], a + 1);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 1) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xf7: /* DEO2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1];
			Uint16 b = (u->rst.dat[u->rst.ptr - 2] | (u->rst.dat[u->rst.ptr - 3] << 8));
			devpoke16(&u->dev[a >> 4], a, b);
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 3) {
				u->rst.error = 1;
				goto error;
			}
#endif
			break;
		}
		case 0xf8: /* ADD2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b + a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b + a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xf9: /* SUB2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b - a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b - a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xfa: /* MUL2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b * a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b * a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xfb: /* DIV2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b / a) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b / a) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xfc: /* AND2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d & b;
			u->rst.dat[u->rst.ptr + 1] = c & a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xfd: /* ORA2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d | b;
			u->rst.dat[u->rst.ptr + 1] = c | a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xfe: /* EOR2kr */
		{
			Uint8 a = u->rst.dat[u->rst.ptr - 1], b = u->rst.dat[u->rst.ptr - 2], c = u->rst.dat[u->rst.ptr - 3], d = u->rst.dat[u->rst.ptr - 4];
			u->rst.dat[u->rst.ptr] = d ^ b;
			u->rst.dat[u->rst.ptr + 1] = c ^ a;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
		case 0xff: /* SFT2kr */
		{
			Uint16 a = (u->rst.dat[u->rst.ptr - 1] | (u->rst.dat[u->rst.ptr - 2] << 8)), b = (u->rst.dat[u->rst.ptr - 3] | (u->rst.dat[u->rst.ptr - 4] << 8));
			u->rst.dat[u->rst.ptr] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) >> 8;
			u->rst.dat[u->rst.ptr + 1] = (b >> (a & 0x000f) << ((a & 0x00f0) >> 4)) & 0xff;
#ifndef NO_STACK_CHECKS
			if(u->rst.ptr < 4) {
				u->rst.error = 1;
				goto error;
			}
			if(u->rst.ptr > 253) {
				u->rst.error = 2;
				goto error;
			}
#endif
			u->rst.ptr += 2;
			break;
		}
#pragma GCC diagnostic pop
		}
	}
	return 1;
#ifndef NO_STACK_CHECKS
error:
	dprintf("Halted: %s-stack %sflow#%04x, at 0x%04x\n",
		u->wst.error ? "Working" : "Return",
		((u->wst.error | u->rst.error) & 2) ? "over" : "under",
		instr,
		u->ram.ptr);
	return 0;
#endif
}

int
bootuxn(Uxn *u)
{
	memset(u, 0, sizeof(*u));
	u->ram.dat = malloc(65536);
	memset(u->ram.dat, 0, 65536);
	return 1;
}

int
loaduxn(Uxn *u, char *filepath)
{
	FILE *f;
	if(!(f = fopen(filepath, "rb"))) {
		dprintf("Halted: Missing input rom.\n");
		return 0;
	}
	fread(u->ram.dat + PAGE_PROGRAM, sizeof(u->ram.dat) - PAGE_PROGRAM, 1, f);
	dprintf("Uxn loaded[%s].\n", filepath);
	return 1;
}

Device *
portuxn(Uxn *u, Uint8 id, char *name, void (*talkfn)(Device *d, Uint8 b0, Uint8 w))
{
	Device *d = &u->dev[id];
	d->addr = id * 0x10;
	d->u = u;
	d->mem = u->ram.dat;
	d->talk = talkfn;
	dprintf("Device added #%02x: %s, at 0x%04x \n", id, name, d->addr);
	return d;
}
