/* ***************************************************************************
 *                               Sunsetter                                   *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: evaluation.h                                                       *
 *  Purpose: Has the tables used to give scores to things like development   *
 *           king safety.                                                    *
 *                                                                           *
 *                                                                           *
 *************************************************************************** */

/* Development tables are how Sunsetter measures development.  The piece is
   assigned the value of the square it's on.  Like the other evaluation, they're
   measuerd in centipawns.  These are done so that they are from white's
   perspective.  The bottom row is for the first rank and the left column is
   for the a file.  */

int pawnDevelopmentTable[64] = {   0,  0,  0,  0,  0,  0,  0,  0,
                                   5,  6,  5,  5,  5,  5,  6,  5,
                                   0,  0,  0,  3,  3,  0,  0,  0,
                                   0,  0,  0,  3,  3,  0,  0,  0,
                                  -2, -1,  0,  2,  2,  0, -1, -2,
                                  -2, -1,  0,  0,  0,  0, -1, -2,
                                   4,  6,  2, -4, -6,  4,  8,  4,
                                   0,  0,  0,  0,  0,  0,  0,  0 };

int rookDevelopmentTable[64]   = { 5,  5,  5,  5,  5,  5,  5,  5,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   1,  0,  0,  0,  0,  0,  0,  1 };

int knightDevelopmentTable[64] ={  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0, 0,  0,  0,  0,  0, 0,  0 };

int bishopDevelopmentTable[64] ={  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0 };


int queenDevelopmentTable[64] = { -10,-10,-10,-10,-10,-10,-10,-10,
                                  -10,-10,-10,-10,-10,-10,-10,-10,
                                  -20,-20,-20,-20,-20,-20,-20,-20,
                                  -20,-20,-20,-20,-20,-20,-20,-20,
                                  -20,-20,-20,-20,-20,-20,-20,-20,
                                  -20,-20,-20,-20,-20,-20,-20,-20,
                                  -12,-12,-10, -8, -8,-10,-12,-12,
                                  -16,-14,-10, -4, -8,-10,-14,-16 };

int kingDevelopmentTable[64] =   {-10,-18,-25,-25,-25,-25,-18,-10,
                                  -18,-25,-25,-25,-25,-25,-25,-18,
                                  -25,-35,-35,-35,-35,-35,-35,-25,
                                  -25,-35,-35,-35,-35,-35,-35,-25,
                                  -18,-25,-35,-35,-35,-35,-25,-18,
                                  -10,-18,-25,-25,-25,-25,-25,-10,
                                  -2, -6,-11,-11,-11,-11, -6, -2,
                                   1,  2,  1,  0,  1,  0,  2,  1 };



