
/*----------------------------------------------------------------------+
 |                                                                      |
 |      kpk.c -- pretty fast KPK endgame table generator                |
 |                                                                      |
 +----------------------------------------------------------------------*/

/*
 *  Copyright (C) 2015, Marcel van Kervinck
 *  All rights reserved
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/*----------------------------------------------------------------------+
 |      Includes                                                        |
 +----------------------------------------------------------------------*/

#include <stdint.h>
#include <stdlib.h>

#include "kpk.h"

/*----------------------------------------------------------------------+
 |      Definitions                                                     |
 +----------------------------------------------------------------------*/

enum { N = a2-a1, S = -N, E = b1-a1, W = -E }; // Derived geometry

#define inPawnZone(square) (rank(square)!=rank1 && rank(square)!=rank8)
#define conflict(wK,wP,bK) (wK==wP || wP==bK || wK==bK)

#define taxi(a,b) (abs(file(a)-file(b)) | abs(rank(a)-rank(b))) // logical-or as max()
#define wInCheck(wK,wP,bK) (taxi(wK,bK)==1)
#define bInCheck(wK,wP,bK) (taxi(wK,bK)==1                  \
                         || (file(wP)!=fileA && wP+N+W==bK) \
                         || (file(wP)!=fileH && wP+N+E==bK))

// Square set macros (no need to adopt these to the specific geometry)
#define bit(i) (1ULL << (i))
#define mask 0x0101010101010101ULL
#define allW(set) ((set) >> 8)
#define allE(set) ((set) << 8)
#define allS(set) (((set) & ~mask) >> 1)
#define allN(set) (((set) << 1) & ~mask)
#define king64(set) (allW(allN(set)) | allN(set) | allE(allN(set)) \
                   | allW(set)                   | allE(set)       \
                   | allW(allS(set)) | allS(set) | allE(allS(set)))

#define arrayLen(a) (sizeof(a) / sizeof((a)[0]))
enum { white, black };
#define wKsquare(ix) ((ix)>>5)
#define wPsquare(ix) square(((ix)>>3)&3, (ix)&7)

/*----------------------------------------------------------------------+
 |      Data                                                            |
 +----------------------------------------------------------------------*/

static uint64_t kpkTable[2][64*32];
static const int kingSteps[] = { N+W, N, N+E, W, E, S+W, S, S+E };

/*----------------------------------------------------------------------+
 |      Functions                                                       |
 +----------------------------------------------------------------------*/

int kpkProbe(int side, int wK, int wP, int bK)
{
        if (!kpkTable[0][0]) kpkGenerate();

        if (file(wP) >= 4) {
                wK = square(7 - file(wK), rank(wK));
                wP = square(7 - file(wP), rank(wP));
                bK = square(7 - file(bK), rank(bK));
        }
        int ix = (wK << 5) + (file(wP) << 3) + rank(wP);
        int bit = (kpkTable[side][ix] >> bK) & 1;
        return (side == white) ? bit : -bit;
}

int kpkGenerate(void)
{
        uint64_t legal[64*32];

        // Prepare won
        for (int ix=0; ix<arrayLen(kpkTable[0]); ix++) {
                int wK = wKsquare(ix), wP = wPsquare(ix);

                // Positions after winning pawn promotion
                if (rank(wP) == rank8 && wK != wP) {
                        uint64_t lost = ~king64(bit(wK)) & ~bit(wK) & ~bit(wP);
                        if (taxi(wK, wP) > 1)
                                lost &= ~king64(bit(wP));
                        kpkTable[black][ix] = lost;
                }

                if (inPawnZone(wP) && wK != wP) {
                        legal[ix] = ~king64(bit(wK));
                        if (file(wP) != fileA) legal[ix] &= ~bit(wP+N+W);
                        if (file(wP) != fileH) legal[ix] &= ~bit(wP+N+E);
                }
        }

        uint64_t changed;
        do {
                // White to move
                for (int ix=0; ix<arrayLen(kpkTable[0]); ix++) {
                        int wK = wKsquare(ix), wP = wPsquare(ix);

                        // King moves
                        uint64_t won = 0;
                        for (int i=0; i<arrayLen(kingSteps); i++) {
                                int to = wK + kingSteps[i];
                                if (taxi(wK, to & 63) == 1 && to != wP)
                                        won |= kpkTable[black][ix+(kingSteps[i]<<5)] & ~king64(bit(to));
                        }
                        // Pawn moves
                        if (wP+N != wK) {
                                won |= kpkTable[black][ix+rank2-rank1] & ~bit(wP+N);
                                if (rank(wP) == rank2 && wP+N+N != wK)
                                        won |= kpkTable[black][ix+rank4-rank2] & ~bit(wP+N) & ~bit(wP+N+N);
                        }
                        kpkTable[white][ix] = won & legal[ix];
                }

                // Black to move
                changed = 0;
                for (int ix=0; ix<arrayLen(kpkTable[0]); ix++) {
                        int wK = wKsquare(ix), wP = wPsquare(ix);

                        uint64_t canDraw = king64(~kpkTable[white][ix]);
                        uint64_t hasMoves = king64(legal[ix]);
                        uint64_t lost = hasMoves & ~canDraw;

                        changed |= kpkTable[black][ix] ^ lost;
                        kpkTable[black][ix] = lost;
                }
        } while (changed);

        return sizeof kpkTable;
}

int kpkSelfCheck(void)
{
        int counts[] = { // As given by Steven J. Edwards (1996)
                163328/2, 168024/2, // legal positions for each side
                124960/2, 97604/2   // nondraw positions for each side
        };
        for (int ix=0; ix<arrayLen(kpkTable[0]); ix++) {
                int wK = wKsquare(ix), wP = wPsquare(ix);
                for (int bK=0; bK<boardSize; bK++)
                        if (inPawnZone(wP) && !conflict(wK, wP, bK)) {
                                counts[0] -= !bInCheck(wK, wP, bK);
                                counts[1] -= !wInCheck(wK, wP, bK);
                                counts[2] -= !bInCheck(wK, wP, bK) & (kpkTable[white][ix] >> bK);
                                counts[3] -= !wInCheck(wK, wP, bK) & (kpkTable[black][ix] >> bK);
                        }
        }
        return !counts[0] && !counts[1] && !counts[2] && !counts[3];
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

