/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: board.h                                                            *
 *  Purpose: Has the data structures and functions relating to the board.    *
 *                                                                           *
 *  Comments: board.h contains the structure for the board and all the       *
 * prototypes for functions accessing it.  All of the board members          *
 * variables are private.                                                    *
 *                                                                           * 
 * pieces is an two dimentional array of bit boards with an element for      *
 * every color's piece that has 1s where one of those pieces is.             *
 *                                                                           *
 * occupied is an array with what squares white and black occupy.            *
 *                                                                           *
 * position is an array of what pieces are on each square.                   *
 *                                                                           *
 * hand is a two dimentional array of ints telling how many of each piece    *
 * each side has in their hand.                                              *
 *                                                                           *
 * A square is represented by one byte.  The format is xxFFFRRR (in binary), *
 * where Fs stand for the file, Rs stand for the rank and x's should be 0    *
 *                                                                           *
 *************************************************************************** */



#ifndef _BOARD_
#define _BOARD_


#include <time.h>      
#include <memory.h>         // for mem*() functions
#include <assert.h>

#include "definitions.h"
#include "variables.h"


// MSVC warning about inline functions getting deleted,
// since those are not needed

#ifdef _win32_

	#pragma warning( disable : 4514 ) 
	#pragma warning( disable : 4711 ) 

#endif


#define EXACT 0
#define FAIL_HIGH 1
#define FAIL_LOW 2
#define WORTHLESS 3

#define ALL_CAP 0
#define WINNING_CAP 1
#define MATE_TRIES 2
#define CHECK_EVASION_CAP 3
#define CHECK_EVASION_OTHER 4
#define HASH_MOVE 5
#define ALL_NON_CAP 6
#define MOVEGEN_TYPES 7

   
/* A couple of useful numbers */

#define SQUARES (64)
#define COLORS (2)
#define PIECES (7)         /* All the piece plus a space for NONE */

/* Before we can make the board's structure make defines and enums for colors,
 squares and pieces. */

/* Give numbers to the places pieces can be besides on the board. */

#define IN_HAND        64
#define OFF_BOARD      65

/* Define KING_SIDE and QUEEN_SIDE used in the canCastle array */

#define KING_SIDE 1
#define QUEEN_SIDE 0

#define NONE 0
#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6 

#define FIRST_PIECE 1 
#define LAST_PIECE 6
#define AFTER_PIECES 7 

#define WHITE 0
#define BLACK 1
#define FIRST_COLOR 0
#define LAST_COLOR 1
#define AFTER_COLORS 2

typedef int piece; 
typedef int square;
typedef int color;


enum gameResult { WHITE_MATE, BLACK_MATE, WHITE_RESIGNATION, BLACK_RESIGNATION,
         WHITE_FLAG_FALL, BLACK_FLAG_FALL, BOTH_FLAG_FALL };


/* move is Sunsetter's way of representing a move.  
   
   It is a 32 bit value in the following format:
   6 bits -> from square
   2 bits -> padding
   6 bits -> to square
   2 bits -> padding
   3 bits -> the piece that moved
   3 bits -> piece to promote to (if any)
   1 bit  -> if the move was en passant
   1 bit  -> if the move is no good (i.e. the value of the hashMove when
                                     there isn't a hash move)
*/

extern qword BitInBB[SQUARES]; 

struct move {

  private:
  
  duword data;

  public:
  __forceinline 
	  move(square f, square t, piece mp, piece p) { data = f | (t << 8) |
                    (mp << 16) | (p << 19); };
  __forceinline move(square f, square t, piece mp) { data = f | (t << 8) | 
					(mp << 16); };
  __forceinline move()                    {};

  __forceinline int operator ==(move m)   { return data == m.data; };
  __forceinline int operator !=(move m)   { return data != m.data; };
  __forceinline square from()             { return (square) data & 0xFF; };
  __forceinline square to()               { return (square) (data >> 8) & 0xFF; };
  __forceinline piece moved()             { return (piece) ((data >> 16) & 7); };
  __forceinline piece promotion()         { return (piece) ((data >> 19) & 7); };
  __forceinline int isBad()               { return data >> 23; };
  __forceinline void makeBad()            { data |= 1 << 23; };

};



/* A bitboard is a 64 bit piece of data, with 1 bit for every square */

struct bitboard {
  
  public:
  qword data;
   
  __forceinline bitboard(qword d) { data = d; };
  bitboard()        {};

  int popCount();                  
  // destroys the bitboard and tells how many bits were set 

  __forceinline int moreThanOne() { return (data & (data - 1)) != 0; };
  __forceinline void setBits(qword d) { data |= d; };
  __forceinline void setBits(bitboard b) { data |= b.data; };
  __forceinline void unsetBits(qword d) { data ^= d; }; 
  
  // Note that this only works to unset already set bits 
  __forceinline void unsetBits(bitboard b) { data ^= b.data; };

  /* Using Pre-generated tables for just about everything makes 
		it a bit faster , BitInBB has only the to sq corresponding 
		bit set */ 
  
  __forceinline void setSquare(square sq) { /* assert ((sq >= A1) && (sq<= H8)); */ data |= (BitInBB[sq]); };
  __forceinline void unsetSquare(square sq) { /* assert ((sq >= A1) && (sq<= H8)); */ data ^= BitInBB[sq]; }; 
  __forceinline int bitIsSet(square s) { return (data & s) != 0; };
  __forceinline int squareIsSet(square s) { /* assert ((s >= A1) && (s<= H8)); */ return (data & BitInBB[s]) != 0; };
  __forceinline int hasBits() { return data != 0; };
  __forceinline bitboard operator =(qword d) { data = d; return *this; };
  __forceinline bitboard operator =(bitboard b) { data = b.data; return *this; };
  __forceinline int operator ==(bitboard b) { return data == b.data; };
  __forceinline int operator !=(bitboard b) { return data != b.data; };  
  
  __forceinline bitboard operator |(qword b) { return bitboard(data | b); };
  __forceinline bitboard operator |(bitboard b) { return bitboard(data | b.data); };
  __forceinline bitboard operator |=(bitboard b) { data |= b.data; return *this; };
  
  __forceinline bitboard operator &(qword b) { return bitboard(data & b); };
  __forceinline bitboard operator &(bitboard b) { return bitboard(data & b.data); };
  __forceinline bitboard operator &=(bitboard b) { data &= b.data; return *this; };
  __forceinline bitboard operator <<(int i) {  return bitboard(data << i); };
  __forceinline bitboard operator >>(int i) {  return bitboard(data >> i); };  
  __forceinline duword operator >(int i) { return (i < 32 ? ((duword) data) >> i :
                ((duword) (data >> 32)) >> (i - 32)); };

  /* The '>' operator is split shift right, an idea borrowed from crafty.  It
     It only gives the bottom 32 bits of what normally would be the result, 
     but on a 32 bit machine it doesn't have to bother with all the overhead of
     doing 64 bit shifts and is much quicker to user then the normal shift
     right.  It should be used when only the bottom 32 bits are needed */
  
  __forceinline bitboard operator ~(void) { return bitboard(~data); };
};

/* Function: firstSquare
 * Input:    bitboard.data , a qword
 * Output:   A square
 * Purpose:  Used to get the lowest square that is set in a bitboard.
 */

#ifdef _win32_
#pragma warning( disable : 4035 ) // no return value warning 


__forceinline int firstSquare (qword a) 
   
{

	// bitboard must NOT be empty !

	__asm { 
		bsf eax, dword ptr a+4
		add eax, 32
		bsf eax, dword ptr a 
	}
}


#pragma warning( default : 4035 )

// above works for MSVC windows compiler,  
// the the one below 
// works for MinGW compiler and for gcc.
// Thanks to Angrim

#elif defined(__i386__)

inline
int firstSquare(qword a)
{
	// code for 32bit Intel type cpus.
	register int res;
	union {
		unsigned long long x;
		unsigned long y[2];
	} z;
	z.x = a;
	assert(a != 0); // bsf doesn't work on 0, which has no bits set.
	__asm__(
		"bsf    %1,%0\n\t"
		"addl   $32, %0\n\t"
		"bsf    %2,%0\n\t"
		: "=&r" (res)
		: "g" (z.y[1]), "g" (z.y[0])
		: "cc"
	);
	return res;
}
#elif defined(__x86_64__)
 // code for 64bit Athlon type cpus
inline
int firstSquare(qword a)
{
	register qword res;
	assert(a != 0); // bsf doesn't work on 0, which has no bits set.
	__asm__(
		"bsfq %1, %0\n\t"
		: "=r" (res)
		: "g" (a)
		: "cc");
	return (int)res;
}
#elif defined(__GNUC__)
inline
int firstSquare(qword a)
{
    assert(a != 0);
    return __builtin_ctzll(a);
}
#else
#error not a supported architecture
 // If someone wants to support non-intel architectures, there are some (slower) C versions
 // at  https://chessprogramming.wikispaces.com/BitScan
 // one of which could be used here.
#endif


/* *Attacks is where a piece attacks from a square */

extern bitboard pawnAttacks[COLORS][SQUARES];
extern bitboard contactBishopAttacks[SQUARES]; 
extern bitboard contactRookAttacks[SQUARES];
extern bitboard knightAttacks[SQUARES];
extern bitboard kingAttacks[SQUARES];

/* squaresTo is the squares from the first square to the second.  Including
   the second square, but not including the first */

extern bitboard squaresTo[SQUARES][SQUARES];

/* squaresPast is the squares are after the second square, one the same
   line as the first */

extern bitboard squaresPast[SQUARES][SQUARES];

/* nearSquares are squares 2 or less squares away */

extern bitboard nearSquares[SQUARES][COLORS];

/* directionPiece is a bishop, rook or none, depending on what line moving
   piece can move from the first square to the second */

extern piece directionPiece[SQUARES][SQUARES];

/* takeBackInfo has stuff to take back a move */

struct takeBackInfo {
  sword attacks[COLORS][64];
  piece captured;
  byte oldCastle[COLORS][2];
  square oldep; 
  qword oldHash;

#ifdef DEBUG_HASH
  qword oldHashT; 
#endif

  byte capturedPromotedPawn; /* Only used in crazyhouse */
};

							/* transpositionEntry is what the transposition 
							tables are built on */

#ifdef _win32_
#pragma pack(4)				/* I don't know why I need to do this to pack 
							the table entry in 16 byte */
#endif

struct transpositionEntry {
  byte type : 4;			/* fail low, high, or exact */
  byte moveNr : 4;			/* to check whether an entry can be overwritten */
  byte depth;				/* How deep the position was searched */
  
  sword value;              /* The value of the position */
  qword hash : 48;          /* The upper 48 bits of the hash value of the  position. */
#ifdef DEBUG_HASH   
  qword hashT : 48;			/* To check for hash collisions */
#endif

  move hashMove;            /* The best move last time */
  
};

#ifdef _win32_
#pragma pack(8)
#endif


							// Utilities for the board

void dumpBoardInfo(); 
         

void initBitboards();       // Must be called durring initialization 
void initializeEval();  

void initializeHistory();	// Used to create the "good" moves in aimoves first. 
void updateHistory(move m, int depth);
void makeHistoryOld(); 


color otherColor(color c);
int isInHand(square sq);
int rank(square sq);
int file(square sq);

void zapHashValues();


								/* The board structure. */

struct boardStruct {
  private:

  piece position[64];            /* What piece is on each square */
	  
  bitboard occupied[COLORS];     /* What squares are occupied by each color */

  bitboard moveAttackedSomething[MAX_GAME_LENGTH];
								 /* Which squares the last move directly attacked */

  bitboard occupiedMirror;       /* What squares are occupied, but with the
									file and rank bits switched */
  bitboard occupiedUR;           /* What squares are occupied, organized by
									diagonals going up and to the left */
  bitboard occupiedUL;           /* What squares are occupied, organized by
                                    diagonals going up and to the left */
  bitboard pieces[PIECES];       /* What squares are occupied by each piece */
  
  square kingSquare[COLORS];     /* Where each king is */
  
  sword attacks[COLORS][64];       /* How well each color attacks a square */
  int hand[COLORS][PIECES];      /* What is in the player's hands */
  int whiteTime;                 /* Time in 1/100th seconds on white's clock */
  int blackTime;                 /* Time in 1/100th seconds on black's clock */
  int timeTaken;                 /* Time in 1/100th seconds of how long the
									player thought on the last move */

  
  
  
  double lastMoveTime;		     /* When the last move was played */ 
  color playing;                 /* The color the computer in playing */
  color onMove;                  /* The color on the move */
  byte canCastle[COLORS][2];     /* If the players can castle on either side */
  square enPassant;              /* The square, if any, a pawn passed over */
  bitboard promotedPawns;        /* Where promotedPawns are */
  move moveHistory[MAX_GAME_LENGTH]; 
								 /* What moves were played. */
  takeBackInfo takeBackHistory[MAX_GAME_LENGTH]; 
								 /* Information to take back
									the moves played (hopefully no more than MAX_GAME_LENGTH)*/
  int checkHistory[MAX_GAME_LENGTH];
								 /* whether we were in check on that move */
  
  int bestCaptureGain[MAX_GAME_LENGTH];
								 /* the best possible capture at that move,
								    because of this we need to make sure 
									that orderCaptures is used on _every_
									node */

  int moveNum;                   /* What half-move the position is on. */
  
  qword hashValue;               /* The hash value for the position, used for
									the transposition table. */
#ifdef DEBUG_HASH
  qword hashValueT; 
#endif

  /* changeBoard changes depending on if we're playing bug or crazyhouse.
     To handle this it calls the function pointed to by currentChangeBoard.
     Ditto for unchangeBoard */

  void (boardStruct::*currentChangeBoard)(move m);
  void (boardStruct::*currentUnchangeBoard)();

  void bugChangeBoard(move m);
  void bugUnchangeBoard();
  void ZHChangeBoard(move m);
  void ZHUnchangeBoard();

  /* Members used by eval() */  
  
  int material;                 /* The relative material with positive 
									 meaning white is up in material */
  int development;              /* The relative delelopment */

  public:                       /* Primitives to access the variables */

  /* Some primitives to change the board */

  bool custom; // niklasf

  void addPieceToHand(color c, piece p, int hash);
  void removePieceFromHand(color c, piece p, int hash);
  
  void setPieceInHand(color c, piece p, int num);
  void setTime(color side, int time);
  void setDeepBugColor(color c);
  void setColorOnMove(color c);
  
  void setCheckHistory(int check);
  void setBestCapture();

  
  void copy(boardStruct *dest);        
								/* Copies itself to another board,
									the other board must already
									have been init()ed */

  void resetBoard();            /* Sets the pieces to the original
									position and does the other
									things needed to start a 
									game.*/

  void setBoard(const char *pieces, const char *turn, const char *castles, const char *ep);
								/* Sets the board from FEN , niklasf */
  
  void resetBitboards();        /* resets the bitboards for a new
									game */

  void updateDiagonalBitboards(); /* Creates the diagonal bitboards when the others are already set, niklasf */

  int playMove(move m, int report);
								/* Plays a move. */

  int unplayMove();             /* Takes back the last move. */

  void changeBoard(move m);		/* changeBoard() is a low level
									version of playMove(). */
  void unchangeBoard();			/* Takes back the last move made
									by changeBoard() */

  void makeNullMove();			/* Make and unmake NullMoves, see search() */
  void unmakeNullMove(); 

  void playBughouse();			/* Tells Sunsetter to play bughouse */
  void playCrazyhouse();		/* Tells Sunsetter to play crazyhouse*/

  void setLastMoveNow();		/* Used that when resaerching it doesn't think 
									it has used up all time */

  
  /* The primitives that get information from the board */

  /* Book code by Angrim
  char * getLineText();
  */ 


  int getTime(color c);         /* Be carefull reading the time, it's 
									only updated when winboard tells 
									Sunsetter the time, so it's rarely right */
  int outOfTime(color c);
  int timeToMove();             /* TRUE if Sunsetter should stop thinking
									and move now */

  int getMoveNum();
  int getPieceInHand(color c, piece p);
  
  color getDeepBugColor();
  color getOpponentColor();
  color getColorOnMove();
  color getColorOffMove();
  square getEnPassantSquare();
  piece pieceOnSquare(square sq);
  bool isPieceOnSquare(square sq, piece p, color c);   //niklasf for setboard


#ifdef DEBUG_HASH
  qword getHashValue();
#endif

  move rawAlgebraicMoveToDBMove(const char *notation);
  move algebraicMoveToDBMove(const char *notation);

  int moves(move *m);                  /* moves() fills an array of Moves
									 with all of the moves in the 
									 position */

  int isLegal(move m);                /* returns TRUE if the move is legal,
									 FALSE if not */
  
  int isInCheck(color side);         /* returns 1 if side is in check,
									 2 if side is in double check */

  int isCheckmate();                  /* returns TRUE if the side to move is 
									checkmate */

  int cantBlock();                    /* returns TRUE if the side to move
									can't block a check */

  int aiMoves(move *m);               /* AIMoves() fills an array of 
									with moves in the position */
  
  int captureMoves(move *m);          /* captureMoves fills an array of 
									moves with the captures in a 
									position. */
  int captureGain(color c, move m);			/* How much material a move gains */
  
  move *orderCaptures(move *m);				/* Orders captures based on material gain*/

  void captureMovesTo(move *m, square sq);	/* Fills an array of moves with
												captures to a certain square */
  int mateTries(move *m);					/* fills an array of moves with 
												contact checks which can't be taken*/

  int checkEvasionCaptures(move *m);		/* fills an array of with move that get
												out of check */  

  int checkEvasionOthers(move *m);

  int eval();                  /* eval() evaluates the position and
								 returns the value */

  int adjustInHand();

  int bughouseSitForEval();		/* Malus in Bughouse if we'd have to sit for a piece */
  int bughouseMateEval();		/* see above, for mates */ 

  int highestAttacked(square moveTo);	/* Razor full search conditions */
  int escapingAttack(square movedFrom, square moveTo);

  int captureExtensionCondition();
  int reSearchCondition(move bestMove);

  int standpatCondition();				
  int checksInRow();
 
  #ifdef GAMETREE  // needed when outputing a game tree in html 
  
  piece showPiece (square sq); 
  int showColor (square sq);
  
  int getDevelopment();   	
  int getControl();
  int getMaterial(); 

  int getPieceDevOnSquare(color c, square sq, piece p); 
  int oneSquareBoardControl(square sq, int report); 
  
  
  move getMoveHistory(int ply); 

  #endif

									  
  void getEval(char *buf);           /* outputs The current evaluation
										split into components 
										in the prinicpal variation */

 #ifndef NDEBUG

  int badMove(move m);				 /* True if a move is bad (not just illegal)*/
 #endif
  /* Other primitives */
 #ifndef NDEBUG
  void showDebugInfo();               /* Prints debuging information to the
									  screen. */

 #endif


  void store(int depthSearched, move bestMove,	/* Stores a position in the */
        int value, int alpha, int beta, int ply);		/* transposition tables */

  transpositionEntry *lookup();                 /* Attempts to look up the
												position from the 
												transposition tables */

  int checkLearnTable(); 
  void saveLearnTable(int pointsWon); 
  
  private:

  /* Primitives called internally */

  /* These update the hash value of the board */
  
  void initHash();
  void addToHash(color c, piece p, square sq);

  void addToInHandHash(color c, piece p);
  void subtractFromInHandHash(color c, piece p);
  void updateCastleHash(color c, int side);
  void addEnPassantHash(square sq);


  /* swap() tells how much material a exchange will win/lose */

  int swap(square from, square to);

  /* functions to move pieces */
  
  void movePiece(color c, piece p, square to, square from, int hash, int attack);
  
  bitboard replacePiece(color oldc, piece oldp, color newc,
          piece newp, square sq, int hash, int attack);
  bitboard removePiece(color c, piece p, square sq, int hash, int attack);
  bitboard addPiece(color c, piece p, square sq, int hash, int attack);

  void setCastleOptions(color c, int side, int can, int hash);
  void addToBitboards(color c, piece p, square sq);
  void removeFromBitboards(color c, piece p, square sq);

  /* Functions to help with the bitboards and attack information */  


  bitboard pawnAttacksFrom(color c, square sq);
  bitboard attacksFrom(piece p, square sq);
  bitboard contactAttacksFrom(piece p, square sq); 
  bitboard attacksTo(square sq);
  bitboard blockedAttacks(color c, square where);
  int isAttacked(color c, square sq);
  bitboard fileAttacks(square sq);
  bitboard rankAttacks(square sq);
  bitboard diagULAttacks(square sq);
  bitboard diagURAttacks(square sq);
  
  bitboard addAttacks(color c, piece p, square sq);
  bitboard removeAttacks(color c, piece p, square sq);
  bitboard blockAttacks(color c, square sq);
  bitboard uncoverAttacks(color c, square sq);

  /* These help out eval() */

  int kingSafetyEval(color c); 
  int boardControlEval();
  int getMaterialInHand(color c); /* gets the values of material in hand */

  /* These help out making moves */

  void whitePawnMovesTo(move **m, bitboard possibleTo, piece promote);
  void blackPawnMovesTo(move **m, bitboard possibleTo, piece promote);
  void whitePawnCapturesTo(move **m, bitboard possibleTo);
  void blackPawnCapturesTo(move **m, bitboard possibleTo);
  int  skippedMoves(move *m);

  /* Functions to verify that nothing screwy happened */
#ifndef NDEBUG
  void stopIfBad();               /* Halts if the board is messed up */
#endif
};

/* Externs */

extern boardStruct gameBoard;   /* The main board */
extern const int adOffset[8];	/* What to add to a square to make a piece
									move in an attackDirection */
extern const piece adPiece[8];  /* What piece (other than the queen) moves
									in an attackDirection. */
extern const int knightDirection[8]; 
								/* The possible knight directions */

/* Some constants for the board */

#define ONE_FILE    (8)			/* Number to add to get the next file of a square */
#define TWO_FILES   (16)
#define THREE_FILES (24)
#define FOUR_FILES  (32)
#define FIVE_FILES  (40)
#define SIX_FILES   (48)
#define SEVEN_FILES (56)

#define ONE_RANK    (1)
#define TWO_RANKS   (2)
#define THREE_RANKS (3)
#define FOUR_RANKS  (4)
#define FIVE_RANKS  (5)
#define SIX_RANKS   (6)
#define SEVEN_RANKS (7)

#define A1 0
#define A2 1
#define A3 2
#define A4 3
#define A5 4
#define A6 5
#define A7 6
#define A8 7
#define B1 8
#define B2 9
#define B3 10
#define B4 11
#define B5 12
#define B6 13
#define B7 14
#define B8 15
#define C1 16
#define C2 17
#define C3 18
#define C4 19
#define C5 20
#define C6 21
#define C7 22
#define C8 23
#define D1 24
#define D2 25
#define D3 26
#define D4 27
#define D5 28
#define D6 29
#define D7 30
#define D8 31
#define E1 32
#define E2 33
#define E3 34
#define E4 35
#define E5 36
#define E6 37
#define E7 38
#define E8 39
#define F1 40
#define F2 41
#define F3 42
#define F4 43
#define F5 44
#define F6 45
#define F7 46
#define F8 47
#define G1 48
#define G2 49
#define G3 50
#define G4 51
#define G5 52
#define G6 53
#define G7 54
#define G8 55
#define H1 56
#define H2 57
#define H3 58
#define H4 59
#define H5 60
#define H6 61
#define H7 62
#define H8 63


/* Some usefull bitboard constants */

#ifdef _win32_

	#define A_FILE      (qword(0x00000000000000FF))
	#define H_FILE      (qword(0xFF00000000000000))
	#define EIGHTH_RANK (qword(0x8080808080808080))
	#define FIFTH_RANK  (qword(0x1010101010101010))
	#define FOURTH_RANK (qword(0x0808080808080808))
	#define FIRST_RANK  (qword(0x0101010101010101))

#endif

#ifndef _win32_

	#define A_FILE      (0x00000000000000FFULL)
	#define H_FILE      (0xFF00000000000000ULL)
	#define EIGHTH_RANK (0x8080808080808080ULL)
	#define FIFTH_RANK  (0x1010101010101010ULL)
	#define FOURTH_RANK (0x0808080808080808ULL)
	#define FIRST_RANK  (0x0101010101010101ULL)

#endif



#endif

