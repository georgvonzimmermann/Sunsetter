/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: move.cc                                                            *
 *  Purpose: Has moves() and other functions used for generating legal moves *
 *                                                                           *
 *  Comments: moves() generates legal moves.  It uses the moves that         *
 * AIMoves() generates and adds promotion to bishops and rooks.  Then it     *
 * makes sure that none of the moves put the side in check.                  *
 *                                                                           *
 *************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include "interface.h"
#include "variables.h"
#include "board.h"

/*
 * Function: moves
 * Input:    None
 * Output:   A array of all the legal moves.
 * Purpose:  Used to generate legal moves.  It gets the move array from 
 *           aiMoves() and sees which one of those are really legal.
 *
 */

int boardStruct::moves(move *m)
   {
   int n, count, i;
   move aim[MAX_MOVES];

  
   count = captureMoves(aim);
   orderCaptures(aim); 
   count += aiMoves(aim + count);
   count += skippedMoves(aim + count);

   i = 0;
   for (n = 0; n < count; n++) 
      {
      changeBoard(aim[n]);
      if (!isInCheck(otherColor(onMove))) 
         {
         m[i++] = aim[n];
         }
      unchangeBoard();
      }
   return i;
   }

