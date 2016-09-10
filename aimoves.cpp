/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *                                                                           *
 *  Name: aimove.cc                                                          *
 *  Purpose: Has aimoves() and other functions used for generating moves for *
 *          use in the search.                                               *
 *                                                                           *
 *  Comments: aimoves() generates normal moves for the search to use. It     *
 *  generates non-legal moves, because it doesn't check to see if a move     *
 *  puts the king in check.                                                  *
 *                                                                           *
 *************************************************************************** */

#include <stdlib.h>
#include <stdio.h>

#include "variables.h"
#include "board.h"
#include "brain.h"
#include "interface.h"

#define OFF_MOVE (otherColor(onMove))

/* 
 * Function: whitePawnMovesTo
 * Input:    A pointer to an array of moves, a bitboard of squares and a piece.
 * Output:   None.
 * Purpose:  Fills the array and increments the pointer to that array with
 *           white's pawn moves to a bitboard of squares.
 */

void boardStruct::whitePawnMovesTo(move **m, bitboard possibleTo, piece promote)
   {
   	
   bitboard to, to2;
   square sq;

   /* Step 1: Generate single push moves to the square by shifting the pawn
      bitboard forward one rank */

   to = possibleTo & (pieces[PAWN] & occupied[WHITE]) << ONE_RANK &
         ~(occupied[WHITE] | occupied[BLACK]);

   /* Step 2: Generate double push moves to the square by shifting moving the
      pawns forward again and anding off rank other than the 4th */
   
   to2 = possibleTo & (((pieces[PAWN] & occupied[WHITE]) << ONE_RANK &
	   ~(occupied[WHITE] | occupied[BLACK])) << ONE_RANK) &
          FOURTH_RANK & ~(occupied[WHITE] | occupied[BLACK]);

   /* Step 3: Put the pawn moves into the array */

   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq - ONE_RANK, sq, PAWN, promote);
      }

   while (to2.hasBits()) 
      {
      sq = firstSquare(to2.data);
      to2.unsetSquare(sq);
      *((*m)++) = move(sq - TWO_RANKS, sq, PAWN, promote);
      }

   /* Step 4: Generate captures to the left by anding off the pawns on the
      a file and shifting the bitboard by a rank minus a file */

   to = (possibleTo & ((pieces[PAWN] & occupied[WHITE] & ~A_FILE) >> 7) &
         occupied[BLACK]);
  
   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq + 7, sq, PAWN, promote);
      }

   /* Step 5: Generate captures to the right by doing the opposite of
      captures to the left */

   to = (possibleTo & ((pieces[PAWN] & occupied[WHITE] & ~H_FILE) << 9) &
         occupied[BLACK]);

   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq - 9, sq, PAWN, promote);
      }

   }


/* 
 * Function: blackPawnMovesTo
 * Input:    A pointer to an array of moves, a bitboard of squares and a piece
 * Output:   None.
 * Purpose:  Fills the array and increments the pointer to that array with
 *           black's pawn moves to a bitboard of squares.
 */

void boardStruct::blackPawnMovesTo(move **m, bitboard possibleTo, piece promote)
   {
   bitboard to, to2;
   square sq;

   /* Step 1: Generate single push moves to that square by shifting the pawn
      bitboard forward one rank */

   to = possibleTo & (pieces[PAWN] & occupied[BLACK]) >> ONE_RANK &
        ~(occupied[WHITE] | occupied[BLACK]);

   /* Step 2: Generate double push moves to the square by shifting moving the
      pawns forward again and anding off rank other than the 5th */

   to2 = possibleTo & (((pieces[PAWN] & occupied[BLACK]) >> ONE_RANK &
	   ~(occupied[WHITE] | occupied[BLACK])) >> ONE_RANK) &
         FIFTH_RANK & ~(occupied[WHITE] | occupied[BLACK]);


   /* Step 3: Put the pawn moves into the array */

   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq + ONE_RANK, sq, PAWN, promote);
      }

   while (to2.hasBits()) 
      {
      sq = firstSquare(to2.data);
      to2.unsetSquare(sq);
      *((*m)++) = move(sq + TWO_RANKS, sq, PAWN, promote);
      }

   /* Step 4:  Generate captures to the left by anding off the pawns on the
      a file and shifting the bitboard by a rank minus a file */

   to = (possibleTo & ((pieces[PAWN] & occupied[BLACK] & ~A_FILE) >> 9) &
         occupied[WHITE]);

   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq + 9, sq, PAWN, promote);
      }

   /* Step 5:  Generate captures to the right by doing the opposite of
      captures to the left */

   to = (possibleTo & ((pieces[PAWN] & occupied[BLACK] & ~H_FILE) << 7) &
         occupied[WHITE]);

   while (to.hasBits()) 
      {
      sq = firstSquare(to.data);
      to.unsetSquare(sq);
      *((*m)++) = move(sq - 7, sq, PAWN, promote);
      }
   }


/*
 * Function: fillMoveArray
 * Input:    A pointer to an array of move, a square to move from the piece 
 *           that moves a bitboard of squares to move to (dest is a reference
 *           so the compiler doesn't make a copy of bitboard which wastes
 *           time and is unnecesary).
 * Output:   None.
 * Purpose:  Fills an array of moves from a square to all of the set bits
 *           in a bitboard
 */

void fillMoveArray(move **m, square from, piece p, bitboard dest)
   {
   square to;
  
   while (dest.hasBits()) 
      {
      to = firstSquare(dest.data);
      dest.unsetSquare(to);
      *((*m)++) = move(from, to, p);
      }
   }

/*
 * Function: fillMoveArrayMateTriesKnights
 * Input:    A pointer to an array of move, a square to move from, a bitboard of squares to move to, 
 *           the attacks array and the color to move
 * Output:   None.
 * Purpose:  Fills an array of moves from a square to all of the set bits
 *           in a bitboard if the knight can not be captured ( its a possible mate ) 
 */

void fillMoveArrayMateTriesKnights (move **m, square from, bitboard dest, sword attacks[COLORS][64], color onMove)
   {
   square to;
  
   while (dest.hasBits()) 
      {
      to = firstSquare(dest.data);
      dest.unsetSquare(to);
      if (!attacks[OFF_MOVE][to]) *((*m)++) = move(from, to, KNIGHT);
      }
   }

/*
 * Function: fillMoveArrayMateTriesOthers
 * Input:    A pointer to an array of move, a square to move from, the piece 
 *           that moves, a bitboard of squares to move to, the attacks array and the color to move
 * Output:   None.
 * Purpose:  Fills an array of moves from a square to all of the set bits
 *           in a bitboard if the piece can not be captured ( its a possible mate ) 
 */

void fillMoveArrayMateTriesOthers (move **m, square from, piece p, bitboard dest, sword attacks[COLORS][64], color onMove)
   {
   square to;
  
   while (dest.hasBits()) 
      {
      to = firstSquare(dest.data);
      dest.unsetSquare(to);
      if ((attacks[onMove][to]) && (attacks[OFF_MOVE][to] == 1)) *((*m)++) = move(from, to, p);
      }
   }

   
  
/*
 * Function: AIMoves
 * Input:    An array to fill with moves.
 * Output:   The number of moves generated.
 * Purpose:  Used by search() to generate moves.  It fills an array
 *           of moves.
 *           Note that aiMoves does not generate all legal moves! It doesn't
 *           make sure that the move doesn't put the side into check, and it doesn't
 *			 generate captures.
 *           Moves are done by generating
 *           a bitboard of possible moves, using the first bit for the
 *           to square and then taking off the bits until the bitboard is empty.
 *			 First the moves to generateToFirst[Piece][Color][dropMove] are created, 
 *			 since they have been better in history.
 */

int boardStruct::aiMoves(move *m)
   {
   move *original;
   square sq;
   piece p;
   bitboard thePieces, dest, dest2, unoccupied, trythose;

   original = m;   

   unoccupied = (~(occupied[WHITE] | occupied[BLACK]));


   /* Generate bishop/rook/queen moves bitboard by and-ing the attack bitboard
      with the empty squares */

   trythose = generateToFirst[BISHOP][onMove][0] & unoccupied; 
  
   
   thePieces = pieces[BISHOP] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(BISHOP, sq) & trythose;
      fillMoveArray(&m, sq, BISHOP, dest);
      }

   trythose = generateToFirst[ROOK][onMove][0] & unoccupied;

   thePieces = pieces[ROOK] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(ROOK, sq) & trythose;
      fillMoveArray(&m, sq, ROOK, dest);
      }

   trythose = generateToFirst[QUEEN][onMove][0] & unoccupied;

   thePieces = pieces[QUEEN] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(QUEEN, sq) & trythose;
      fillMoveArray(&m, sq, QUEEN, dest);
      }

  /* Generate the knight moves bitboard by using the lookup table */

   trythose = generateToFirst[KNIGHT][onMove][0] & unoccupied;

   thePieces = pieces[KNIGHT] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = knightAttacks[sq] & trythose;
      fillMoveArray(&m, sq, KNIGHT, dest);
      }

  /* Generate pawn moves shifting the pawn bitboard by one rank and anding
     that with unoccupied squares, the from square is one rank before the to 
     square.
     Since captureMoves() generates promotions, take off any destination
     squares on the last rank.
     Then shift that bitboard forward one rank and and in it with unoccupied
     spaces on the 4th rank to get double pawn moves.
  */

   trythose = generateToFirst[PAWN][onMove][0] & unoccupied;

   if (onMove == WHITE) 
      {
      dest = ((pieces[PAWN] & occupied[WHITE]) << ONE_RANK) & ~EIGHTH_RANK & trythose;

     //* Get the bitboard of double pawn moves 

	  // to2 = possibleTo & (to << ONE_RANK) &FOURTH_RANK & ~(occupied[WHITE] | occupied[BLACK]);
      dest2 = ((((pieces[PAWN] & occupied[WHITE]) << ONE_RANK) & ~EIGHTH_RANK & unoccupied) << ONE_RANK) & trythose & FOURTH_RANK;
                        
      while (dest.hasBits()) 
         {
         sq = firstSquare(dest.data);
         dest.unsetSquare(sq);
         *(m++) = move(sq - ONE_RANK, sq, PAWN);
         }

      while (dest2.hasBits()) 
         {
         sq = firstSquare(dest2.data);
         dest2.unsetSquare(sq);
         *(m++) = move(sq - TWO_RANKS, sq, PAWN);
         }
      }
   else 
      {
      dest = ((pieces[PAWN] & occupied[BLACK]) >> ONE_RANK) & ~FIRST_RANK & trythose;

      //* Get the bitboard of double pawn moves

      dest2 = ((((pieces[PAWN] & occupied[BLACK]) >> ONE_RANK) & ~EIGHTH_RANK & unoccupied) >> ONE_RANK) & trythose & FIFTH_RANK;

      while (dest.hasBits()) 
         {
         sq = firstSquare(dest.data);
         dest.unsetSquare(sq);
         *(m++) = move(sq + ONE_RANK, sq, PAWN);
         }

      while (dest2.hasBits()) 
         {
         sq = firstSquare(dest2.data);
         dest2.unsetSquare(sq);
         *(m++) = move(sq + TWO_RANKS, sq, PAWN);
         }
      }



    /* Get the dropping moves, the from square in IN_HAND the to square
      is any unoccupied square (except for the 1st, 8th rank for pawns). */

   if (hand[onMove][PAWN]) 
      {
      trythose = generateToFirst[PAWN][onMove][1] & unoccupied;
	  dest = trythose & ~(FIRST_RANK | EIGHTH_RANK);
      fillMoveArray(&m, IN_HAND, PAWN, dest);
      }

   for (p = ROOK; p <= LAST_PIECE; p = (piece) (p + 1)) 
      {
      if (hand[onMove][p]) 
         {
		 trythose = generateToFirst[p][onMove][1] & unoccupied;
         fillMoveArray(&m, IN_HAND, p, trythose);
         }
      }

   /* Now the moves to squares that have been not as good in history are tried */

   /* Generate bishop/rook/queen moves bitboard by and-ing the attack bitboard
      with the empty squares */

   trythose = ~generateToFirst[BISHOP][onMove][0] & unoccupied;
  
   thePieces = pieces[BISHOP] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(BISHOP, sq) & trythose;
      fillMoveArray(&m, sq, BISHOP, dest);
      }

   trythose = ~generateToFirst[ROOK][onMove][0] & unoccupied;
   
   thePieces = pieces[ROOK] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(ROOK, sq) & trythose;
      fillMoveArray(&m, sq, ROOK, dest);
      }

   trythose = ~generateToFirst[QUEEN][onMove][0] & unoccupied;
   
   thePieces = pieces[QUEEN] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(QUEEN, sq) & trythose;
      fillMoveArray(&m, sq, QUEEN, dest);
      }

  /* Generate the knight moves bitboard by using the lookup table */

   trythose = ~generateToFirst[KNIGHT][onMove][0] & unoccupied;
   
   thePieces = pieces[KNIGHT] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = knightAttacks[sq] & trythose;
      fillMoveArray(&m, sq, KNIGHT, dest);
      }

  /* Generate pawn moves shifting the pawn bitboard by one rank and anding
     that with unoccupied squares, the from square is one rank before the to 
     square.
     Since captureMoves() generates promotions, take off any destination
     squares on the last rank.
     Then shift that bitboard forward one rank and and in it with unoccupied
     spaces on the 4th rank to get double pawn moves.
	 */

   trythose = ~generateToFirst[PAWN][onMove][0] & unoccupied;

   if (onMove == WHITE) 
      {
      dest = ((pieces[PAWN] & occupied[WHITE]) << ONE_RANK) & ~EIGHTH_RANK & trythose;

      //* Get the bitboard of double pawn moves 

      dest2 = ((((pieces[PAWN] & occupied[WHITE]) << ONE_RANK) & ~EIGHTH_RANK & unoccupied) << ONE_RANK) & FOURTH_RANK & trythose ;
                        
      while (dest.hasBits()) 
         {
         sq = firstSquare(dest.data);
         dest.unsetSquare(sq);
         *(m++) = move(sq - ONE_RANK, sq, PAWN);
         }

      while (dest2.hasBits()) 
         {
         sq = firstSquare(dest2.data);
         dest2.unsetSquare(sq);
         *(m++) = move(sq - TWO_RANKS, sq, PAWN);
         }
      }
   else 
      {
      dest = ((pieces[PAWN] & occupied[BLACK]) >> ONE_RANK) & ~FIRST_RANK & trythose;

      //* Get the bitboard of double pawn moves 

      dest2 = ((((pieces[PAWN] & occupied[BLACK]) >> ONE_RANK) & ~FIRST_RANK & unoccupied) >> ONE_RANK) & FIFTH_RANK & trythose;

      while (dest.hasBits()) 
         {
         sq = firstSquare(dest.data);
         dest.unsetSquare(sq);
         *(m++) = move(sq + ONE_RANK, sq, PAWN);
         }

      while (dest2.hasBits()) 
         {
         sq = firstSquare(dest2.data);
         dest2.unsetSquare(sq);
         *(m++) = move(sq + TWO_RANKS, sq, PAWN);
         }
      }



    // Get the dropping moves, the from square in IN_HAND the to square
    //  is any unoccupied square (except for the 1st, 8th rank for pawns). 

   if (hand[onMove][PAWN]) 
      {
	  trythose = ~generateToFirst[PAWN][onMove][1] & unoccupied;
      dest = trythose & ~(FIRST_RANK | EIGHTH_RANK);
      fillMoveArray(&m, IN_HAND, PAWN, dest);
      }

   for (p = ROOK; p <= LAST_PIECE; p = (piece) (p + 1)) 
      {
      if (hand[onMove][p]) 
         {
		 trythose = ~generateToFirst[p][onMove][1] & unoccupied;
         fillMoveArray(&m, IN_HAND, p, trythose);
         }
      }

   
   
   /* Get king moves.  This is just a lookup table. */

   sq = kingSquare[onMove];
   dest = kingAttacks[sq] & unoccupied;
   fillMoveArray(&m, sq, KING, dest);

  
   /* See if castle moves are possible by looking in canCastle and making sure
      the squares in between are okay to go through */
  
   if (onMove == WHITE && !isInCheck(WHITE)) 
      {
      if (canCastle[WHITE][KING_SIDE] && position[F1] == NONE && 
          position[G1] == NONE && attacks[BLACK][F1] == 0 &&
          attacks[BLACK][G1]== 0)
         *m++ = move(E1, G1, KING);

      if (canCastle[WHITE][QUEEN_SIDE] && position[D1] == NONE &&
            position[C1] == NONE && position[B1] == NONE &&
            attacks[BLACK][D1] == 0 && attacks[BLACK][C1] == 0)
         *m++ = move(E1, C1, KING);
      }
   else if (onMove == BLACK && !isInCheck(BLACK)) 
      {
      if (canCastle[BLACK][KING_SIDE] && position[F8] == NONE &&
             position[G8] == NONE && attacks[WHITE][F8] == 0 &&
             attacks[WHITE][G8] == 0)
         *m++ = move(E8, G8, KING);
    
      if (canCastle[BLACK][QUEEN_SIDE] && position[D8] == NONE &&
            position[C8] == NONE && position[B8] == NONE &&
            attacks[WHITE][D8] == 0 && attacks[WHITE][C8] == 0)
         *m++ = move(E8, C8, KING);
      }

  

   m->makeBad();
   return m - original;

	
	  }

/*
 * Function: mateTries
 * Input:    An array to fill with moves.
 * Output:   The number of moves generated.
 * Purpose:  generates contact checks where the checking piece can not be captured.
 */

int boardStruct::mateTries(move *m)
   {
   move *original;
   square sq;
   bitboard thePieces, dest, unoccupied, attksquare ;

   original = m;

   unoccupied = ~(occupied[WHITE] | occupied[BLACK]);   
     

   attksquare = unoccupied & contactAttacksFrom(QUEEN, kingSquare[OFF_MOVE]); 

   thePieces = pieces[QUEEN] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(QUEEN, sq) & attksquare;
      fillMoveArrayMateTriesOthers (&m, sq, QUEEN, dest, attacks, onMove);
      }

   attksquare = unoccupied & contactAttacksFrom(ROOK, kingSquare[OFF_MOVE]);

   thePieces = pieces[ROOK] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(ROOK, sq) & attksquare;
      fillMoveArrayMateTriesOthers (&m, sq, ROOK, dest, attacks, onMove);
      }   
   
   attksquare = unoccupied & contactAttacksFrom(KNIGHT, kingSquare[OFF_MOVE]);

   thePieces = pieces[KNIGHT] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = knightAttacks[sq] & attksquare;
      fillMoveArrayMateTriesKnights (&m, sq, dest, attacks, onMove);
      }

   
   attksquare = unoccupied & contactAttacksFrom(BISHOP, kingSquare[OFF_MOVE]); 
   
   thePieces = pieces[BISHOP] & occupied[onMove];
   while (thePieces.hasBits()) 
      {
      sq = firstSquare(thePieces.data);
      thePieces.unsetSquare(sq);
      dest = attacksFrom(BISHOP, sq) & attksquare;
      fillMoveArrayMateTriesOthers (&m, sq, BISHOP, dest, attacks, onMove);
      }
  

   if (hand[onMove][QUEEN]) 
   
   {		 
		 attksquare = unoccupied & contactAttacksFrom(QUEEN, kingSquare[OFF_MOVE]);
		 fillMoveArrayMateTriesOthers (&m, IN_HAND, QUEEN, attksquare, attacks, onMove); 
   }

   if (hand[onMove][ROOK]) 
   
   {		 
		 attksquare = unoccupied & contactAttacksFrom(ROOK, kingSquare[OFF_MOVE]);
		 fillMoveArrayMateTriesOthers (&m, IN_HAND, ROOK, attksquare, attacks, onMove); 
   }
   
   if (hand[onMove][KNIGHT]) 
   {
		 
		 attksquare = unoccupied & contactAttacksFrom(KNIGHT, kingSquare[OFF_MOVE]);
         fillMoveArrayMateTriesKnights (&m, IN_HAND, attksquare, attacks, onMove); 
   }
   
   if (hand[onMove][BISHOP]) 
   
   {		 
		 attksquare = unoccupied & contactAttacksFrom(BISHOP, kingSquare[OFF_MOVE]);
		 fillMoveArrayMateTriesOthers (&m, IN_HAND, BISHOP, attksquare, attacks, onMove); 
   }
   
   if (hand[onMove][PAWN]) 
      
   {
	  attksquare = unoccupied & pawnAttacksFrom(OFF_MOVE, kingSquare[OFF_MOVE]) ;
      dest = attksquare & ~(FIRST_RANK | EIGHTH_RANK);
      fillMoveArrayMateTriesOthers (&m, IN_HAND, PAWN, dest, attacks, onMove);
   }


   

   m->makeBad();
   return m - original;

	
	  }

/* 
 * Function: skippedMoves
 * Input:    An array to fill with moves.
 * Output:   The number of moves generated.
 * Purpose:  Generates the underpromotions that aiMoves skips
 */

int boardStruct::skippedMoves(move *m)
   {
   bitboard dest;
   move *original;

   original = m;

   if (onMove == WHITE) 
      {
      dest = EIGHTH_RANK;
      whitePawnMovesTo(&m, dest, ROOK);
      whitePawnMovesTo(&m, dest, BISHOP);
      }
   else 
      {
      dest = FIRST_RANK;
      blackPawnMovesTo(&m, dest, ROOK);
      blackPawnMovesTo(&m, dest, BISHOP);
      }
   m->makeBad();
   return m - original;
   }

