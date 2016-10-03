/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: bitboard.cc                                                        *
 *  Purpose: Has the code for bit boards.                                    *
 *           bitboards are 64 bits of data with each bit representing a      *
 *           square on a board.  They're used for things like what squares   *
 *           are occupied by a pieces, or attacked by a color                *
 *                                                                           *
 *  Comments:																 *
 *                                                                           *
 *            Most of the bitboards go in increasing order of squares (a1 a2 *
 *            a3 a4 ... h8).  But bitboards organized in other ways are also *
 *            used.  There is one that has rank as the lower value, so it    *
 *            goes (a1 b1 c1 d1 ... h8), it's helpful when trying to find    *
 *            out what pieces are on a rank.  Also there are bitboards       *
 *            organized by diagonals (a1 b1 a2 c1 b2 a3 ... h8) to help with *
 *            finding what pieces are on a diagonal.  Although they don't    *
 *            really go in order, some of the diagonals re-aranged.  This is *
 *            so that no diagonal is split over the two 32 bit values in the *
 *            bitboard which allows split shifts to be used (see board.h for *
 *            a description of split shifts).                                *
 *                                                                           *
 *************************************************************************** */



#include <stdlib.h>

#include "board.h"
#include "bughouse.h"
#include "interface.h"
#include "brain.h" 

/* following are lots of arrays for bitboards.  They contain information that
   can be calculated beforehand and so when the value is needed durring a
   search all that as to be done is an index to the array.  */

/* a bitboard where just the one bit is set */

qword BitInBB[SQUARES]; 


/* Attacks is an array of bitboards of where a short range piece attacks from 
   each square */

bitboard pawnAttacks[COLORS][SQUARES];
bitboard contactBishopAttacks[SQUARES];
bitboard contactRookAttacks[SQUARES];
bitboard knightAttacks[SQUARES];
bitboard kingAttacks[SQUARES];

/* squaresTo gives the squares in between the first square and the second
   square, including the second square (if the squares are a knight move apart
   then just the second square).  It's used to see what squares a
   player can block or capture to get out of check with */

bitboard squaresTo[SQUARES][SQUARES];

/* nearSquares are squares 2 or less squares away */

bitboard nearSquares[SQUARES][COLORS];

/* directionPiece either a bishop, rook or none for the piece that can
   move along a line from one one square to the next. */

piece directionPiece[SQUARES][SQUARES];

/* squaresPast is the squares are after the second square, on the same
   line as the first */

bitboard squaresPast[SQUARES][SQUARES];

/* urShift and ulShift are to help with the diagonal bitboards.  The square on
   the up-right diagonal bitboard that corresponds to a normal bitboard is
   found by indexing urShift.  So square 5 (a6) on a normal bitboard is 24 on
   the up-right diagonal bitboard (ulShift[5]).  The order of the squares is
   kind of funky, the only things that are important are that squares of
   diagonals have adjacent bits and that the 32 bit is the end of one diagonal
   (so that no diagonals cross 32 bit boundaries and optimizations can be
   used). */

int ulShift[SQUARES] = 
   {  
    0,  2,  5, 13, 18, 24, 31, 39,
    1,  4, 12, 17, 23, 30, 38, 46,
    3, 11, 16, 22, 29, 37, 45, 52,
   10, 15, 21, 28, 36, 44, 51, 57,
   14, 20, 27, 35, 43, 50, 56,  9,
   19, 26, 34, 42, 49, 55,  8, 60,
   25, 33, 41, 48, 54,  7, 59, 62,
   32, 40, 47, 53,  6, 58, 61, 63 
   };

int urShift[SQUARES] = 
   {
   32, 40, 47, 53,  6, 58, 61, 63,
   25, 33, 41, 48, 54,  7, 59, 62,
   19, 26, 34, 42, 49, 55,  8, 60,
   14, 20, 27, 35, 43, 50, 56,  9,
   10, 15, 21, 28, 36, 44, 51, 57,
    3, 11, 16, 22, 29, 37, 45, 52,
    1,  4, 12, 17, 23, 30, 38, 46,
    0,  2,  5, 13, 18, 24, 31, 39 
   };

/* urDiagShift and ulDiagShift are to help with the diagonal bitboards.
   To get the diagonal of a square on the lowest bits of a bitboard, right 
   shift it by the amout in the array indexed at the square */

int ulDiagShift[SQUARES] = 
   {
    0,  1,  3, 10, 14, 19, 25, 32,
    1,  3, 10, 14, 19, 25, 32, 40,
    3, 10, 14, 19, 25, 32, 40, 47,
   10, 14, 19, 25, 32, 40, 47, 53,
   14, 19, 25, 32, 40, 47, 53,  6,
   19, 25, 32, 40, 47, 53,  6, 58,
   25, 32, 40, 47, 53,  6, 58, 61,
   32, 40, 47, 53,  6, 58, 61, 63
   };

int urDiagShift[SQUARES] = 
   {
   32, 40, 47, 53,  6, 58, 61, 63,
   25, 32, 40, 47, 53,  6, 58, 61,
   19, 25, 32, 40, 47, 53,  6, 58,
   14, 19, 25, 32, 40, 47, 53,  6,
   10, 14, 19, 25, 32, 40, 47, 53,
    3, 10, 14, 19, 25, 32, 40, 47,
    1,  3, 10, 14, 19, 25, 32, 40,
    0,  1,  3, 10, 14, 19, 25, 32
   };



/* *AttacksArray[square][info_on_a_line] gives where a piece can attacks on a line
   based on where the piece is on the board, and what squares are occupied
   along the line */

bitboard rankAttacksArray[SQUARES][256];
bitboard fileAttacksArray[SQUARES][256];
bitboard diagURAttacksArray[SQUARES][256];
bitboard diagULAttacksArray[SQUARES][256];


/* 
 * Function: fileRankSwap
 * Input:    A square
 * Output:   A square with it's rank and file bits swaped
 * Purpose:  Mostly used to get the bit for a mirrored bitboard
 */

square fileRankSwap(square sq)
   {
   return ((sq & 7) << 3 ) | sq >> 3;
   }


/* 
 * Function: pawnAttacksFrom
 * Input:    The color and square the pawn is on
 * Output:   A bit board of places it attacks
 * Purpose:  Used to generate a bitboard of places a pawn attacks
 */

bitboard boardStruct::pawnAttacksFrom(color c, square sq)
   {
   return pawnAttacks[c][sq];
   }


/* 
 * Function: rankAttacks
 * Input:    A square.
 * Output:   A bitboard of the squares that a rook would attack along the
 *           rank of the square
 * Purpose:  Used to find rank attacks
 */

bitboard boardStruct::rankAttacks(square sq)
   {
   return rankAttacksArray[sq][((occupied[WHITE] | occupied[BLACK]) >
            (sq & 0x38)) & 0xFF];
   }


/* 
 * Function: fileAttacks
 * Input:    A square.
 * Output:   A bitboard of the squares that a rook would attack along the
 *           file of the square
 * Purpose:  Used to find file attacks
 */

bitboard boardStruct::fileAttacks(square sq)
   {
   return fileAttacksArray[sq][(occupiedMirror > ((sq << 3) & 0x38)) & 0xFF];
   }


/* 
 * Function: diagULAttacks
 * Input:    A square.
 * Output:   A bitboard of the squares that a bishop would attack along the
 *           diagonal going up and left from the square
 * Purpose:  Used to find diagonal attacks going up and left.
 */

bitboard boardStruct::diagULAttacks(square sq)
   {
   return diagULAttacksArray[sq][(occupiedUL > ulDiagShift[sq]) & 0xFF];
   }


/* 
 * Function: diagURAttacks
 * Input:    A square.
 * Output:   A bitboard of the squares that a bishop would attack along the
 *           diagonal going up and right from the square
 * Purpose:  Used to find diagonal attacks going up and right.
 */

bitboard boardStruct::diagURAttacks(square sq)
   {
   return diagURAttacksArray[sq][(occupiedUR > urDiagShift[sq]) & 0xFF];
   }


/* 
 * Function: attacksFrom
 * Input:    A piece, the square it's on.
 * Output:   A bit board of places it attacks
 * Purpose:  Used to generate a bitboard of places a piece attacks
 */

bitboard boardStruct::attacksFrom(piece p, square sq)
   {
   switch(p) 
      {
      case KNIGHT:
         return knightAttacks[sq];
      case ROOK:
         return rankAttacks(sq) | fileAttacks(sq);
      case BISHOP:
         return  diagULAttacks(sq) | diagURAttacks(sq);
      case QUEEN:
         return attacksFrom(ROOK, sq) | attacksFrom(BISHOP, sq);
      case KING:
         return kingAttacks[sq];
 
	  default:

		  assert (0); 
		  return 1;		// just there so the compiler doesn't say I'm not 
						// returning a value */
		 
      }
   }

/* 
 * Function: contactAttacksFrom
 * Input:    A piece, the square it's on.
 * Output:   A bit board of places it attacks
 * Purpose:  Used to generate a bitboard of places a piece attacks without allowing interposition
 */

bitboard boardStruct::contactAttacksFrom(piece p, square sq)
   {
   switch(p) 
      {
      case KNIGHT:
         return knightAttacks[sq];
      case ROOK:
         return contactRookAttacks[sq];
      case BISHOP:
         return contactBishopAttacks[sq];
      case QUEEN:
         return kingAttacks[sq];

	  default:

		  assert (0); 
		  return 1;		// just there so the compiler doesn't say I'm not 
						// returning a value */
     }
   }


/*
 * Function: attacksTo
 * Input:    A square
 * Output:   A bit board with squares of the pieces that attack that square
 * Purpose:  Used to generate a bitboard of places that attack a square
 */

bitboard boardStruct::attacksTo(square sq)
   {
   bitboard attacksT;

   /* For all attacks just look it up in the tables where one of the pieces
      would attack from the square and and it with the
      squares that are occupied by those pieces.  */

   /* WARNING does not include king attacks */


   attacksT = attacksFrom(KNIGHT, sq) & pieces[KNIGHT];
   attacksT |= pawnAttacksFrom(WHITE, sq) & pieces[PAWN] & occupied[BLACK];
   attacksT |= pawnAttacksFrom(BLACK, sq) & pieces[PAWN] & occupied[WHITE];
   attacksT |= attacksFrom(BISHOP, sq) & (pieces[QUEEN] | pieces[BISHOP]);
   attacksT |= attacksFrom(ROOK, sq) & (pieces[QUEEN] | pieces[ROOK]);

   return attacksT;
   }


/* 
 * Function: blockedAttacks
 * Input:    A color and a square
 * Output:   A bitboard of all the attacks that a color has blocked by a square
 * Purpose:  Used to get squares that would be attacked if the piece on a
 *           square was moved off.
 */

bitboard boardStruct::blockedAttacks(color c, square where)
   {
   bitboard bb, blocked;
   square sq;

   bb = 0;
   blocked = rankAttacks(where) & (pieces[ROOK] | pieces[QUEEN]) & occupied[c];
   while (blocked.hasBits()) 
      {
      sq = firstSquare(blocked.data);
      blocked.unsetSquare(sq);
      bb |= rankAttacks(sq) & squaresPast[sq][where];
      }

   blocked = fileAttacks(where) & (pieces[ROOK]|pieces[QUEEN]) & occupied[c];
   while (blocked.hasBits()) 
      {
      sq = firstSquare(blocked.data);
      blocked.unsetSquare(sq);
      bb |= fileAttacks(sq) & squaresPast[sq][where];
      }

   blocked = diagULAttacks(where) & (pieces[BISHOP]|pieces[QUEEN])&occupied[c];
   while (blocked.hasBits()) 
      {
      sq = firstSquare(blocked.data);
      blocked.unsetSquare(sq);
      bb |= diagULAttacks(sq) & squaresPast[sq][where];
      }

   blocked = diagURAttacks(where) & (pieces[BISHOP]|pieces[QUEEN])&occupied[c];
   while (blocked.hasBits()) 
      {
      sq = firstSquare(blocked.data);
      blocked.unsetSquare(sq);
      bb |= diagURAttacks(sq) & squaresPast[sq][where];
      }
   
   
   
   return bb;
   }


/* 
 * Function: initBitboards
 * Input:    None
 * Output:   None
 * Purpose:  Used to set up all of the tables used in the bit boards.
 */

void initBitboards()
   {
   int leftborder, rightborder, upperborder, lowerborder; 
   int n, o, i;
   bitboard bb, bb2;
   square sq, sq2;


   /* First generate the Square -> Bitboard with only that square set 
      Lookup table, makes setting / unsetting squares MUCH faster */
	
	 // fix by angrim
	 for (sq = 0; sq < SQUARES; sq++) 
     // for (sq = 0; sq < (SQUARES+1); sq++) 
     { BitInBB[sq] = (qword(1) << (sq)); }

   /* Generate the attacks of short range pieces.  Just make sure that the
      attacks dont wrap around the board */

   for (sq = 0; sq < SQUARES; sq++) 
      {
	  
      pawnAttacks[WHITE][sq] = pawnAttacks[BLACK][sq] = qword(0);
      if (file(sq) == 0) 
         {
         pawnAttacks[WHITE][sq].setSquare(sq + ONE_FILE + ONE_RANK);
         pawnAttacks[BLACK][sq].setSquare(sq + ONE_FILE - ONE_RANK);
		 
		 
         } 
      else if (file(sq) == 7) 
         {
         pawnAttacks[WHITE][sq].setSquare(sq - ONE_FILE + ONE_RANK);
         pawnAttacks[BLACK][sq].setSquare(sq - ONE_FILE - ONE_RANK);
		 
		 
         } 
      else 
         {
         if (sq+ONE_FILE+ONE_RANK <= H8)  pawnAttacks[WHITE][sq].setSquare(sq + ONE_FILE + ONE_RANK);
         if (sq-ONE_FILE+ONE_RANK <= H8)  pawnAttacks[WHITE][sq].setSquare(sq - ONE_FILE + ONE_RANK);
         if (sq+ONE_FILE-ONE_RANK >= A1)  pawnAttacks[BLACK][sq].setSquare(sq + ONE_FILE - ONE_RANK);
         if (sq-ONE_FILE-ONE_RANK >= A1)  pawnAttacks[BLACK][sq].setSquare(sq - ONE_FILE - ONE_RANK);		 
		}



	  leftborder = rightborder = upperborder = lowerborder = 0; 
	  if (file(sq) == 0) leftborder = 1; 
	  if (file(sq) == 7) rightborder = 1; 
	  if (rank(sq) == 0) lowerborder = 1; 
	  if (rank(sq) == 7) upperborder = 1; 

	  if ((!leftborder) && (!lowerborder)) contactBishopAttacks[sq].setSquare(sq - ONE_FILE - ONE_RANK);
	  if ((!rightborder) && (!lowerborder)) contactBishopAttacks[sq].setSquare(sq + ONE_FILE - ONE_RANK);
	  if ((!leftborder) && (!upperborder)) contactBishopAttacks[sq].setSquare(sq - ONE_FILE + ONE_RANK);
	  if ((!rightborder) && (!upperborder)) contactBishopAttacks[sq].setSquare(sq + ONE_FILE + ONE_RANK);
	 
	  if (!leftborder) contactRookAttacks[sq].setSquare(sq-ONE_FILE); 
	  if (!rightborder) contactRookAttacks[sq].setSquare(sq+ONE_FILE); 
	  if (!lowerborder) contactRookAttacks[sq].setSquare(sq-ONE_RANK); 
	  if (!upperborder) contactRookAttacks[sq].setSquare(sq+ONE_RANK); 


      kingAttacks[sq] = 0;
      for (n = -1; n <= 1; n++)
         for (o = -1; o <= 1; o++) 
            {
            if (n == 0 && o == 0) 
               continue;
            if (rank(sq) + n >= 0 && rank(sq) + n <= 7 &&
                file(sq) + o >= 0 && file(sq) + o <= 7)
               kingAttacks[sq].setSquare(sq + n * ONE_RANK + o * ONE_FILE);
            }
    
      knightAttacks[sq] = 0;
      for (n = -2; n <= 2; n++) 
         {
         if (n == 0) 
            continue;
         if (file(sq) + n <= 7 && file(sq) + n >=0 &&
             rank(sq) + (3 - abs(n)) <= 7 && rank(sq) + (3 - abs(n)) >= 0)
            knightAttacks[sq].setSquare(sq + n * ONE_FILE + (3 - abs(n))*ONE_RANK);
    
         if (file(sq) + n <= 7 && file(sq) + n >=0 &&
             rank(sq) + (abs(n) - 3) <= 7 && rank(sq) + (abs(n) - 3) >= 0)
            knightAttacks[sq].setSquare(sq + n * ONE_FILE + (abs(n) - 3)*ONE_RANK);
         }
      }

   for (sq = 0; sq < SQUARES; sq++) 
      {
 
      /* Set up all of the attack bitboards.  The general formula to do this is
         to start on the square the piece is on.  Then add bits in every 
         direction until a piece or the edge is hit */

      for (o = 0; o < 256; o++) 
         {
         bb = 0;

         if (file(sq) < 7) 
            {
            for (i = ONE_FILE; ((1 << file(sq + i)) & o) == 0 && file(sq + i) < 7;
                 i += ONE_FILE)
               bb.setSquare(sq + i);
            bb.setSquare(sq + i);
            }
      
         if (file(sq) > 0) 
            {
            for (i = ONE_FILE; ((1 << file(sq - i)) & o) == 0 && file(sq - i) > 0;
                 i += ONE_FILE)
               bb.setSquare(sq - i);
            bb.setSquare(sq - i);
            }

         fileAttacksArray[sq][o] = bb;
         }
    
      for (o = 0; o < 256; o++) 
         {
         bb = 0;

         if (rank(sq) < 7) 
            {
            for (i = ONE_RANK; ((1 << rank(sq + i)) & o) == 0 && rank(sq + i) < 7;
                 i += ONE_RANK)
               bb.setSquare(sq + i);
            bb.setSquare(sq + i);
            }

         if (rank(sq) > 0) 
            {
            for (i = ONE_RANK; ((1 << rank(sq - i)) & o) == 0 && rank(sq - i) > 0;
                 i += ONE_RANK)
               bb.setSquare(sq - i);
            bb.setSquare(sq - i);
            }

         rankAttacksArray[sq][o] = bb;
         }

      for (o = 0; o < 256; o++) 
         {
         bb = 0;

         if (file(sq) < 7 && rank(sq) < 7) 
            {
            for (i = ONE_FILE + ONE_RANK;
                  ((1 << (urShift[sq + i] - urDiagShift[sq + i])) & o) == 0 &&
                  file(sq + i) < 7 && rank(sq + i) < 7;
                  i += ONE_FILE + ONE_RANK)
               bb.setSquare(sq + i);
            bb.setSquare(sq + i);
            }

         if (file(sq) > 0 && rank(sq) > 0) 
            {
            for (i = ONE_FILE + ONE_RANK;
                  ((1 << (urShift[sq - i] - urDiagShift[sq - i])) & o) == 0 &&
                  file(sq - i) > 0 && rank(sq - i) > 0;
                  i += ONE_FILE + ONE_RANK)
               bb.setSquare(sq - i);
            bb.setSquare(sq - i);
            }

         diagURAttacksArray[sq][o] = bb;
         }

      for (o = 0; o < 256; o++) 
         {
         bb = 0;

         if (rank(sq) < 7 && file(sq) > 0) 
            {
            for (i = ONE_RANK - ONE_FILE;
                  ((1 << (ulShift[sq + i] - ulDiagShift[sq + i])) & o) == 0 && 
                  rank(sq + i) < 7 && file(sq + i) > 0; 
                  i += ONE_RANK - ONE_FILE)
               bb.setSquare(sq + i);
            bb.setSquare(sq + i);
            }

         if (rank(sq) > 0 && file(sq) < 7) 
            {
            for (i = ONE_RANK - ONE_FILE;
                  ((1 << (ulShift[sq - i] - ulDiagShift[sq - i])) & o) == 0 &&
                  rank(sq - i) > 0 && file(sq - i) < 7; 
                  i += ONE_RANK - ONE_FILE)
               bb.setSquare(sq - i);
            bb.setSquare(sq - i);
            }

      diagULAttacksArray[sq][o] = bb;
      }
   }

    /* Get the squaresTo, squaresPast and directionPiece array.  See if the
       from square is along the same line as to to square, if it is then set
       all of the squares between them on the squaresTo and all the bits past
       in squaresPast and set directionPiece to be the piece that moves like
       that. */

   for (n = 0; n < SQUARES; n++) 
      {
      for (o = 0; o < SQUARES; o++) 
         {
         bb = 0;
         if (n == o) 
            {
            squaresTo[n][o] = 0;
            squaresPast[n][o] = 0;
            directionPiece[n][o] = NONE;
            }
         else if (file(n) == file(o)) 
            {
            directionPiece[n][o] = ROOK;
            sq = n;
            do 
               {
               if (rank(o) > rank(sq)) 
                  sq += ONE_RANK;
               else 
                  sq -= ONE_RANK;
               bb.setSquare(sq);
               } while(sq != o);
            squaresTo[n][o] = bb;

            bb = 0;
            if (rank(sq) != 0 && rank(sq) != 7) 
               {
               do 
                  {
                  if (rank(o) > rank(n)) 
                     sq += ONE_RANK;
                  else 
                     sq -= ONE_RANK;
                  bb.setSquare(sq);
                  } while(rank(sq) != 0 && rank(sq) != 7);
            }
         squaresPast[n][o] = bb;
         } 
      else if (rank(n) == rank(o)) 
         {
         directionPiece[n][o] = ROOK;
         sq = n;
         do 
            {
            if (file(o) > file(sq)) 
               sq += ONE_FILE;
            else 
               sq -= ONE_FILE;
            bb.setSquare(sq);
            } while(sq != o);
         squaresTo[n][o] = bb;

         bb = 0;
         if (file(sq) != 0 && file(sq) != 7) 
            {
            do 
               {
               if (file(o) > file(n)) 
                  sq += ONE_FILE;
               else 
                  sq -= ONE_FILE;
               bb.setSquare(sq);
               } while(file(sq) != 0 && file(sq) != 7);
            }
         squaresPast[n][o] = bb;
      } 
   else if (rank(o) - rank(n) == file(o) - file(n)) 
      {
      directionPiece[n][o] = BISHOP;
      sq = n;
      do 
         {
         if (rank(o) > rank(sq)) 
            sq += ONE_FILE + ONE_RANK;
         else 
            sq -= ONE_FILE + ONE_RANK;
         bb.setSquare(sq);
         } while(sq != o);
      squaresTo[n][o] = bb;

      bb = 0;
      if (file(sq) != 0 && file(sq) != 7 &&
         rank(sq) != 0 && rank(sq) != 7) 
         {
         do 
            {
            if (file(o) > file(n)) 
               sq += ONE_FILE + ONE_RANK;
            else 
               sq -= ONE_FILE + ONE_RANK;
            bb.setSquare(sq);
            } while(file(sq) != 0 && file(sq) != 7 &&
         rank(sq) != 0 && rank(sq) != 7);
         }
      squaresPast[n][o] = bb;
      } 
   else if (rank(o) - rank(n) == -(file(o) - file(n))) 
      {
      directionPiece[n][o] = BISHOP;
      sq = n;
      do {
         if (rank(o) > rank(sq)) 
            sq += ONE_RANK - ONE_FILE;
         else 
            sq -= ONE_RANK - ONE_FILE;
         bb.setSquare(sq);
         } while(sq != o);
      squaresTo[n][o] = bb;

      bb = 0;
      if (file(sq) != 0 && file(sq) != 7 &&
            rank(sq) != 0 && rank(sq) != 7) 
         {
         do 
            {
            if (rank(o) > rank(n)) 
               sq += ONE_RANK - ONE_FILE;
            else 
               sq -= ONE_RANK - ONE_FILE;
            bb.setSquare(sq);
            } while (file(sq) != 0 && file(sq) != 7 &&
                     rank(sq) != 0 && rank(sq) != 7);
         }
      
      squaresPast[n][o] = bb;
      }
   else if (knightAttacks[n].squareIsSet(o)) 
      {
      squaresPast[n][o] = 0;
      squaresTo[n][o] = qword(1) << o;
      } 
   else 
      {
      squaresPast[n][o] = squaresTo[n][o] = 0;
      }
    }
  }

   /* Set up the near squares, these are squares that are no more than two
      squares away */
  
  for (sq = 0; sq < SQUARES; sq++)
  {
	  // in this implementation WHITE and BLACK is the same, could be just one array (TODO).
	  nearSquares[sq][WHITE] = (qword)0;
	  nearSquares[sq][BLACK] = (qword)0;

	  bb = kingAttacks[sq];

	  while (bb.hasBits())
	  {
		  sq2 = firstSquare(bb.data);
		  bb.unsetSquare(sq2);

		  nearSquares[sq][BLACK] |= kingAttacks[sq2];
		  nearSquares[sq][WHITE] |= kingAttacks[sq2];
	  }
  }

   return;
   }


/* 
 * Function: popCount
 * Input:    None
 * Output:   An int
 * Purpose:  Used to find how many squares are set in a bitboard, in the
 *           process it clears the bitboard
 */

int bitboard::popCount()
   {
   int count = 0;
  
   while(data) 
      {
      count++;
      data = data & (data - 1);
      }
   return count;
   }


/* 
 * Function: addToBitBoards
 * Input:    The color that moved, a square and a piece
 * Output:   None
 * Purpose:  Used update the bitboards based on a new piece comming a square
 */

void boardStruct::addToBitboards(color c, piece p, square sq)
   {
   occupied[c].setSquare(sq);
   pieces[p].setSquare(sq);
   occupiedMirror.setSquare(fileRankSwap(sq));
   occupiedUR.setSquare(urShift[sq]);
   occupiedUL.setSquare(ulShift[sq]);
   }
 

/* 
 * Function: removeFromBitBoards
 * Input:    The color that moved, a square and a piece
 * Output:   None
 * Purpose:  Used update the bitboards based on a piece leaving a square
 */

void boardStruct::removeFromBitboards(color c, piece p, square sq)
   {
   occupied[c].unsetSquare(sq);
   pieces[p].unsetSquare(sq);
   occupiedMirror.unsetSquare(fileRankSwap(sq));
   occupiedUR.unsetSquare(urShift[sq]);
   occupiedUL.unsetSquare(ulShift[sq]);
   }



/*
* Function: resetBitboards
* Input:    None
* Output:   None
* Purpose:  makes the bitboards the starting position.
*/

void boardStruct::resetBitboards()
{

	/* On the starting position the first 16 squares are white, so set the
	first 16 bits on the bitboard, do the same with the last 16 bits for
	black */

	occupied[WHITE] = qword(0x0303030303030303);
	occupied[BLACK] = qword(0xC0C0C0C0C0C0C0C0);

	/* Get the mirrored and diagonal bitboards by getting each bit
	from the occupied bitboards and doing the apropriate transformation */

	updateDiagonalBitboards();
	

	/* Set up the pieces array */

	pieces[PAWN] = qword(0x4242424242424242);
	pieces[ROOK] = qword(0x8100000000000081);
	pieces[KNIGHT] = qword(0x0081000000008100);
	pieces[BISHOP] = qword(0x0000810000810000);
	pieces[QUEEN] = qword(0x0000000081000000);
	pieces[KING] = qword(0x0000008100000000);
}



void boardStruct::updateDiagonalBitboards() {
	bitboard bb;
	square sq;

	occupiedUL = occupiedUR = occupiedMirror = 0;
	bb = occupied[WHITE] | occupied[BLACK];
	while (bb.hasBits())
	{
		sq = firstSquare(bb.data);
		bb.unsetSquare(sq);
		occupiedMirror.setSquare(fileRankSwap(sq));
		occupiedUR.setSquare(urShift[sq]);
		occupiedUL.setSquare(ulShift[sq]);
	}
}


