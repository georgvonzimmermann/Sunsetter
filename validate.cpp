/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: validate.cc                                                        *
 *  Purpose: Has functions to test if moves are legal, a side is in check    *
 *           and other stuff.                                                *
 *                                                                           *
 *************************************************************************** */
 
#include <stdlib.h>
#include <stdio.h>
#include "interface.h"
#include "board.h"
#include "bughouse.h"



/* 
 * Function: isLegal.
 * Input:    A Move.
 * Output:   Whether the move is legal.
 * Purpose:  Used to check the legality of a move.
 */

int boardStruct::isLegal(move m)
   {
   move legalMoves[MAX_MOVES];
   int n, count;

   if (m.isBad()) 
      return 0;
   count = moves(legalMoves);
   for (n = 0; n < count; n++)
      if (legalMoves[n] == m) 
         return 1;
   return 0; 
   }


/*
 * Function: isInCheck.
 * Input:    A color.
 * Output:   0 if the side is not in check, 1 if they are in check and 2 if
 *           they are in double check.
 * Purpose:  Used to see if the side to move is in check.
 */

int boardStruct::isInCheck(color side)
   {
   return attacks[otherColor(side)][kingSquare[side]];
   }


/* 
 * Function: cantBlock.
 * Input:    None
 * Output:   Whether the side to move can block a check.
 * Purpose:  Used by the AI to see if a position is really mate, or if it's
 *           just a check that the opponent has to sit to get out of.
 */

int boardStruct::cantBlock()
   {
   bitboard checks, unblockable;

   checks = attacksTo(kingSquare[onMove]) & occupied[otherColor(onMove)];
  
   if (checks.moreThanOne()) 
      return 1;
   unblockable = attacksFrom(KING, kingSquare[onMove]) |
                  attacksFrom(KNIGHT, kingSquare[onMove]);
   if ((checks & unblockable).hasBits()) 
      return 1;
   return 0;
   }



/*
 * Function: isCheckmate.
 * Input:    None
 * Output:   Whether the side to is in checkmate.
 * Purpose:  Used to see if a position is checkmate
 */

int boardStruct::isCheckmate()
   {
   move m[MAX_MOVES];

   if (!isInCheck(onMove)) 
      return 0;
   if (moves(m) != 0) 
      return 0;
   if (!cantBlock() && currentRules == BUGHOUSE) 
      return 0;
   return 1;
   }


