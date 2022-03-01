/*-
 * Copyright (c) 2017, 2019 Taylor R. Campbell
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Gimli: A 384-bit permutation out of which to make dwarven sponges.
 *
 *	Daniel J. Bernstein, Stefan Kölbl, Stefan Lucks, Pedro Maat
 *	Costa Massolino, Florian Mendel, Kashif Nawaz, Tobias
 *	Schneider, Peter Schwabe, François-Xavier Standaert, Yosuke
 *	Todo, and Benoît Viguier.  `Gimli: a cross-platform
 *	permutation.'  Cryptographic Hardware and Embedded Systems --
 *	CHES 2017, edited by Wieland Fischer, Naofumi Homma.
 *
 *	Available as IACR Cryptology ePrint Archive: Report 2017/630:
 *	<https://eprint.iacr.org/2017/630>.
 *
 * Portable C implementation -- more portable, and perhaps marginally
 * faster, than the reference implementation provided.  Does not quite
 * match the paper, but does match the reference implementation.
 */

/* Gimli permutation */

#include <stddef.h>
#include <stdint.h>

#define	secret	/* secret */

static uint32_t
rol32(secret uint32_t x, unsigned c)
{

	return (x << c) | (x >> (32 - c));
}

static inline void
swap32(secret uint32_t *p, secret uint32_t *q)
{
	const uint32_t t = *p;

	*p = *q;
	*q = t;
}

static inline void
gimli_round(secret uint32_t *S0, secret uint32_t *S4, secret uint32_t *S8)
{
	const uint32_t x = rol32(*S0, 24);
	const uint32_t y = rol32(*S4, 9);
	const uint32_t z =       *S8;

	*S8 = x ^ (z << 1) ^ ((y & z) << 2);
	*S4 = y ^  x       ^ ((x | z) << 1);
	*S0 = z ^  y       ^ ((x & y) << 3);
}

void
gimli(secret uint32_t S[12])
{
	secret uint32_t S0 = S[0], S1 = S[1], S2 = S[2], S3 = S[3];
	secret uint32_t S4 = S[4], S5 = S[5], S6 = S[6], S7 = S[7];
	secret uint32_t S8 = S[8], S9 = S[9], S10 = S[10], S11 = S[11];
	unsigned r;

	for (r = 24; r > 0; r--) {
		gimli_round(&S0, &S4, &S8);
		gimli_round(&S1, &S5, &S9);
		gimli_round(&S2, &S6, &S10);
		gimli_round(&S3, &S7, &S11);

		if ((r & 3) == 0) {
			swap32(&S0, &S1);
			swap32(&S2, &S3);
		} else if ((r & 3) == 2) {
			swap32(&S0, &S2);
			swap32(&S1, &S3);
		}

		if ((r & 3) == 0)
			S0 ^= UINT32_C(0x9e377900) ^ r;
	}

	S[0] = S0; S[1] = S1; S[2] = S2; S[3] = S3;
	S[4] = S4; S[5] = S5; S[6] = S6; S[7] = S7;
	S[8] = S8; S[9] = S9; S[10] = S10; S[11] = S11;
}
