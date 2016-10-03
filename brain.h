/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: brain.h                                                            *
 *  Purpose: Has function prototypes for the files search.c, move.c,         *
 * quiescense.c and evaluation.c.                                            *
 *                                                                           *
 *  Comments: This is the header files that has everything pertaining to     *
 * Sunsetter's artificial intellegence.                                      *
 *                                                                           *
 * Sunsetter finds the move by using an alpha-beta  search, then a quiesense *
 * search and finally its evaluation function.  See the files search.c,      *
 * quiescence.c and evaluation.c for details.                                *
 *                                                                           *
 *************************************************************************** */

#ifndef _BRAIN_
#define _BRAIN_

#include "board.h"
#include "interface.h"

/* Various search value definitions.  These kind of random, but also kind of
   interlinked so don't change 'em */

#define INFINITY    32000
#define MATE        20000	/* If a move is mate it will be at least this */
#define MATE_IN_ONE 30000	/* The value of mate in one, mate in 2 will be 
							less this make Sunsetter pick the fastest mate */
			     
#define ALMOST_MATE 18000	/* Almost mate means that there aren't any legal
							moves, but that the person can sit for a piece
							then there will be legal moves */

#define EXTREME_EVAL 10000
							/* Almost as bad/good as a mate, but shouldn't result in mate announcement */

#define MAX_SEARCH_DEPTH 48
/* The most depth db will search in normal search.cc */

#define DEPTH_LIMIT 64
/* The most depth that Sunsetter will ever get to */

#define MAX_QUIESCE_SEARCH_DEPTH 64
/* The most depth that Sunsetter will search in the quiescesce search */


/* Stuff for search extensions.  These can be in fractions of one ply.  */
#define ONE_PLY               4

/* Define PrincipalVariation which is a struct to hold the best moves by
   both sides that Sunsetter searched */

struct PrincipalVariation {
  
  int depth[DEPTH_LIMIT];
  move moves[DEPTH_LIMIT][DEPTH_LIMIT]; 

};


/* Externs.  See the file their defined in for more info. */

extern char		personalityIni[10][512]; 
extern int		PERSONALITY; 

extern int		FIXED_DEPTH; 

extern int		CHECK_EXTENSION;
extern int		CAPTURE_EXTENSION;
extern int		FORCING_EXTENSION;

extern int		CC_DEPTH; 


extern int      NULL_REDUCTION; 

extern int		NK_FACTOR;  
extern int		BC_FACTOR;  
extern int		DE_FACTOR;  

extern int		paramA;
extern int		paramB;


extern int pValue[PIECES]; 

extern int DevelopmentTable[COLORS][PIECES][64];   /* How good it is to have a
                                                   piece on a square */

extern bitboard generateToFirst[PIECES][COLORS][2]; 

extern int hashMoveCircle;



extern int ratingDiff;							/* The rating difference */
extern double millisecondsPerMove;                 /* How long to think */
extern int pondering;                           /* If we're pondering */
extern int tryToPonder;                         /* If we should try to
                                                   ponder */

extern int learning;							/* If we should use the learn file */
extern int analyzeMode;							/* If we should not make a move 
												   ever and accept all move input */
extern int xboardMode;							/* should we send "tellics" stuff */

/* AIBoard is the board Sunsetter uses to think */

extern boardStruct AIBoard;

/* Now some function prototypes */


void findMove(move *rightMove);               /* Called by the main loop to
                                                 find the right move. */

void ponder(void);                            /* Called by the main loop
                                                 when it's Sunsetter's
                                                 opponent's move */

int search(int alpha, int beta,
           int depth, int ply, int wasNullMove); /* Uses a recursive alpha-beta
                                                 search to assign a value to
                                                 the position. */



int quiesce(int alpha, int beta, int ply);    
											  /* evaluates the position with a
												 quiescense search*/

void setpValue(color c, piece p, sword value); 
											  /* Changes the value
                                                 of a piece */
void sit();                                   /* Commands to make Sunsetter sit*/
void unsit();                                 /* or stop sitting */
void startSearchOver();                       /* Makes Sunsetter restart
                                                 searching */
void analyzeUpdate();						  /* In analyze mode we provide
											     updates at least every 4
												 seconds. */
void stopThought();                           /* Called to make Sunsetter
                                                 stop thinking about the move
                                                 and play whatever it has. */
void forceDeepBugToMove();                    /* Makes Sunsetter play a move
                                                 even if it gets it mated */
void doMove(move m);                          /* Makes Sunsetter play a move
                                                 regardless of what it finds */
												 
void resetAI(void);                           /* Stop all thinking about
                                                 moves */



void saveLearnTableToDisk();				  /* Guess what this does :) */
void readLearnTableFromDisk(); 





#endif