/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: board.cc                                                           *
 *  Purpose: Has the functions relating to the user interface.               *
 *                                                                           *
 *  Comments: board.cc has most of the primitives to access the board        *
 * structure.  All of the board's members are private, so to access them one *
 * of the primitives must be used.                                           *
 *                                                                           *
 *************************************************************************** */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "brain.h"
#include "interface.h"
#include "definitions.h"
#include "bughouse.h"
#include "variables.h"
#include "notation.h"

#define OFF_MOVE otherColor(onMove)

boardStruct gameBoard;
int hashMoveCircle = 0;

#ifdef GAMETREE
/*
 * Function: showPiece
 * Input: A square
 * Output: The Piece on that square
 * Prurpose: To display the current position.
 */

piece boardStruct::showPiece(int sq)
{
return position[sq];
}

/*
 * Function: showColor
 * Input: A square
 * Output: The Color on that square
 * Purpose: To display the current position.
 */

int boardStruct::showColor(int sq)
{
	if (occupied[BLACK].squareIsSet(sq)) { return BLACK ;}
	if (occupied[WHITE].squareIsSet(sq)) { return WHITE ;}
	return 3;
}

#endif

/* Book code by Angrim
char * boardStruct::getLineText()
{
	int n;
	static char buf[MAX_STRING];
	char tbuf[MAX_STRING];

	// kludge, 16 is a magic number, means 16 ply.
	if (moveNum>16) { strcpy(buf, "out of book"); return buf; }
	strcpy(buf, onMove == WHITE ? "W" : "B");
	for (n = 1; n<moveNum; n++) {
		strcat(buf, " ");
		DBMoveToRawAlgebraicMove(moveHistory[n], tbuf);
		strcat(buf, tbuf);
	}
	return buf;
}
*/ 


/* 
 * Function: addAttacks
 * Input:    A color a piece and a square
 * Output:   None.
 * Purpose:  Increments the attacks array on the squares that a piece attacks
 */

bitboard boardStruct::addAttacks(color c, piece p, square sq)
   {
   bitboard bb, bb2; 
	
   if (p == PAWN) 
   {
      bb = pawnAttacksFrom(c, sq);	  
   }
   else 
   {
      bb = attacksFrom(p, sq);	  
   }

   bb2 = bb; 

   while (bb.hasBits()) 
      {
      sq = firstSquare(bb.data);
      bb.unsetSquare(sq);
      attacks[c][sq]++;	  

      }

   return bb2;
   }


/* 
 * Function: removeAttacks
 * Input:    A color and a bitboard of squares
 * Output:   None.
 * Purpose:  Decrements the attacks array on squares that a piece attacks
 *
 */

bitboard boardStruct::removeAttacks(color c, piece p, square sq)
   {
   bitboard bb,bb2;
   square sq2;

   
   if (p == PAWN) 
      bb = pawnAttacksFrom(c, sq);
   else 
      bb = attacksFrom(p, sq);

   bb2 = bb;
   
   while (bb.hasBits()) 
      {
      sq2 = firstSquare(bb.data);
      bb.unsetSquare(sq2);
      attacks[c][sq2]--;

      }
   
   return bb2;
   }


/* 
 * Function: blockAttacks
 * Input:    A color and A square
 * Output:   None.
 * Purpose:  Decrements the attacks array on squares that aren't attacked 
 *           because a piece is blocking them.
 */

bitboard boardStruct::blockAttacks(color c, square where)
   {
   bitboard bb = (qword) 0;
   square sq;

   if (attacks[c][where]) {
   bb = blockedAttacks(c, where);
   while (bb.hasBits()) 
      {
      sq = firstSquare(bb.data);
      bb.unsetSquare(sq);
      attacks[c][sq]--;
      }
   }
   return bb; 
}

/* 
 * Function: uncoverAttacks
 * Input:    A color and A square
 * Output:   None.
 * Purpose:  Increments the attacks array on squares that are now attacked 
 *           because a piece isn't blocking them anymore.
 */

bitboard boardStruct::uncoverAttacks(color c, square where)
   {
   bitboard bb = (qword) 0;
   
   square sq;

   if (attacks[c][where])
   {
	   bb = blockedAttacks(c, where);
	   while (bb.hasBits())
	   {
		   sq = firstSquare(bb.data);
		   bb.unsetSquare(sq);
		   attacks[c][sq]++;
	   }
   }

   return bb; 
   }


/* 
 * Function: movePiece
 * Input:    A color and a piece to move and what square it should be moved 
 *           from and to and the hash value should be updated
 * Output:   None.
 * Purpose:  Used to move a piece
 */

void boardStruct::movePiece(color c, piece p, square from, square to, int hash, int attack)
   {
   if (attack)
   {
		moveAttackedSomething[moveNum+1] = ~  (removePiece(c, p, from, hash, attack));
		moveAttackedSomething[moveNum+1] &= addPiece(c, p, to, hash, attack);		   
   }
   else
   {
		removePiece(c, p, from, hash, attack);
	    addPiece(c, p, to, hash, attack);
   }

   }


/* 
 * Function: replacePiece
 * Input:    A color and a piece to replace another color and a piece,
 *           a square and if it should be hashed, and whether attack info
 *			 should be updated. 
 * Output:   None.
 * Purpose:  Used to replace one piece with another when a capture happens.
 */

bitboard boardStruct::replacePiece(color oldc, piece oldp, color newc, piece newp,
                      square sq, int hash, int attack)
   {
	bitboard bb = (qword) 0;
   
   position[sq] = newp;
   if (oldc == WHITE) 
      {
      material -= pValue[oldp];
      material -= pValue[newp];
      development -= DevelopmentTable[WHITE][oldp][sq];
      development -= DevelopmentTable[BLACK][newp][sq];
      } 
   else 
      {
      material += pValue[oldp];
      material += pValue[newp];
      development += DevelopmentTable[BLACK][oldp][sq];
      development += DevelopmentTable[WHITE][newp][sq];
      }
   removeFromBitboards(oldc, oldp, sq);
   
   if (attack) bb = removeAttacks(oldc, oldp, sq);
   
   addToBitboards(newc, newp, sq);
   
   if (attack) bb |= addAttacks(newc, newp, sq); 

   if (hash) 
      {
      addToHash(oldc, oldp, sq);
      addToHash(newc, newp, sq);
      }

   return bb;
   }


/* 
 * Function: removePiece
 * Input:    The color, piece and square to remove and if the hash value for
 *           the position should be changed.
 * Output:   None.
 * Purpose:  Used by functions in board.cc to update pieces if a piece has
 *           left the board.
 */

bitboard boardStruct::removePiece(color c, piece p, square sq, int hash, int attack)
   {

	bitboard bb = (qword) 0;

   position[sq] = NONE;
   if (c == WHITE) 
      {
      material -= pValue[p];
      development -= DevelopmentTable[WHITE][p][sq];
      } 
   else 
      {
      material += pValue[p];
      development += DevelopmentTable[BLACK][p][sq];
      }
   removeFromBitboards(c, p, sq);
   
   if (attack)
   {
   bb = removeAttacks(c, p, sq);
   uncoverAttacks(WHITE, sq);
   uncoverAttacks(BLACK, sq); 
   }

   if (hash) 
      addToHash(c, p, sq);

   return bb;
   }


/* 
 * Function: addPiece
 * Input:    The square to add the piece to, the piece to be add, the color
 *           of the piece to add, and if the hash value of the position
 *           should be changed.
 * Output:   None.
 * Purpose:  Used by functions in board.cc to update the board if a new piece
 *           comes on the board
*/

bitboard boardStruct::addPiece(color c, piece p, square sq, int hash, int attack)
   {

	bitboard bb = (qword) 0;

   position[sq] = p;
   if (c == WHITE) 
      {
      material += pValue[p];
      development += DevelopmentTable[WHITE][p][sq];
      } 
   else 
      {
      material -= pValue[p];
      development -= DevelopmentTable[BLACK][p][sq];
      }
   if (attack)
   {
   bb = addAttacks(c, p, sq);
   blockAttacks(WHITE, sq);
   blockAttacks(BLACK, sq);
   }
   addToBitboards(c, p, sq);
   
   if (hash) addToHash(c, p, sq);

   return bb; 
   }


/*
* Function: setBestCapture
* Input:    None.
* Output:   None.
* Purpose:  BestCaptureGain set to 0, else razoring is random as long
*			 as capture moves are not ordered yet (hash move or node
*           has no captures) .
*/

void boardStruct::setBestCapture()
{
	bestCaptureGain[moveNum] = 0;
}



/* 
 * Function: changeCastleOptions
 * Input:    A color, a side, the value to set it to and if the hash value
 *           of the position should be changed.
 * Output:   None.
 * Purpose:  Used to set a sides castling options.
 */

void boardStruct::setCastleOptions(color c, int side, int can, int hash)
   {
   if ((hash) && (canCastle[c][side] != can))
      {
		updateCastleHash(c, side);               /* Add the new hash value for
                                                        castling */ 
	  }
   canCastle[c][side] = byte (can);
   }

/* 
 * Function: getEval()
 * Input:    None.
 * Output:   None.
 * Purpose:  Prints the current evaluation split into components
 */


void boardStruct::getEval(char *buf)
{


  if(isCheckmate()) {
    sprintf(buf,"#");
    } else {
      sprintf(buf, "(M: %+i D: %+i C: %+i KW: %+i KB: %+i )", -
              playing == WHITE ? material : -material,
			  playing == WHITE ? development : -development,            
			  playing == WHITE ? boardControlEval() : -boardControlEval(),
			  playing == WHITE ? (-kingSafetyEval(WHITE)*getMaterialInHand(BLACK) ): (kingSafetyEval(WHITE)*getMaterialInHand(BLACK)),
			  playing == WHITE ? (kingSafetyEval(BLACK)*getMaterialInHand(WHITE) ): (-kingSafetyEval(BLACK)*getMaterialInHand(WHITE)));
    }    
  
}


/* 
 * Function: otherColor
 * Input:    A color.
 * Output:   The other color.
 * Purpose:  Used to get the other color.
 */

color otherColor(color c)
   {
   return (color) (BLACK - c);
   }


/* 
 * Function: addPieceToHand
 * Input:    A color, a piece and if the hash value of the position
 *           should be changed.
 * Output:   None.
 * Purpose:  Used to add a piece to a side's hand.
 */

void boardStruct::addPieceToHand(color c, piece p, int hash)
   {

   hand[c][p]++;
   if (c == WHITE) 
      material += pValue[p];
   else 
      material -= pValue[p];
   if (hash) 
      addToInHandHash(c, p);

   }


/* 
 * Function: removePieceFromHand
 * Input:    A color, a piece and if the hash value of the position should
 *           be changed
 * Output:   None.
 * Purpose:  Used to remove a piece to a side's hand.
 */

void boardStruct::removePieceFromHand(color c, piece p, int hash)
   {

   hand[c][p]--;
   if (c == WHITE) 
      material -= pValue[p];
   else 
      material += pValue[p];
   if (hash) 
      subtractFromInHandHash(c, p);

   }


/* 
 * Function: setPieceInHand
 * Input:    A color and a piece and the number of pieces that side has in hand
 * Output:   None.
 * Purpose:  Used to set the number of pieces a side has.
 */

void boardStruct::setPieceInHand(color c, piece p, int num)
   {
   int n;

   for (n = 0; n < hand[c][p]; n++)
      subtractFromInHandHash(c, p);
   for (n = 0; n < num; n++)
      addToInHandHash(c, p);
   if (c == WHITE)
      material += (num - hand[WHITE][p]) * pValue[p];
   else
      material -= (num - hand[BLACK][p]) * pValue[p];
   hand[c][p] = num;
   }


/*
 * Function: rank
 * Input:    A square.
 * Output:   The rank it's on.
 * Purpose:  Used to find the rank of a square.  The ranks start at 0.
 */

int rank(square sq)
   {
   return sq & 7;
   }


/*
 * Function: file
 * Input:    A square.
 * Output:   The file it's on.
 * Purpose:  Used to find the file of a square.  The files go from a=0 to h=7.
 */

int file(square sq)
   {
   return (sq & 0x38) >> 3;
   }


/*
 * Function: setTime
 * Input:    A color and an int.
 * Output:   None.
 * Purpose:  Used to set a color's time.  Time is specified in 1/100 of a
 *           second
 */

void boardStruct::setTime(color side, int time)
   {
   if (side == WHITE) 
      whiteTime = time;
   else
      blackTime = time;
   }


/*
 * Function: setDeepBugColor
 * Input:    A color.
 * Output:   None.
 * Purpose:  Used to change the color Sunsetter is playing.
 */

void boardStruct::setDeepBugColor(color c)
   {
   playing = c;
   }


/*
 * Function: setColorOnMove
 * Input:    A color.
 * Output:   None.
 * Purpose:  Used to change the color that has the move.
 */

void boardStruct::setColorOnMove(color c)
   {
	// is it not already that color to move ? Then dont clear the en passant square !
	if (onMove != c)
	{

		onMove = c;

		/* After changing the color to move there isn't any chance of en passant */

		enPassant = OFF_BOARD;
	}
   }

/*
 * Function: setCheckHistory
 * Input:    A color.
 * Output:   None.
 * Purpose:  Used to store whether we are in check 
 */

void boardStruct::setCheckHistory(int check)
   {
	checkHistory[moveNum] = check;

   }

/*
 * Function: resetBoard
 * Input:    None.
 * Output:   None.
 * Purpose:  Does everthing to the board to set up a new game.
 */

void boardStruct::resetBoard()
   {
   int n;
 
   color c;
   piece p;
	
   /* Set variables to starting values */   

   memset(takeBackHistory, 0, sizeof(takeBackHistory));
   memset(moveHistory, 0, sizeof(moveHistory));

   /* For move order */
   initializeHistory(); 
   
   /* Bughouse Garbage */
   psittinglong =  partsitting = toldpartisit = toldparttosit= parttoldgo = 0; 
   
   /* For the stats */
   stats_overallsearches = stats_overallqsearches = stats_overallticks = 0; 
   
   /* For the learning */
   firstBigValue = 0; 

#ifdef DEBUG_STATS
   stats_overallDebugA =  stats_overallDebugB = 0; 
#endif

   // @all: what is "custom" doing, please ? (georg)
   custom = false;
   moveNum = 1;
   onMove = WHITE;
   development = 0;
   
   hashMoveCircle++; 
   if (hashMoveCircle == 8) hashMoveCircle = 0; 

assert ((hashMoveCircle >= 0) && (hashMoveCircle <= 7));

   
   canCastle[BLACK][KING_SIDE] = canCastle[BLACK][QUEEN_SIDE] = 1;
   canCastle[WHITE][KING_SIDE] = canCastle[WHITE][QUEEN_SIDE] = 1;
   enPassant = OFF_BOARD;
   timeTaken = 0;
   playing = BLACK;
   blackTime = whiteTime = 60000;
   sitting = forceMode =  0;
   promotedPawns = qword(0);

   resetBitboards();

   /* set up the position */
	
     for (c = WHITE; c <= BLACK; c = (color) (c + 1)) 
   {
      for(p = PAWN; p <= QUEEN; p = (piece) (p + 1))
	  {
         gameBoard.setPieceInHand(c, p, 0);
	  }
   }
 
   
   kingSquare[WHITE] = E1;
   kingSquare[BLACK] = E8;

   memset(position, NONE, sizeof(position));

   for (n = 0; n < 8; n++)
      position[n * ONE_FILE + SIX_RANKS] = position[n * ONE_FILE + ONE_RANK] 
            = PAWN;
  
   position[A1] = position[A8] = position[H1] = position[H8] = ROOK;
   position[B1] = position[B8] = position[G1] = position[G8] = KNIGHT;
   position[C1] = position[C8] = position[F1] = position[F8] = BISHOP;
   position[D1] = position[D8] = QUEEN;
   position[E1] = position[E8] = KING;
  


   for (n = 0; n < 64; n++)
      {
      attacks[WHITE][n] = sword (((attacksTo(n) | (attacksFrom(KING, n) & pieces[KING])) & occupied[WHITE]).popCount());
      attacks[BLACK][n] = sword (((attacksTo(n) | (attacksFrom(KING, n) & pieces[KING])) & occupied[BLACK]).popCount());
      }

   material = 0; 
   initHash();



   lastMoveTime = getSysMilliSecs(); 
   }

int symbolColor(char symbol) {
	if ('A' <= symbol && symbol <= 'Z') return WHITE;
	else return BLACK;
}

piece symbolPiece(char symbol) {
	if ('a' <= symbol && symbol <= 'z') symbol = symbol - 'a' + 'A';
	if (symbol == 'P') return PAWN;
	else if (symbol == 'N') return KNIGHT;
	else if (symbol == 'B') return BISHOP;
	else if (symbol == 'R') return ROOK;
	else if (symbol == 'Q') return QUEEN;
	else {
		assert(symbol == 'K');
		return KING;
	}
}

// @all:
// this code needs validity checking of the input fen 

void boardStruct::setBoard(const char *fen, const char *turn, const char *castles, const char *ep) {
	// Reset
	initializeHistory();
	psittinglong = partsitting = toldpartisit = toldparttosit = parttoldgo = 0;
	stats_overallsearches = stats_overallqsearches = stats_overallticks = 0;
	firstBigValue = 0;
	moveNum = 1;
	custom = 1;

	memset(takeBackHistory, 0, sizeof(takeBackHistory));
	memset(moveHistory, 0, sizeof(moveHistory));

	material = 0;
	development = 0;

#ifdef DEBUG_STATS
	stats_overallDebugA = stats_overallDebugB = 0;
#endif

	
	hashMoveCircle++;
	if (hashMoveCircle == 8) hashMoveCircle = 0;

	promotedPawns = qword(0);
	occupied[WHITE] = qword(0);
	occupied[BLACK] = qword(0);
	pieces[PAWN] = pieces[ROOK] = pieces[KNIGHT] = pieces[BISHOP] = pieces[QUEEN] = pieces[KING] = qword(0);
	for (int c = WHITE; c <= BLACK; c++) {
		for (piece p = PAWN; p <= QUEEN; p++) {
			setPieceInHand(c, p, 0);
		}
	}
	memset(position, NONE, sizeof(position));

	// Pieces
	square sq = A8;
	int part = 0;
	const char *ch;
	for (ch = fen; *ch; ch++) 
	{
		if ('0' <= *ch && *ch <= '9') 
		{
			sq += 8 * (*ch - '0');
		}
	
		else if (*ch == '[') // XB
		{
			ch++;
			break;
		}

		else if (*ch == '/') 
		{
			part++;
			if (part >= 8) 
			{
				ch++;
				break;
			}
			sq -= 57 + 8;
		}
		else if (*ch == '~') 
		{
			promotedPawns.setSquare(sq - 8);
		}
		else 
		{
			if (*ch == 'k') kingSquare[BLACK] = sq;
			else if (*ch == 'K') kingSquare[WHITE] = sq;

			addPiece(symbolColor(*ch), symbolPiece(*ch), sq, 0, 0);
			sq += 8;
		}
	}
	for (; *ch; ch++) 
	{
		if (*ch == ']')
			break;
		if (*ch == '-')
			break;

		addPieceToHand(symbolColor(*ch), symbolPiece(*ch), 0);
	}

	// Update bitboards
	// this might not be needed because addPiece() already updates the diagonal bitboard, but this needs testing (georg)
	updateDiagonalBitboards();

	// Turn
	onMove = (turn[0] == 'b') ? BLACK : WHITE;

	// Castles.
	canCastle[BLACK][KING_SIDE] = canCastle[BLACK][QUEEN_SIDE] = 0;
	canCastle[WHITE][KING_SIDE] = canCastle[WHITE][QUEEN_SIDE] = 0;

	bool wk = isPieceOnSquare(E1, KING, WHITE);
	bool bk = isPieceOnSquare(E8, KING, BLACK);

	for (const char *ch = castles; *ch; ch++) {
		if (*ch == 'K' && wk && isPieceOnSquare(H1, ROOK, WHITE)) {
			canCastle[WHITE][KING_SIDE] = 1;
		}
		else if (*ch == 'Q' && wk && isPieceOnSquare(A1, ROOK, WHITE)) {
			canCastle[WHITE][QUEEN_SIDE] = 1;
		}
		else if (*ch == 'k' && bk && isPieceOnSquare(H8, ROOK, BLACK)) {
			canCastle[BLACK][KING_SIDE] = 1;
		}
		else if (*ch == 'q' && bk && isPieceOnSquare(A8, ROOK, BLACK)) {
			canCastle[BLACK][QUEEN_SIDE] = 1;
		}
	}

	// En passant
	if ('a' <= ep[0] && ep[0] <= 'h' && (ep[1] == '3' || ep[1] == '6')) {
		int file = ep[0] - 'a';
		int rank = ep[1] - '0';
		enPassant = rank * 8 + file;
	}
	else enPassant = OFF_BOARD;

	// Meta data
	timeTaken = 0;
	playing = !onMove;
	blackTime = whiteTime = 60000;
	sitting = forceMode = 0;


	for (int n = 0; n < 64; n++) {
		attacks[WHITE][n] = sword(((attacksTo(n) | (attacksFrom(KING, n) & pieces[KING])) & occupied[WHITE]).popCount());
		attacks[BLACK][n] = sword(((attacksTo(n) | (attacksFrom(KING, n) & pieces[KING])) & occupied[BLACK]).popCount());
	}

	// remember to also zap the hash values (this is currently correctly done in interface.cpp when this function is called)
	initHash();
	lastMoveTime = getSysMilliSecs();

	// @georg please review:

	// check history ?
}



void boardStruct::setLastMoveNow()

{
lastMoveTime = getSysMilliSecs(); 
return;
}

/* 
 * Function: copy
 * Input:    A pointer to the board to copy the board to.
 * Output:   None
 * Purpose:  Copies it's own board to another one.
 */

void boardStruct::copy(boardStruct *dest)
   {
   memcpy(dest, this, sizeof(boardStruct));
   return;
   }


/*
 * Function: playMove
 * Input:    A move pointer and whether to give the move to Xboard
 * Output:   -1 if the move was illegal, 0 otherwise.
 * Purpose:  Plays a move on the board.
 */

int boardStruct::playMove(move m, int report)
   {


   if (!isLegal(m))
      return -1;

   changeBoard(m); 


   if (currentRules == BUGHOUSE)
      { 
				/* Do stuff to keep track of promotions.  
                This is done is changeBoard() in crazyhouse
                but for bug it's not so importantt for the
                search, so time isn't taken up in
                changeBoard() and it's done here. */

	   // promoted pawns fixed in zh as of 30/08/16, compare if needs fixing here?  (GZ)

      if (promotedPawns.squareIsSet(m.to()))
         {
         promotedPawns.unsetSquare(m.to());
         }

    
      if (promotedPawns.squareIsSet(m.from()))
         {
         promotedPawns.unsetSquare(m.from());
         promotedPawns.setSquare(m.to());
         }

      if (m.promotion() != NONE)
         promotedPawns.setSquare(m.to());
      }

   
	if (report == 1) giveMove(m);

	if (gameBoard.isCheckmate() && (!analyzeMode))
		{
			forceMode = 1; 
			if (onMove == WHITE)
				reportResult(BLACK_MATE);
			else
				reportResult(WHITE_MATE);
			gameInProgress = 0; 
		}



   lastMoveTime = getSysMilliSecs(); 
   
   hashMoveCircle++; 
   if (hashMoveCircle == 8) hashMoveCircle = 0; 

assert ((hashMoveCircle >= 0) && (hashMoveCircle <= 7));
   
   return 0;
   }


/*
 * Function: unplayMove
 * Input:    None.
 * Output:   -1 if unsucessfull, 0 otherwise.
 * Purpose:  Takes back a move on the board.  It's not really implemented well
 *           (it doesn't change the time or take back moves on the other board
 *           since the move).
 */

int boardStruct::unplayMove()
   {
   if (moveNum == 1 && onMove == WHITE)
      return -1;
      
   unchangeBoard();

   lastMoveTime = getSysMilliSecs(); 
   
   hashMoveCircle--; 
   if (hashMoveCircle == -1) hashMoveCircle = 7; 

assert ((hashMoveCircle >= 0) && (hashMoveCircle <= 7));
   return 0;
   }

#ifdef GAMETREE

move boardStruct::getMoveHistory (int ply)
{

	return moveHistory[gameBoard.getMoveNum()+ply];
}

#endif

/*
 * Function: makeNullMove
 * Input:    None.
 * Purpose:  Makes a nullmove
 */

void boardStruct::makeNullMove()
   {
		takeBackHistory[moveNum].oldep = enPassant;
		takeBackHistory[moveNum].captured = NONE;
		
		addEnPassantHash(enPassant); // remove current ep square
		enPassant = OFF_BOARD;
		addEnPassantHash(enPassant); // add OFF_Board = after a null move ep can't be possible

		onMove = OFF_MOVE;    
		moveNum++;

		moveAttackedSomething[moveNum] = qword (0); 

   }


/*
 * Function: unmakeNullMove
 * Input:    None.
 * Purpose:  "Takes back" a nullmove
 */

void boardStruct::unmakeNullMove()
   {
		
		moveNum--;

		onMove = OFF_MOVE;
		
		addEnPassantHash(enPassant);	// remove the OFF_Board ep
		enPassant = takeBackHistory[moveNum].oldep;
		addEnPassantHash(enPassant);
   }



/*
 * Function: playBughouse
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to tell the board to use bughouse rules
 */

void boardStruct::playBughouse()
   {
   currentChangeBoard = &boardStruct::bugChangeBoard;
   currentUnchangeBoard = &boardStruct::bugUnchangeBoard;
   }


/*
 * Function: playCrazyhouse
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to tell the board to use crazyhouse rules
 */

void boardStruct::playCrazyhouse()
   {
   currentChangeBoard = &boardStruct::ZHChangeBoard;
   currentUnchangeBoard = &boardStruct::ZHUnchangeBoard;
   }


/*
 * Function: changeBoard
 * Input:    A move and a takeBackInfo pointer
 * Output:   None.
 * Purpose:  Use to play a move on the board.  It is a quick low level version 
 *           of playMove it doesn't take time to do things  like check to see
 *           if a move is legal.  Also it doesn't tell Xboard the move.
 *           This is just a wrapper function that calls whichever changeBoard
 *           that we're curently using (either the bughouse or crazyhouse one)
 */

void boardStruct::changeBoard(move m)
   {
   (this->*currentChangeBoard)(m);
   }


/*
 * Function: unchangeBoard
 * Input:    A move and a takeBackInfo pointer
 * Output:   None.
 * Purpose:  Used by the AI to take back a move that's been played.
 *           It does the steps of changeBoard pretty much in reverse.
 *           This is just a wrapper function that call the current version of
 *           unchangeBoard (either the bughouse or crazyhouse one)
 */

void boardStruct::unchangeBoard()
   {
   (this->*currentUnchangeBoard)();   
   }


/*
 * Function: bugChangeBoard
 * Input:    A move and a takeBackInfo pointer
 * Output:   None.
 * Purpose:  
 */

void boardStruct::bugChangeBoard(move m)
   {

   /* Save the information needed for when
      unchangeBoard() has to restore the position. 
      (capture has to be done after the check to move the pawn back because
      of en passant) */  

   memcpy(takeBackHistory[moveNum].oldCastle, canCastle, sizeof(takeBackHistory[moveNum].oldCastle));
   memcpy(takeBackHistory[moveNum].attacks,attacks,  sizeof(attacks));
  
   takeBackHistory[moveNum].oldep = enPassant;
   takeBackHistory[moveNum].oldHash = hashValue;

#ifdef DEBUG_HASH
   takeBackHistory[moveNum].oldHashT = hashValueT; 
#endif
   

   /*  If the move was capturing en passant, then put the captured pawn to 
       where it would be if it only moved one square */

   if (m.moved() == PAWN && m.to() == enPassant && m.from() != IN_HAND) 
      {

      /* Right now en passant is shakey so check to see that it really is legal 
         */

      if (onMove == WHITE) 
         {
         movePiece(BLACK, PAWN, enPassant - 1, enPassant, 1, 1);
         } 
      else 
         {
         movePiece(WHITE, PAWN, enPassant + 1, enPassant, 1, 1);
         }
      }

   takeBackHistory[moveNum].captured = position[m.to()]; 

   /* Handle drop moves */

   if (m.from() == IN_HAND) 
      {
      moveAttackedSomething[moveNum+1] = addPiece(onMove, m.moved(), m.to(), 1, 1);
      removePieceFromHand(onMove, m.moved(), 1);

      /* en passant is not possible after a drop move */

      addEnPassantHash(enPassant);
      addEnPassantHash(OFF_BOARD);
      enPassant = OFF_BOARD;
      onMove = OFF_MOVE;
	  moveHistory[moveNum] = m;
	  moveNum++;
      return;
      }

   if (takeBackHistory[moveNum].captured == NONE) 
      {
      movePiece(onMove, m.moved(), m.from(), m.to(), 1, 1);
      }
   else 
      {
      moveAttackedSomething[moveNum+1] = ~(removePiece(onMove, m.moved(), m.from(), 1, 1));
      moveAttackedSomething[moveNum+1] &= replacePiece(OFF_MOVE, takeBackHistory[moveNum].captured, onMove, m.moved(), m.to(), 1, 1);
      }

   /* If the move was a pawn promotion then:
      Remove the pawn from the board
      Add the promoted piece to the board */

   if (m.promotion() != NONE) 
      {
	
   /* We are also keeping track of what new squares the last move attacked.
	  So we AND the bitboard of newly attacked squares with the reverse
	  of what the piece already attacked where it came from. */


      moveAttackedSomething[moveNum+1] = ~(removePiece(onMove, PAWN, m.to(), 1, 1));
      moveAttackedSomething[moveNum+1] &= addPiece(onMove, m.promotion(), m.to(), 1, 1);
      }


   /* Do special things for casling and en passant:
      If the move was castles move the rook to the other side of the king
      Decide if the move affected castling
      Decide if the move affected en passant
      Update the castle and en passant hash value by removing the current
      value and adding the new value */

   /* Remove the current en passant square from the hash value */

   addEnPassantHash(enPassant);

   /* Assume for now that the move didn't allow the possibility of en passant */

   enPassant = OFF_BOARD;
 
   if (takeBackHistory[moveNum].captured == ROOK &&
        m.to() == kingSquare[OFF_MOVE] + THREE_FILES) 
      {
      setCastleOptions(OFF_MOVE, KING_SIDE, 0, 1);
      }
   else if (takeBackHistory[moveNum].captured == ROOK &&
            m.to() == kingSquare[OFF_MOVE] - FOUR_FILES) 
      {
      setCastleOptions(OFF_MOVE, QUEEN_SIDE, 0, 1);
      } 
  
   if (m.moved() == KING) 
      {
      if (m.to() == m.from() + TWO_FILES)
         {
         movePiece(onMove, ROOK, m.from() + THREE_FILES,m.from()+ONE_FILE, 1, 1);
         }
      else if (m.to() == m.from() - TWO_FILES) 
         {
         movePiece(onMove, ROOK, m.from() - FOUR_FILES, m.from()-ONE_FILE, 1, 1);
         }
      setCastleOptions(onMove, KING_SIDE, 0, 1);
      setCastleOptions(onMove, QUEEN_SIDE, 0, 1);
      kingSquare[onMove] = m.to();
      } 
   else if (m.moved() == ROOK) 
      {
      if (onMove == WHITE) 
         {
         if (m.from() == A1) 
            setCastleOptions(WHITE, QUEEN_SIDE, 0, 1);
         if (m.from() == H1) 
            setCastleOptions(WHITE,KING_SIDE,0,1);
         } 
      else 
         {
         if (m.from() == A8) 
            setCastleOptions(BLACK, QUEEN_SIDE, 0, 1);
         if (m.from() == H8) 
            setCastleOptions(BLACK, KING_SIDE, 0, 1);
         }
      } 
   else if (m.moved() == PAWN) 
      {
      /* This is kinda tricky since a pawn can be dropped on the board.  But if
         it is then from is 65 and to will never be within 2 of it */

      if (m.to() == m.from() + TWO_RANKS) 
         enPassant = m.to() - ONE_RANK;
      else if (m.to() == m.from() - TWO_RANKS)
         enPassant = m.to() + ONE_RANK;
      }

   /* Add the new en passant square to the hash value */

   addEnPassantHash(enPassant);

   /*  Change the color on move */

   onMove = OFF_MOVE;
  
   moveHistory[moveNum] = m;
   moveNum++;
  
   return;
   }


/* 
 * Function: ZHChangeBoard
 * Input:    A move and a takeBackInfo pointer
 * Output:   None.
 * Purpose:  Use to play a move on the board in crazyhouse.  It is a quick low
 *           level version of playMove it doesn't take time to do things like
 *           check to see if a move is legal.  Also it doesn't tell Xboard the
 *           move.
 *           Differences in the crazyhouse version of changeBoard:
 *           1) If we capture a piece then we add it to the players hand
 *           2) We have to keep track of promotions so when we capture a
 *              promoted pawn all we get is a pawn.
 *            
 *              Warning:  We don't call addPieceToHand() just
 *              increment the hand array.  This is because addPieceToHand()
 *              also adds material to the side getting the piece and updates
 *              the hash value, neither things are nessesary or wanted in
 *              crazyhouse
 */

void boardStruct::ZHChangeBoard(move m)
   {   

   int capturedPromotedPawn;

   /* See if we captured or moved a promoted pawn */

   if (promotedPawns.squareIsSet(m.to())) 
      {
      promotedPawns.unsetSquare(m.to());
      capturedPromotedPawn = 1;
      } 
   else  
      capturedPromotedPawn = 0;

   if ((m.from() != IN_HAND) && (promotedPawns.squareIsSet(m.from())))
      {
      promotedPawns.unsetSquare(m.from());
      promotedPawns.setSquare(m.to());
      }

   /* Save the information needed for when
      unchangeBoard() has to restore the position. */

   memcpy(takeBackHistory[moveNum].oldCastle, canCastle, sizeof(takeBackHistory[moveNum].oldCastle));
   memcpy(takeBackHistory[moveNum].attacks,attacks,  sizeof(attacks));

   takeBackHistory[moveNum].oldep = enPassant;
   takeBackHistory[moveNum].oldHash = hashValue;

#ifdef DEBUG_HASH
   takeBackHistory[moveNum].oldHashT = hashValueT; 
#endif

   takeBackHistory[moveNum].capturedPromotedPawn = byte (capturedPromotedPawn);

   /*  If the move was capturing en passant, then put the captured pawn to 
       where it would be if it only moved one square */

   if ( m.to() == enPassant && m.moved() == PAWN && m.from() != IN_HAND) 
      {

      /* Right now en passant is shakey so check to see that it really is legal 
       */

      if (onMove == WHITE) 
         {
         movePiece(BLACK, PAWN, enPassant - 1, enPassant, 1, 1);
         } 
      else 
         {
         movePiece(WHITE, PAWN, enPassant + 1, enPassant, 1, 1);
         }
      }

   /* Save the information needed for when
      unchangeBoard() has to restore the position. 
      (capture has to be done after the check to move the pawn back because
      of en passant) */  

   takeBackHistory[moveNum].captured = position[m.to()];

   /* Handle drop moves */

   if (m.from() == IN_HAND) 
      {
      moveAttackedSomething[moveNum+1] = addPiece(onMove, m.moved(), m.to(), 1, 1);
      removePieceFromHand(onMove, m.moved(), 1);

      /* en passant is not possible after a drop move */

      addEnPassantHash(enPassant);
      addEnPassantHash(OFF_BOARD);
      enPassant = OFF_BOARD;
      onMove = OFF_MOVE;
	  moveHistory[moveNum] = m;
	  moveNum++;
      return;
      }

   /* See if we moved a promoted pawn */

   if (takeBackHistory[moveNum].captured == NONE) 
      {
      movePiece(onMove, m.moved(), m.from(), m.to(), 1, 1);
      }
   else 
      {
      moveAttackedSomething[moveNum+1] = ~(removePiece(onMove, m.moved(), m.from(), 1, 1));
      moveAttackedSomething[moveNum+1] &= replacePiece(OFF_MOVE, takeBackHistory[moveNum].captured, onMove, m.moved(), m.to(), 1, 1);
      
	  if (!capturedPromotedPawn) 
	  {
         hand[onMove][takeBackHistory[moveNum].captured]++;
		 addToInHandHash(onMove,takeBackHistory[moveNum].captured );
	  }
      else 
	  {
         hand[onMove][PAWN]++;
		 addToInHandHash(onMove,PAWN );
	  }
      }


   /* If the move was a pawn promotion then:
      Remove the pawn from the board
      Add the promoted piece to the board 
      Keep track of where it is */

   if (m.promotion() != NONE) 
      {

   /* We are also keeping track of what new squares the last move attacked.
	  So we AND the bitboard of newly attacked squares with the reverse
	  of what the piece already attacked where it came from. */

      moveAttackedSomething[moveNum+1] = ~(removePiece(onMove, PAWN, m.to(), 1, 1));
      moveAttackedSomething[moveNum+1] &= addPiece(onMove, m.promotion(), m.to(), 1, 1);
      promotedPawns.setSquare(m.to());
      }

   /* Do special things for castling and en passant:
      If the move was castles move the rook to the other side of the king
      Decide if the move affected castling
      Decide if the move affected en passant
      Update the castle and en passant hash value by removing the current
      value and adding the new value */

   /* Remove the current en passant square from the hash value */

   addEnPassantHash(enPassant);

   /* Assume for now that the move didn't allow the possibility of en passant */

   enPassant = OFF_BOARD;
 
   if (takeBackHistory[moveNum].captured == ROOK &&
      m.to() == kingSquare[OFF_MOVE] + THREE_FILES) 
      {
      setCastleOptions(OFF_MOVE, KING_SIDE, 0, 1);
      } 
   else if (takeBackHistory[moveNum].captured == ROOK &&
            m.to() == kingSquare[OFF_MOVE] - FOUR_FILES) 
      {
      setCastleOptions(OFF_MOVE, QUEEN_SIDE, 0, 1);
      } 
  
   if (m.moved() == KING) 
      {
      if (m.to() == m.from() + TWO_FILES) 
         {
         movePiece(onMove, ROOK, m.from() + THREE_FILES,m.from()+ONE_FILE, 1, 1);
         } 
      else if (m.to() == m.from() - TWO_FILES) 
         {
         movePiece(onMove, ROOK, m.from() - FOUR_FILES, m.from()-ONE_FILE, 1, 1);
         }
      setCastleOptions(onMove, KING_SIDE, 0, 1);
      setCastleOptions(onMove, QUEEN_SIDE, 0, 1);
      kingSquare[onMove] = m.to();
      } 
   else if (m.moved() == ROOK) 
      {
      if (onMove == WHITE) 
         {
         if (m.from() == A1) 
            setCastleOptions(WHITE, QUEEN_SIDE, 0, 1);
         if (m.from() == H1) 
            setCastleOptions(WHITE,KING_SIDE,0,1);
         } 
      else 
         {
         if (m.from() == A8) 
            setCastleOptions(BLACK, QUEEN_SIDE, 0, 1);
         if (m.from() == H8) 
            setCastleOptions(BLACK, KING_SIDE, 0, 1);
         }
      } 
   else if (m.moved() == PAWN) 
      {
      /* This is kinda tricky since a pawn can be dropped on the board.  But if
         it is then from is 65 and to will never be within 2 of it */
      if (m.to() == m.from() + TWO_RANKS) 
         enPassant = m.to() - ONE_RANK;
      else if (m.to() == m.from() - TWO_RANKS)
         enPassant = m.to() + ONE_RANK;
      }

   /* Add the new en passant square to the hash value */

   addEnPassantHash(enPassant);

   /*  Change the color on move */

   onMove = OFF_MOVE;

   moveHistory[moveNum] = m;
   moveNum++;
  
   
   return;
   }



/*
 * Function: bugUnchangeBoard
 * Input:    A Move pointer
 * Output:   None.
 * Purpose:  Used by the AI to take back a move that's been played.
 *           It does the steps of changeBoard pretty much in reverse.
 *           This one is for when Sunsetter is playing bughouse.  When it's
 *           playing crazyhouse it uses ZHUnchangeBoard
 */

void boardStruct::bugUnchangeBoard()
   {

    moveNum--;
	
	/* Change the color to move */

   onMove = OFF_MOVE;  

   /* Handle drop moves */

   if (moveHistory[moveNum].from() == IN_HAND) 
      {
      removePiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].to(), 0, 0);
      position[moveHistory[moveNum].to()] = NONE;
      addPieceToHand(onMove, moveHistory[moveNum].moved(), 0);
      enPassant = takeBackHistory[moveNum].oldep;
      hashValue = takeBackHistory[moveNum].oldHash;
#ifdef DEBUG_HASH
	  hashValueT = takeBackHistory[moveNum].oldHashT; 
#endif

	   memcpy(attacks,takeBackHistory[moveNum].attacks,  sizeof(attacks));

      return;
      }

   /* If the move was castles move the rook back to it's starting square */

   if (moveHistory[moveNum].moved() == KING) 
      {
      if (moveHistory[moveNum].to() == moveHistory[moveNum].from() + TWO_FILES) 
         {
         movePiece(onMove, ROOK, moveHistory[moveNum].from() + ONE_FILE,
         moveHistory[moveNum].from() + THREE_FILES, 0, 0);
         } 
      else if (moveHistory[moveNum].to() == moveHistory[moveNum].from() - TWO_FILES) 
         {
         movePiece(onMove, ROOK, moveHistory[moveNum].from() - ONE_FILE,
         moveHistory[moveNum].from() - FOUR_FILES, 0, 0);
         }
      kingSquare[onMove] = moveHistory[moveNum].from();
      }

   /* If the move was a pawn promotion then:
      Remove the promoted piece from the board
      Add the pawn to the board */
 
   if (moveHistory[moveNum].promotion() != NONE) 
      {
      removePiece(onMove, moveHistory[moveNum].promotion(), moveHistory[moveNum].to(), 0, 0);
      addPiece(onMove, PAWN, moveHistory[moveNum].to(), 0, 0);
      }
   if (takeBackHistory[moveNum].captured == NONE) 
      {
      movePiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].to(), moveHistory[moveNum].from(), 0, 0);
      } 
   else 
      {
      replacePiece(onMove, moveHistory[moveNum].moved(), OFF_MOVE, takeBackHistory[moveNum].captured, moveHistory[moveNum].to(), 0, 0);
      addPiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].from(), 0, 0);
      }

   /* Restore the saved information */

   memcpy(attacks,takeBackHistory[moveNum].attacks,  sizeof(attacks));

   setCastleOptions(WHITE, KING_SIDE, takeBackHistory[moveNum].oldCastle[WHITE][KING_SIDE], 0);
   setCastleOptions(WHITE, QUEEN_SIDE, takeBackHistory[moveNum].oldCastle[WHITE][QUEEN_SIDE], 0);
   setCastleOptions(BLACK, KING_SIDE, takeBackHistory[moveNum].oldCastle[BLACK][KING_SIDE], 0);
   setCastleOptions(BLACK, QUEEN_SIDE, takeBackHistory[moveNum].oldCastle[BLACK][QUEEN_SIDE], 0);
   enPassant = takeBackHistory[moveNum].oldep;
   hashValue = takeBackHistory[moveNum].oldHash;
#ifdef DEBUG_HASH
   hashValueT = takeBackHistory[moveNum].oldHashT; 
#endif

  

   /* If the move was capturing en passant then put the pawn back two
      squares advanced */

   if (takeBackHistory[moveNum].captured == PAWN && moveHistory[moveNum].to() == enPassant) 
      {
      if (onMove == WHITE) 
         {
         movePiece(BLACK, PAWN, enPassant, enPassant - 1, 0, 0);
         }
      else 
         {
         movePiece(WHITE, PAWN, enPassant, enPassant + 1, 0, 0);
         }
      }

   return;
   }


/* 
 * Function: ZHUnchangeBoard
 * Input:    A Move pointer
 * Output:   None.
 * Purpose:  Used by the AI to take back a move that's been played.
 *           It does the steps of changeBoard pretty much in reverse.
 *           This one is for when Sunsetter is playing crazyhouse.
 *           the differences are:
 *           1) If we captured a piece then we remove it to the players hand
 *           2) We have to keep track of promotions so when we capture a
 *              promoted pawn all we get is a pawn.
 *            
 *              Warning:  We don't call addPieceToHand() just
 *              increment the hand array.  This is because addPieceToHand()
 *              also adds material to the side getting the piece and updates
 *              the hash value, neither things are nessesary or wanted in
 *              crazyhouse
 */

void boardStruct::ZHUnchangeBoard()
   {  
		    
    moveNum--;
	
	
	/* Change the color to move */
  
   onMove = OFF_MOVE;  

   /* Handle drop moves */

   if (moveHistory[moveNum].from() == IN_HAND) 
      {
      removePiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].to(), 0, 0);
      position[moveHistory[moveNum].to()] = NONE;
      addPieceToHand(onMove, moveHistory[moveNum].moved(), 0);
      enPassant = takeBackHistory[moveNum].oldep;
      hashValue = takeBackHistory[moveNum].oldHash;
#ifdef DEBUG_HASH
	  hashValueT = takeBackHistory[moveNum].oldHashT; 
#endif
	  memcpy(attacks,takeBackHistory[moveNum].attacks,  sizeof(attacks));
      
	  return;
      }

   /* If the move castled move the rook back to it's starting square */

   if (moveHistory[moveNum].moved() == KING) 
      {
      if (moveHistory[moveNum].to() == moveHistory[moveNum].from() + TWO_FILES) 
         {
         movePiece(onMove, ROOK, moveHistory[moveNum].from() + ONE_FILE,
         moveHistory[moveNum].from() + THREE_FILES, 0, 0);
         }
      else if (moveHistory[moveNum].to() == moveHistory[moveNum].from() - TWO_FILES) 
         {
         movePiece(onMove, ROOK, moveHistory[moveNum].from() - ONE_FILE,
         moveHistory[moveNum].from() - FOUR_FILES, 0, 0);
         }
      kingSquare[onMove] = moveHistory[moveNum].from();
      }

   /* If the move was a pawn promotion then:
      Remove the promoted piece from the board
      Add the pawn to the board */
 
   if (moveHistory[moveNum].promotion() != NONE) 
      {
      removePiece(onMove, moveHistory[moveNum].promotion(), moveHistory[moveNum].to(), 0, 0);
      addPiece(onMove, PAWN, moveHistory[moveNum].to(), 0, 0);
      promotedPawns.unsetSquare(moveHistory[moveNum].to());
      }

   /* See if we moved a promoted pawn */

   if (promotedPawns.squareIsSet(moveHistory[moveNum].to())) 
      {
      promotedPawns.unsetSquare(moveHistory[moveNum].to());
      promotedPawns.setSquare(moveHistory[moveNum].from());
      }

   
   /* Normal Move */
   
   if (takeBackHistory[moveNum].captured == NONE)
      {
      movePiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].to(), moveHistory[moveNum].from(), 0, 0);
      }
   else 
      {
      replacePiece(onMove, moveHistory[moveNum].moved(), OFF_MOVE, takeBackHistory[moveNum].captured, moveHistory[moveNum].to(), 0, 0);
      addPiece(onMove, moveHistory[moveNum].moved(), moveHistory[moveNum].from(), 0, 0);
      if (!takeBackHistory[moveNum].capturedPromotedPawn) 
         {
         hand[onMove][takeBackHistory[moveNum].captured]--;
         }
      else 
         {
         hand[onMove][PAWN]--;
         promotedPawns.setSquare(moveHistory[moveNum].to());
         }
      }

   /* Restore the saved information */

   
   memcpy(attacks,takeBackHistory[moveNum].attacks,  sizeof(attacks));
   
   setCastleOptions(WHITE, KING_SIDE, takeBackHistory[moveNum].oldCastle[WHITE][KING_SIDE], 0);
   setCastleOptions(WHITE, QUEEN_SIDE, takeBackHistory[moveNum].oldCastle[WHITE][QUEEN_SIDE], 0);
   setCastleOptions(BLACK, KING_SIDE, takeBackHistory[moveNum].oldCastle[BLACK][KING_SIDE], 0);
   setCastleOptions(BLACK, QUEEN_SIDE, takeBackHistory[moveNum].oldCastle[BLACK][QUEEN_SIDE], 0);

   enPassant = takeBackHistory[moveNum].oldep;
   hashValue = takeBackHistory[moveNum].oldHash;
#ifdef DEBUG_HASH   
   hashValueT = takeBackHistory[moveNum].oldHashT; 
#endif

   /* If the move was capturing en passant then put the pawn back two
      squares advanced */

   if (takeBackHistory[moveNum].captured == PAWN && moveHistory[moveNum].to() == enPassant) 
      {
      if (onMove == WHITE) 
         {
         movePiece(BLACK, PAWN, enPassant, enPassant - 1, 0, 0);
         }
      else 
         {
         movePiece(WHITE, PAWN, enPassant, enPassant + 1, 0, 0);
         }
      }

   
   return;
   }




/*
 * Function: getTime
 * Input:    A color.
 * Output:   The amount of time that color has
 * Purpose:  Used for reading the clock.  Be carefull.  It's only updated
 *           after a new, so it will give the wrong time.
 *           Also the time is given in 1/100s of a second.
 */

int boardStruct::getTime(color c)
   {
   if (c == WHITE)
      return whiteTime;
   else
      return blackTime;
   }


/*
 * Function: outOfTime
 * Input:    A color.
 * Output:   If that color is out of time
 * Purpose:  To see if one of the flags have fallen.
 */

int boardStruct::outOfTime(color c)
   {

   double currentTime; 
   double usedTime;

   if (c != onMove)
      {
      if (getTime(c) < 0)
         return 1;
      return 0;
      }

   currentTime = getSysMilliSecs(); 
   usedTime = currentTime-lastMoveTime; 
   
   if (c == WHITE && (usedTime  > whiteTime))
      {
      return 1;
      }
  else if (c == BLACK && (usedTime  > blackTime))
      {
      return 1;
      }
   return 0;
   }


/*
 * Function: timeToMove
 * Input:    None.
 * Output:   1 if Sunsetter should move
 * Purpose:  Used to see if Sunsetter has taken the maximium amount of time or Nodes
 *           alloted for a move
 */

int boardStruct::timeToMove()
   {

   double timeused;
   double currentTime;

   currentTime = getSysMilliSecs(); 
   timeused = currentTime-lastMoveTime; 

   if (timeused >= millisecondsPerMove) return 1;

   return 0;

   }


/*
 * Function: getMoveNum
 * Input:    None
 * Output:   The number of moves that have been played.
 * Purpose:  Used to get the number of moves played.
 */

int boardStruct::getMoveNum()
   {
   return moveNum;
   }


/*
 * Function: getPieceInHand
 * Input:    A piece and a color
 * Output:   The number of the piece that is in the color's hand
 * Purpose:  Used to see what is in a players hand.
 */

int boardStruct::getPieceInHand(color c, piece p)
   {
   return hand[c][p];
   }


/*
 * Function: getDeepBugColor
 * Input:    None
 * Output:   A color.
 * Purpose:  Used to find the color that Sunsetter is playing currently
 */

color boardStruct::getDeepBugColor()
   {
   return playing;
   }


/*
 * Function: getOpponentColor
 * Input:    None
 * Output:   A color.
 * Purpose:  Used to find the color that Sunsetter's opponent is playing 
 *           currently
 */

color boardStruct::getOpponentColor()
   {
   return otherColor(playing);
   }


/*
 * Function: getColorOnMove
 * Input:    None
 * Output:   A color.
 * Purpose:  Used to find the color that has the move.
 */

color boardStruct::getColorOnMove()
   {
   return onMove;
   }


/*
 * Function: getColorOffMove
 * Input:    None
 * Output:   A color.
 * Purpose:  Used to find the color that doesn't have the move.
 */

color boardStruct::getColorOffMove()
   {
   return OFF_MOVE;
   }


/*
 * Function: getEnPassantSquare
 * Input:    None.
 * Output:   A square.
 * Purpose:  Used to find where en passant can be played on
 */

square boardStruct::getEnPassantSquare()
   {
   return enPassant;
   }


/*
 * Function: pieceOnSquare
 * Input:    A square
 * Output:   A piece
 * Purpose:  Used to find what's on a square
 */

piece boardStruct::pieceOnSquare(square sq)
   {
   return position[sq];
   }

bool boardStruct::isPieceOnSquare(square sq, piece p, color c) {
	return position[sq] == p && occupied[c].squareIsSet(sq);
}



#ifdef GAMETREE


/* Function: getDevelopment
 *
 */
int boardStruct::getDevelopment()

{
	return development; 
}


/* Function: getControl
 *
 */
int boardStruct::getControl()

{
	return boardControlEval(); 
}

/*Function: getMaterial
 *
 */
int boardStruct::getMaterial()

{
	return material;
}

#endif

#ifdef DEBUG_HASH
/*
 * Function: getHashValue
 * Input:    None
 * Output:   The 64 hash value.
 * Purpose:  Used to find the hash value for the current position.
 */

qword boardStruct::getHashValue()
   {
   return hashValue;
   }
#endif

#ifndef NDEBUG

/* Function: showDebugInfo
 * Input:    None
 * Output:   None.
 * Purpose:  Prints to the screen information that hopefully helps to debug
 *           Sunsetter.
 */

void boardStruct::showDebugInfo()
   {
   int n, o;
   char rightSide[8][MAX_STRING], leftSide[MAX_STRING], buf[MAX_STRING];

   sprintf(rightSide[7], "------------%X%X----------",
      (int) (hashValue >> 32), (int) (hashValue & 0xFFFFFFFF));
   sprintf(rightSide[6], "onMove: %d  playing: %d  forceMode: %d sitting: %d", 
      onMove, playing, forceMode, sitting);
   sprintf(rightSide[5], "White time:  %d  Black Time:  %d" , 
	   whiteTime, blackTime);
   sprintf(rightSide[4], "Material: %d Development: %d",
      material, development);
   sprintf(rightSide[3], "Control: %d Total Eval: %d", 
       boardControlEval(), eval());
   sprintf(rightSide[2], "Can Castle: WK: %d WQ: %d BK: %d BQ: %d", 
      canCastle[WHITE][KING_SIDE], canCastle[WHITE][QUEEN_SIDE],
      canCastle[BLACK][KING_SIDE], canCastle[BLACK][QUEEN_SIDE]);
   sprintf(rightSide[1], "White: P %d B %d R %d N %d Q %d", hand[WHITE][PAWN],
      hand[WHITE][BISHOP], hand[WHITE][ROOK], hand[WHITE][KNIGHT], 
      hand[WHITE][QUEEN]);
   sprintf(rightSide[0], "Black: P %d B %d R %d N %d Q %d", hand[BLACK][PAWN],
      hand[BLACK][BISHOP], hand[BLACK][ROOK], hand[BLACK][KNIGHT], 
      hand[BLACK][QUEEN]);
  
   for (n = 7; n >= 0; n--)
      {
      strcpy(leftSide, "");
      for (o = 0; o <= 7; o++)
         {
         if (occupied[WHITE].squareIsSet(n * ONE_RANK + o * ONE_FILE))
            sprintf(buf, " W%c", pieceToChar(position[n * ONE_RANK+o * ONE_FILE]));
         else if (occupied[BLACK].squareIsSet(n * ONE_RANK + o * ONE_FILE))
            sprintf(buf, " B%c", pieceToChar(position[n * ONE_RANK+o * ONE_FILE]));
         else
            sprintf(buf, " XX");
         strcat(leftSide, buf);
         }
      sprintf(buf, "%s %s\n", leftSide, rightSide[n]);
      output(buf);
      }
   output("\n");
   }

 

 
/*
 * Function: stopIfBad
 * Input:    None
 * Output:   Will halt the program if the board is screwed up.
 * Purpose:  Used to find out if something wrong happened and the board is
 *           screwy.  stopIfBad() checks to see that position[] lines up with
 *           the bitboards.  If it is then Sunsetter stops and should be looked
 *           at under a debugger. Note that something can still have screwed
 *           up even if stopIfBad() doesn't stop, it's just not easily
 *           detectable.
 */

void boardStruct::stopIfBad()
   {
   square sq;
   bitboard w, b;

   /* Squares being occupied by both colors is a no-no */

   assert (! (occupied[WHITE] & occupied[BLACK]).hasBits());

   /* Now make sure that position[] agrees with occupied[] and pieces[] */

   w = occupied[WHITE];
   b = occupied[BLACK];

   for (sq = 0; sq < SQUARES; sq++)
      {
      if (position[sq] != NONE)
         {
      
         assert (pieces[position[sq]].squareIsSet(sq));

      
         if (occupied[WHITE].squareIsSet(sq))
            w.unsetSquare(sq);
         else if(occupied[BLACK].squareIsSet(sq))
            b.unsetSquare(sq);
         else
            {
            assert (0); 
            }
         }
      else
         {
         assert (!(occupied[WHITE].squareIsSet(sq) || occupied[BLACK].squareIsSet(sq)));
         
         }
      }
   assert (!(w.hasBits() || b.hasBits()));

   return;
   }
  

/*
 * Function: badMove()
 * Input:    A move
 * Output:   True if there is something wrong with the move.
 * Purpose:  Used to find out if something wrong happened and a bad move
 *           was generated.  A bad move is not just illegal, but just plain
 *           wrong.  Like a piece moving in the wrong direction or trying
 *           to capture on of it's own men (or women).  A move that puts the
 *           king in check doesn't count.
 *
 */

int boardStruct::badMove(move m)
   {
   bitboard bb;

   /* Drop moves are special, handle them now */

   if (m.from() == IN_HAND) 
      {
      if (hand[onMove][m.moved()] == 0) 
         return 1;
      if (position[m.to()] != NONE) 
         return 1;
      if (m.moved() == PAWN && (rank(m.to()) == 0 || rank(m.to()) == 7)) 
         return 1;
      return 0;
      }

   /* moving a piece from a square that it isn't on is a no-no */
   if (m.moved() != position[m.from()]) 
      return 1;

   /* Capturing one of your own piece is a no-no */
   if (occupied[onMove].squareIsSet(m.to())) 
      return 1;

   /* Make sure it's a legal move.  For a regular piece this is easy, just
      see if the piece attacks the square it's moving to.  For a pawn this
      is harder because it moves different if it's capturing or not */

   if (m.moved() != PAWN) 
      {
      bb = attacksFrom(m.moved(), m.from());
      if (bb.squareIsSet(m.to())) 
         return 0;
      else if ((m.moved() == KING) && 
		  (((m.to() == G1) && canCastle[WHITE][KING_SIDE]) || 
		   ((m.to() == C1) && canCastle[WHITE][QUEEN_SIDE])|| 
		   ((m.to() == G8) && canCastle[BLACK][KING_SIDE]) ||
		   ((m.to() == C8) && canCastle[BLACK][QUEEN_SIDE])) )
		  return 0; 
         
	  else
		  return 1;
      } 
   else 
      {
      /* If it's a pawn also check to see if it got the promotion part right */

      if (rank(m.to()) == 0 || rank(m.to()) == 7) 
         {
         if (m.promotion() == NONE) 
            return 1;
         } 
      else 
         {
         if (m.promotion() != NONE) 
            return 1;
         }

      if (position[m.to()] != NONE) 
         {
         bb = pawnAttacksFrom(onMove, m.from());
         if (bb.squareIsSet(m.to())) 
            return 0;
         else 
            return 1;
         } 
      else 
         {
         if (enPassant == m.to()) return 0; 
		  
		  if (onMove == WHITE) 
            {
            if (m.to() == m.from() + ONE_RANK) 
               return 0;
            if ((rank(m.from()) == 1) && m.to() == m.from() + TWO_RANKS)
               return 0;
            return 1;
            } 
         else 
            {
            if (m.to() == m.from() - ONE_RANK)
               return 0;
            if ((rank(m.from()) == 6) && m.to() == m.from() - TWO_RANKS)
               return 0;
            return 1;
            }
         }
      }
   }
 #endif