/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: capture_moves.cc                                                   *
 *  Purpose: Has captureMoves() which is used for generating captures for    *
 *           use in the quiescense search.                                   *
 *                                                                           *
 *  Comments: captureMoves() generates only moves that capture on a certain  *
 *            square or pawn promotions (to queens and knights, because the  *
 *            other pieces are pointless)                                    *
 *                                                                           *
 *************************************************************************** */



#include "board.h"
#include "brain.h"



/* fillMoveArray is from aiMoves.cc given a square that a piece is moving
   from and a bitboard with the destination squares set it fills a move
   array and increments the pointer to that array */

inline extern void fillMoveArray(move **m, square from, piece p,bitboard dest);

/* 
 * Function: whitePawnCapturesTo
 * Input:    A pointer to and array of moves.
 * Output:   None.
 * Purpose:  Fills the array and increments the pointer to that array with
 *           whites's capures moves to a certain square.
 *           This function is very specific.  It assumes it is passed a valid
 *           set of squares (ones that white can do captures on) and doesn't work
 *           if there is a promotion involved
 */

void boardStruct::whitePawnCapturesTo(move **m, bitboard possibleTo)
   {
   bitboard to;
   square sq;

   to = possibleTo & ((pieces[PAWN] & occupied[WHITE] & ~A_FILE) >> 7);
   while(to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq + 7, sq, PAWN);

      }

   to = possibleTo & ((pieces[PAWN] & occupied[WHITE] & ~H_FILE) << 9);
   while(to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq - 9, sq, PAWN);

      }
   }


/* 
 * Function: blackPawnCapturesTo
 * Input:    A pointer to and array of moves moves.
 * Output:   None.
 * Purpose:  Fills the array and increments the pointer to that array with
 *           black's capures moves to a certain square.
 *           This function is very specific.  It assumes it is passed a valid
 *           set of squares (ones that black can captures on) and doesn't work
 *           if there is a promotion involved
 */

void boardStruct::blackPawnCapturesTo(move **m, bitboard possibleTo)
   {
   bitboard to;
   square sq;

   to = possibleTo & ((pieces[PAWN] & occupied[BLACK] & ~A_FILE) >> 9);
   while(to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq + 9, sq, PAWN);

      }

   to = possibleTo & ((pieces[PAWN] & occupied[BLACK] & ~H_FILE) << 7);
   while(to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq - 7, sq, PAWN);

      }
   }


/* 
 * Function: captureMoves
 * Input:    An array to fill with moves
 * Output:   The number of moves generated.
 * Purpose:  Used by quiesce() to generate captures and pawn promotions to a
 *           queen.  It fills an array of moves with them.
 *           As of version 1.0 this uses bitboards so it should be super
 *           fast.  Basically it generates a bitboard of possible captures 
 *           and ands them with what squares are occupied by the opponent.  
 *           The resulting bitboard represents the to squares.
 */

int boardStruct::captureMoves(move *m)
   {
   move *original;
   square from;
   bitboard thePieces, oppOccupied, dest;

   original = m;
   oppOccupied = occupied[otherColor(onMove)];

   /* Generate bishop/rook/queen captures bitboard by anding the attack bitboard
      with the square occupied by the opponent */
  
   thePieces = pieces[BISHOP] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      from = firstSquare(thePieces.data);
      thePieces.unsetSquare(from);
      dest = attacksFrom(BISHOP, from) & oppOccupied;
      fillMoveArray(&m, from, BISHOP, dest);
      }

   thePieces = pieces[ROOK] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      from = firstSquare(thePieces.data);
      thePieces.unsetSquare(from);
      dest = attacksFrom(ROOK, from) & oppOccupied;
      fillMoveArray(&m, from, ROOK, dest);
      }

   thePieces = pieces[QUEEN] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      from = firstSquare(thePieces.data);
      thePieces.unsetSquare(from);
      dest = attacksFrom(QUEEN, from) & oppOccupied;
      fillMoveArray(&m, from, QUEEN, dest);
      }

   /* Generate the knight moves bitboard by using the lookup table and 
      anding it with the square occupied by the opponent */

   thePieces = pieces[KNIGHT] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      from = firstSquare(thePieces.data);
      thePieces.unsetSquare(from);
      dest = knightAttacks[from] & oppOccupied;
      fillMoveArray(&m, from, KNIGHT, dest);
      }

   /* Generate pawn captures and promotions. */

   if (onMove == WHITE) 
      {
      whitePawnMovesTo(&m, EIGHTH_RANK, QUEEN);

      if (enPassant != OFF_BOARD)                                       
         whitePawnCapturesTo(&m, (occupied[BLACK] | qword(1) << enPassant) &
			                     ~EIGHTH_RANK);
      else
         whitePawnCapturesTo(&m, occupied[BLACK] & ~EIGHTH_RANK);

      whitePawnMovesTo(&m, EIGHTH_RANK, KNIGHT);
      }
   else 
      {  
      blackPawnMovesTo(&m, FIRST_RANK, QUEEN);

      if (enPassant != OFF_BOARD)
         blackPawnCapturesTo(&m, (occupied[WHITE] | qword(1) << enPassant) & 
			                    ~FIRST_RANK);
      else
         blackPawnCapturesTo(&m, occupied[WHITE] & ~FIRST_RANK);

      blackPawnMovesTo(&m, FIRST_RANK, KNIGHT);
      }  
    
   /* Get king moves. */

   from = kingSquare[onMove];
   dest = kingAttacks[from] & oppOccupied;
   fillMoveArray(&m, from, KING, dest);
   m->makeBad();
   return m - original;
   }


/* 
 * Function: captureGain
 * Input:    A color and A move
 * Output:   How much material it gains or loses.
 * Purpose:  Used to see how good a capture is.  It trades off all of the
 *           attackers including the x-ray attackers (pieces that can capture
 *           as soon as one of the other attackers moves out of their way).
 */

int boardStruct::captureGain(color c, move m)
   {
   bitboard attacks, bb;

   int gain[32], attackedValue, sign, count;
   square from, to;

   /* Step 1:  Do some initialization */

   count = 1;
   to = m.to();
   from = m.from();
   attacks = attacksTo(to);
  
   /* Step 2: Make the first capture and add the new attackers from x-ray
      attacks */
  
   gain[0] = pValue[position[to]];
   attackedValue = pValue[position[from]];
   attacks.unsetSquare(from);
   sign = -1;
   if (directionPiece[from][to] != NONE) 
      {
      attacks.setBits(attacksFrom(directionPiece[from][to], from)
		                & squaresPast[to][from] & 
            		    (pieces[directionPiece[from][to]] | pieces[QUEEN]));
      }

   /* Step 3: Do the rest of the captures.  The piece that captures is always
      the piece that has the lowest value */


   while(attacks.hasBits()) 
      {
      c = otherColor(c);
      if ((attacks & pieces[PAWN] & occupied[c]).hasBits())
	  {
         bb = (attacks & pieces[PAWN] & occupied[c]);
		 from = firstSquare(bb.data);      
	  }
      else if((attacks & pieces[KNIGHT] & occupied[c]).hasBits())
	  {
		 bb = (attacks & pieces[KNIGHT] & occupied[c]);
         from = firstSquare(bb.data);
	  }
	  else if((attacks & pieces[BISHOP] & occupied[c]).hasBits())
	  {
         bb = (attacks & pieces[BISHOP] & occupied[c]);
		 from = firstSquare(bb.data);
	  }
      else if((attacks & pieces[ROOK] & occupied[c]).hasBits())
	  {
		 bb = (attacks & pieces[ROOK] & occupied[c]);
         from = firstSquare(bb.data);
	  }
      else if((attacks & pieces[QUEEN] & occupied[c]).hasBits())
	  {
         bb = (attacks & pieces[QUEEN] & occupied[c]);
		 from = firstSquare(bb.data);
	  }
      else break;

      gain[count++] = gain[count - 1] + sign * attackedValue;
	  // This is bad code (notice the count++ and count being used on the right side of expression),
	  // might be undefined behaviour. Thx to Angrim for noticing.
	  // Angrims version is this:
	  //    gain[count] = gain[count - 1] + sign * attackedValue;
	  //	count++;
	  // @georg this needs throughout testing to check it does what is intended.


      attackedValue = pValue[position[from]];
      attacks.unsetSquare(from);
      sign *= -1;
      if (directionPiece[from][to] != NONE) 
         {
         attacks.setBits(attacksFrom(directionPiece[from][to], from)
		            & squaresPast[to][from] &
		            (pieces[directionPiece[from][to]] | pieces[QUEEN]));
         }
      }

   /* Step 4:  Now we have what the gain will be after every capture.  
      use a min-max like algorithm to see when the sides will stop capturing */

   while (--count > 0) 
      {
      sign = count & 1;        /* makes sign == 0 if it's the color that started
		                     		the capture and 1 if not */
      if (sign == 1) 
         {
         if (gain[count] < gain[count - 1]) 
            gain[count - 1] = gain[count];
         } 
      else 
         {
         if (gain[count] > gain[count - 1])
            gain[count - 1] = gain[count];
         }
      }

   return gain[0];
   }
