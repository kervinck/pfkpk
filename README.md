Pretty fast KPK endgame table generator
=======================================

Solve the endgame with King and Pawn vs. King and make the result
available through a probing function call interface. For use in
chess programs and provided under a 2-clause BSD-style license. See
the file `LICENSE' for details.

This implementation is quite fast because it handles the 64 possible
black king locations superposition in all operations.

```
kpkGenerate CPU time [seconds]: 0.001579
kpkTable size [bytes]: 32768
kpkSelfCheck: OK
kpkProbe 24/24: OK
```

The implementation is agnostic to board geometry.  See the file
`kpk.h' for instructions on how to adapt to an alternative square
indexing.

Distance to conversion (DTC) is not provided, but is trivial to
add.  Please contact the author if you need advice.

Marcel

```
/*
 *  Probe a KPK position from the in memory endgame table.
 *  Returns 0 for draw, 1 for win and -1 for loss.
 *
 *  The position must be legal for meaningful results.
 *  `side' is 0 for white to move and 1 for black to move.
 *
 *  If the table is not generated yet, this will be done
 *  automatically at the first invocation.
 */
int kpkProbe(int side, int wK, int wP, int bK);

/*
 *  Generate the KPK table.
 *  Returns the memory size for info.
 *  This can take upto 2 milliseconds on a 2.6GHz Intel i7.
 */
int kpkGenerate(void);

/*
 *  Perform a self check on the bitbase.
 *  Returns 0 on failure, 1 for success.
 */
int kpkSelfCheck(void);
```
