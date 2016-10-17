/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: bughouse.h                                                         *
 *  Purpose: Has general purpose function prototypes and global variables.   *
 *                                                                           *
 *************************************************************************** */

#include <assert.h>
#include <stdio.h>
#include "definitions.h"
#include "variables.h"

#ifndef _BUGHOUSE_
#define _BUGHOUSE_

#define VERSION "9a"

enum rules { BUGHOUSE, CRAZYHOUSE };

extern rules currentRules;  /* Are we playing crazyhouse or bug? */

extern int forceMode;		/* forceMode is 1 when Sunsetter isn't playing moves of it's own */


extern int sitting;         /* If Sunsetter is sitting */

extern int toldpartisit;	/* If we decided to sit on our own */
extern int toldparttosit;
extern int parttoldgo;
extern int partsitting;      /* If it told partner to sit  */
extern int psittinglong; 	 /* for how long our part has been sitting on his own */

extern long stats_overallsearches; /* Overall number of searches in one game, for testing */
extern long stats_overallqsearches;
extern long stats_overallticks;


#ifdef DEBUG_STATS
  extern long  stats_overallDebugA; 
  extern long  stats_overallDebugB; 
#endif

#ifdef DEBUG_HASH
  extern int debug_allcoll; 
#endif
  
#ifdef GAMETREE
	extern long tree_positionsSaved; 
#endif



extern int evaluation;
extern int firstBigValue; 


extern int gameInProgress;  /* If a game is going on */

extern int soughtGame;      /* Have we already sought crazyhouse games */

void ReadIniFile(char *filename); 

void initialize();
void setDefaultValues(); 
int makeTranspositionTable(unsigned int size);

int testbpgn(int argc, char **argv);
int speedtest(int argc, char **argv);


// Those are already defined in some win32 library
#undef max
#undef min
#define max(a,b) ( a >= b ? a : b)
#define min(a,b) ( a <= b ? a : b)

#endif
