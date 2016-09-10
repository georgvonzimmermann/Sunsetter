/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: notation.h                                                         *
 *  Purpose: Has the functions and prototypes for converting the algebraic   *
 * notation that humans use to and from the one that Sunsetter uses.          *
 *                                                                           *
 *************************************************************************** */

#ifndef _NOTATION_
#define _NOTATION_

#include "board.h"

piece charToPiece(char ch);
char pieceToChar(piece p);
char colorToChar(color c);
void DBMoveToRawAlgebraicMove(move m, char *str);
void getHoldingString(char *str);

#ifdef GAMETREE
void printHtmlBoard (FILE *fi); 
#endif

#endif
