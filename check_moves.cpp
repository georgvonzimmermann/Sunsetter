/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: check_moves.cc                                                     *
 *  Purpose: Has checkEvasionMoves() which is used to generate moves to get  *
 *  out of check. Note that this does not generate only legal moves !        *
 *                                                                           *
 *************************************************************************** */

#include "board.h"

/* fillMoveArray is from aiMoves.cc given a square that a piece is moving
   from and a bitboard with the destination squares set it fills a move
   array and increments the pointer to that array */

inline extern void fillMoveArray(move **m, square from, piece p,bitboard dest);

/* 
 * Function: checkEvasionCaptures
 * Input:    a pointer to moves.
 * Output:   The number of moves generated
 * Purpose:  Generates moves that (probably) get out of check this includes:
 *           1) Capture of the checking piece if there is only one
 */


int boardStruct::checkEvasionCaptures(move *m)
   {
   bitboard possibleTo, checks, thePieces;
   square sq, checkSquare;
   move *original;
   bitboard bb;

   original = m;

   /* Try captures by pieces other than the king.  */

   checks = attacksTo(kingSquare[onMove]) & occupied[otherColor(onMove)];
   if (!checks.moreThanOne()) 
      {
      checkSquare = firstSquare(checks.data);

      if (rank(checkSquare) != 0 && rank(checkSquare) != 7) 
      {
        thePieces = attacksTo(checkSquare) & occupied[onMove];
        while (thePieces.hasBits()) 
        {
			sq = firstSquare(thePieces.data);
			thePieces.unsetSquare(sq);
		
			removeFromBitboards(onMove, position[sq], sq);
			bb = blockedAttacks(otherColor(onMove),sq); 
			addToBitboards(onMove, position[sq], sq);
			
			if (! bb.squareIsSet(kingSquare[onMove]))
			{
				*m++ = move(sq, checkSquare, position[sq]);
			}			
        }
		/*	The bug of missing ep-captures to get out of check has been fixed
			simultaniously, therefore we have two versions (see below)
			@georg: Test this even further when perft () is in place 

		if (enPassant != OFF_BOARD && position[checkSquare] == PAWN)
		{
			thePieces = attacksTo(enPassant) & pieces[PAWN] & occupied[onMove];
			while (thePieces.hasBits())
			{
				sq = firstSquare(thePieces.data);
				thePieces.unsetSquare(sq);

				removeFromBitboards(onMove, position[sq], sq);
				bb = blockedAttacks(otherColor(onMove), sq);
				addToBitboards(onMove, position[sq], sq);
				if (!bb.squareIsSet(kingSquare[onMove]))
				{
					*m++ = move(sq, enPassant, position[sq]);
				}
			}
		}
		*/ 
      }
      else 
      {
         thePieces = attacksTo(checkSquare) & ~pieces[PAWN] & occupied[onMove];
         while (thePieces.hasBits()) 
            {
            sq = firstSquare(thePieces.data);
            thePieces.unsetSquare(sq);
			
			
			removeFromBitboards(onMove, position[sq], sq);
			bb = blockedAttacks(otherColor(onMove),sq); 
			addToBitboards(onMove, position[sq], sq);
			
			if (! bb.squareIsSet(kingSquare[onMove]))
			{
				*m++ = move(sq, checkSquare, position[sq]);
			}
            }
    
      thePieces = attacksTo(checkSquare) & pieces[PAWN] & occupied[onMove];
      while (thePieces.hasBits()) 
         {
         sq = firstSquare(thePieces.data);
         thePieces.unsetSquare(sq);
		 
			
		removeFromBitboards(onMove, position[sq], sq);
		bb = blockedAttacks(otherColor(onMove),sq); 
		addToBitboards(onMove, position[sq], sq);
			
		if (! bb.squareIsSet(kingSquare[onMove]))
         {
			*m++ = move(sq, checkSquare, position[sq], QUEEN);		 
			*m++ = move(sq, checkSquare, position[sq], KNIGHT);
		 }
         }
      }
	  }

	   /* ep captures? */
	   if (enPassant != OFF_BOARD)
	   {
		   if (onMove == WHITE)
		   {
			   whitePawnCapturesTo(&m, (qword(1) << enPassant));
		   }
		   else
		   {
			   blackPawnCapturesTo(&m, (qword(1) << enPassant));
		   }
	   }

	 /* see what king captures there are */

     possibleTo = attacksFrom(KING, kingSquare[onMove]) &
                           occupied[otherColor(onMove)];

	 // white rook checks black king at g8 on back row, white knight on 
	 // h8 can obviously not be taken. Check for that:
	 /* This code from the chess version might be a speed improvement, needs testing.
	 // remember to update below with "			&& (! bb.squareIsSet(sq)))" !!
	 removeFromBitboards(onMove, KING, kingSquare[onMove]);
	 bb = blockedAttacks(otherColor(onMove), kingSquare[onMove]);
	 addToBitboards(onMove, KING, kingSquare[onMove]);
	 */ 


     while (possibleTo.hasBits()) 
      {
      sq = firstSquare(possibleTo.data);
      possibleTo.unsetSquare(sq);
      if (attacks[otherColor(onMove)][sq] == 0)
         *(m++) = move(kingSquare[onMove], sq, KING);
      }
  
   m->makeBad();
   return m - original;
   }

/* 
 * Function: checkEvasionOthers
 * Input:    a pointer to moves.
 * Output:   The number of moves generated
 * Purpose:  Generates moves that (probably) get out of check this includes:
 *           2) Blocking the checking piece if there is only one
 *           3) Moving the king to an unattacked square
 */



int boardStruct::checkEvasionOthers(move *m)
   {
   bitboard to, possibleTo, checks, thePieces;
   square sq, checkSquare;
   move *original;
   piece p;

   original = m;

    /* Try to block the check */

   checks = attacksTo(kingSquare[onMove]) & occupied[otherColor(onMove)];
   if (!checks.moreThanOne()) 
      {
	   
	  checkSquare = firstSquare(checks.data);

      possibleTo = squaresTo[kingSquare[onMove]][checkSquare];
      possibleTo.unsetSquare(checkSquare);

      thePieces = pieces[KNIGHT] & occupied[onMove];
    
      while (thePieces.hasBits()) 
         {
         sq = firstSquare(thePieces.data);
         thePieces.unsetSquare(sq);
         to = possibleTo & attacksFrom(KNIGHT, sq);
         fillMoveArray(&m, sq, KNIGHT, to);
         } 
    
      thePieces = pieces[BISHOP] & occupied[onMove];
    
      while (thePieces.hasBits()) 
         {
         sq = firstSquare(thePieces.data);
         thePieces.unsetSquare(sq);
         to = possibleTo & attacksFrom(BISHOP, sq);
         fillMoveArray(&m, sq, BISHOP, to);
         }
    
      thePieces = pieces[ROOK] & occupied[onMove];
    
      while (thePieces.hasBits()) 
         {
         sq = firstSquare(thePieces.data);
         thePieces.unsetSquare(sq);
         to = possibleTo & attacksFrom(ROOK, sq);
         fillMoveArray(&m, sq, ROOK, to);
         }
    
      thePieces = pieces[QUEEN] & occupied[onMove];
    
      while (thePieces.hasBits()) 
         {
         sq = firstSquare(thePieces.data);
         thePieces.unsetSquare(sq);
         to = possibleTo & attacksFrom(QUEEN, sq);
         fillMoveArray(&m, sq, QUEEN, to);
         }
    
      if (onMove == WHITE) 
         {
         whitePawnMovesTo(&m, possibleTo & ~EIGHTH_RANK, NONE);
         whitePawnMovesTo(&m, possibleTo & EIGHTH_RANK, QUEEN);
         whitePawnMovesTo(&m, possibleTo & EIGHTH_RANK, KNIGHT);
         } 
      else 
         {
         blackPawnMovesTo(&m, possibleTo & ~FIRST_RANK, NONE);
         blackPawnMovesTo(&m, possibleTo & FIRST_RANK, QUEEN);
         blackPawnMovesTo(&m, possibleTo & FIRST_RANK, KNIGHT);
         }

      /* See if a piece can be dropped between the check.  This is every
        square that was searched before to block */
 
     if (hand[onMove][PAWN] > 0)
         fillMoveArray(&m, IN_HAND, PAWN, possibleTo & 
                      ~(FIRST_RANK | EIGHTH_RANK));
    
      for (p = ROOK; p < KING; p = (piece) (p + 1)) 
         {
         if (hand[onMove][p] > 0) 
            {
            fillMoveArray(&m, IN_HAND, p, possibleTo);
            }
         }
 
      }

   /* Last look for king moves that aren't captures */
  
   possibleTo = attacksFrom(KING, kingSquare[onMove]) &
                   ~(occupied[onMove] | occupied[otherColor(onMove)]);

   // white rook checks black king at g8 on back row, king 
   // can not move to h8. Check for that:
   /* Idea from chess-sunsetter (see above)

   removeFromBitboards(onMove, KING, kingSquare[onMove]);
   to = blockedAttacks(otherColor(onMove), kingSquare[onMove]);
   addToBitboards(onMove, KING, kingSquare[onMove]);
   */

   while (possibleTo.hasBits()) 
      {
      sq = firstSquare(possibleTo.data);
      possibleTo.unsetSquare(sq);
      if (attacks[otherColor(onMove)][sq] == 0)
         *(m++) = move(kingSquare[onMove], sq, KING);
      }

   m->makeBad();
   return m - original;
   }