/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: bughouse.cc                                                        *
 *  Purpose: Has the main game loop, the initialization and other general    *
 *           functions.                                                      *
 *                                                                           *
 *  Comments: The basic game loop is this:                                   *
 *                                                                           *
 *           --------          --------                                      *
 *    ---> |   AI   | -----> | main() | -----------|                         *
 *    |     --------          --------             |                         *
 *    |       /|\  |             |                 |                         *
 *    |        |   |-------------|-------------|   |                         *
 *   \|/       |                \|/           \|/ \|/                        *
 *  --------   |              --------         --------                      *
 * |search()|   ------------ | Board  | <---- | Xboard |                     *
 *  --------                  --------         --------                      *
 *    /|\     --------        --------                                       *
 *     |---> |quiesce()| <--> | eval() |                                     *
 *            --------        --------                                       *
 *                                                                           *
 * Sorry for the pitiful attempt at ASCII art.  The basic idea is that there *
 * a main function that handles initialization and things like that.  The    *
 * main function tells the interface that it's ready and the interface gets  *
 * the player's move.  That changes the board.  Then the AI part looks at    *
 * the board and finds the (hopefully) best move.  The AI uses a recursive   *
 * search function that looks at moves and evaluates them using a quiescense *
 * search then an evaluation function.  More explanation of that is in       *
 * search.cc. Once the AI gets its move it gives it to main and main         *
 * gives the move to the interface and updates the board.  The interface     *
 * then gets the players move and the cycle repeats.  The only other thing   *
 * is that while the AI is looking for the right move it will periodically   *
 * see if there is input waiting (A passed piece, checkmate on the other     *
 * board, etc).                                                              *
 *                                                                           *
 *************************************************************************** */



#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "variables.h"
#include "definitions.h"
#include "board.h"
#include "interface.h"
#include "bughouse.h"
#include "brain.h"


#ifdef _win32_
void sleep (int);
// 7g: above commented out, instead:
// #include <windows.h>

#endif

#ifndef _win32_
#include <unistd.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Book code by Angrim
/* bookMove() should have its own header file, minor todo */
// @ All: Does Book code have to be part of Board Structure? Would maybe prefer it in seperate book.cpp file and structure (gz)
// extern int bookMove(move *rightMove, boardStruct &where);


int sitting;
int toldpartisit;
int toldparttosit;
int parttoldgo; 
int partsitting;
int psittinglong; 


char	personalityIni[10][512]; 

int		PERSONALITY = 0; 
int		FIXED_DEPTH; 
int		CAPTURE_EXTENSION,CHECK_EXTENSION,FORCING_EXTENSION ; 
int		NK_FACTOR, BC_FACTOR, DE_FACTOR; 
int		CC_DEPTH, NULL_REDUCTION; 

int		pValue[PIECES]  = {0, 0, 0, 0, 0, 0, 0 };  

long stats_overallsearches, stats_overallqsearches, stats_overallticks;


#ifdef DEBUG_STATS
long  stats_overallDebugA, stats_overallDebugB; 
#endif

#ifdef DEBUG_HASH
int debug_allcoll;
#endif

#ifdef GAMETREE
long tree_positionsSaved; 
#endif

int evaluation;
int firstBigValue; 

int gameInProgress;

int soughtGame;

rules currentRules;

/* Function:	setDefaultValues
 * Input:		none
 * Output:		none
 * Purpose:		Sets the default values, like knight value et cetera 
 *
 */

void setDefaultValues()
{ 

CC_DEPTH = 3; 

FIXED_DEPTH=			0;
CHECK_EXTENSION=		(ONE_PLY * 3) / 4 ; 
CAPTURE_EXTENSION=		ONE_PLY / 2;
FORCING_EXTENSION=		ONE_PLY / 2;


NULL_REDUCTION=  2; /* How much to reduce search on NullMove 
					  Increase this to make the search in the nodes 
					  that produce "NullCuts" much faster, but miss
					  some tactical shots. Normal values are 1-3 */


NK_FACTOR = 5;   /* Factor for the more precise King Safety eval which 
					takes material into account. */			   
			

BC_FACTOR = 5;   /* Factor for Control of Squares near the king. 
				 	Color gets a bonus for squares near own and enemy 
					king which are controlled (attacked more than 
					defended)  by it. This depends 
					on material in hand as well, and is computed on 
					every eval() call. */

DE_FACTOR = 3;   /* Factor for Developement.
					This is a normal,  static table. 
					Example: Queens and King in Center malus, 
					Pawn on g2 bonus ... */



/* There are the values Sunsetter uses to measure pieces, the order is:
   (No piece), Pawn, Rook, Knight, Bishop, Queen, King (0).  Note that a pawn
   is 100, all evaluations use 100 points to equal a pawn */

pValue[PAWN] = 100; pValue[ROOK] = 200; pValue[KNIGHT] = 192; pValue[BISHOP] = 195; pValue[QUEEN] = 390; 



}

/* Function: findFile
 * Input:    A name of a file, how top open it
 * Output:   A pointer to the file.
 * Purpose:  Searches the for a file in the DB_DIRECTORY setting, the current
 *           directory and the home directory
 */

#ifndef __EMSCRIPTEN__
FILE *findFile(const char *name, const char *mode)
{
  char str[MAX_STRING], *ptr;
  FILE *f;

  ptr = getenv("DB_DIRECTORY");
  if(ptr) {
    sprintf(str, "%s/%s", ptr, name);
    f = fopen(str, mode);
  } else f = NULL;
  if(!f) {
    sprintf(str, "./%s", name);
    f = fopen(str, mode);
  }
  if(!f) {
    ptr = getenv("HOME");
    if(ptr) {
      sprintf(str, "%s/%s", ptr, name);
      f = fopen(str, mode);
    }
  }
  return f;
}
#endif  // #ifndef __EMSCRIPTEN__

/* Function: findZHGame
 * Input:    None
 * Output:   None
 * Purpose:  Used when we're playing crazyhouse.  It seeks for a game and waits
 *           for it to start
 */

// 7g: static void
void findZHGame()
{
  if(!soughtGame) {
    soughtGame = 1;

	output("tellics gameend1\n");
	output("tellics gameend2\n");
    output("tellics gameend3\n");
    output("tellics gameend4\n");

  }
}

/* Function: main
 * Input:    The command line aruments.
 * Output:   The program's exit code.
 * Purpose:  It's the head honcho of the Sunsetter.  It does the initialization
 *           then it runs the game loop.
 */

#ifdef __EMSCRIPTEN__
// code by niklasf,therefore I put it inside __EMSCRIPTEN__  @niklasf: what is this doing? 
void mainLoop(void *);
#endif  // #ifdef __EMSCRIPTEN__

int main(int argc, char **argv)
{
  move m;
  int n, o;
  char buf[MAX_STRING];

  if(argc > 1 && !strcmp(argv[1], "test")) 
  
  {
    /* If the first argument that Sunsetter is given is 
	  "test" then go through a bpgn file  */
    initialize();
	testbpgn(argc, argv);	
	return 0; 
  }

  if(argc > 1 && !strcmp(argv[1], "speedtest")) 
  
  {    
	/* If the first argument that Sunsetter is 
	  given is "speedtest" then use the bpgn file to test 
	  make/unmake eval and movegen speed on all positions
	  in that bpgn game */
    initialize();
	speedtest(argc, argv);	
	return 0; 
  }


  initialize();

  /* parse the options */

  for(n = 1; n < argc; n++) {
    if(argv[n][0] == '-') {
      strcpy(buf, argv[n] + 1);
      for(o = n + 1; o < argc && argv[o][0] != '-'; o++) {
        strcat(buf, " ");
        strcat(buf, argv[o]);
      }
      parseOption(buf);
    }
  }

  for (;;) {

    /* If we don't have a partner, lets try to get one, but only if not playing crazyhouse */

	  if(xboardMode && !gameInProgress && !partner && !analyzeMode)  { findZHGame();} 


    if (gameInProgress && !forceMode) 
	{
		
		if ((gameBoard.getColorOnMove() == gameBoard.getDeepBugColor()))
		{


#ifdef __EMSCRIPTEN__
			if (gameBoard.custom || gameBoard.getMoveNum()>9 || !bookMove(&m, gameBoard)) { // What is this doing?  @niklasf
#endif
			findMove(&m);
#ifdef __EMSCRIPTEN__
		}
#endif
		if(!m.isBad() && gameInProgress && !forceMode && !analyzeMode)
		gameBoard.playMove(m, 1);

		} 
	
		// not pondering in Bug at all  
		//
		
		else if (currentRules == BUGHOUSE) 
		{

			millisecondsPerMove = 10000;
			if (gameBoard.timeToMove() && (!toldparttosit) && (!partsitting)) {toldparttosit =1; output ("tellics ptell sit (my opponent is waiting for something)\n"); }
			millisecondsPerMove = 100000000;
		}		
		
		else if (gameBoard.getMoveNum() != 1 && tryToPonder && !analyzeMode) 
		{

		ponder();
		} 
	

    }
#ifdef __EMSCRIPTEN__
	emscripten_sleep_with_yield(30);
#else
	// sleep for 1/100th of a second, avoid hogging the cpu
#ifdef _win32_
	sleep(10);
#else
	usleep(10000);
#endif
#endif  // #ifdef __EMSCRIPTEN__

checkInput();
  }

}

/* Function: ReadIniFile
 * Input:    char * , the file name 
 * Output:   None.
 * Purpose:  Reads an initialization file, same commands as  
 *			 command line parameters are supported. 
 */


 void ReadIniFile(char *filename) 
  {
#ifndef __EMSCRIPTEN__
   char parambuf[MAX_STRING]; 
   FILE *inif;	

  inif = findFile(filename, "rt");
  if(inif) {	  
	  while(!feof(inif)) {
      strcpy(parambuf, "");
      fgets(parambuf, MAX_STRING, inif);
      parseOption(parambuf);
    }
  }
#endif  // __EMSCRIPTEN__
 }
/*  7g Version by angrim
void ReadIniFile(char *filename)
{
char parambuf[MAX_STRING];
char parambuf[MAX_STRING]="";
FILE *inif;

inif = findFile(filename, "rt");
if(inif) {
while(!feof(inif)) {
strcpy(parambuf, "");
fgets(parambuf, MAX_STRING, inif);
if(!fgets(parambuf, MAX_STRING, inif)) break;
parseOption(parambuf);
}
fclose(inif); inif=NULL;
}
}
*/

/* Function: initialize
 * Input:    None
 * Output:   None.
 * Purpose:  initialize() sets up everything need for the program to start.
 */

void initialize()
{

	char fileName[MAX_STRING];

#ifdef LOG
	sprintf (fileName,"ss-%s.txt",VERSION); 
	logFile = fopen(fileName, "wt");
#endif

  output(clientLogin);
  signal(SIGINT, SIG_IGN);
  srand(time(NULL) + 2);

  setDefaultValues(); 
  initBitboards();
  initializeEval(); 
  

  forceMode = 1;

  gameInProgress = 0;
  partner = 0;
  tryToPonder = 1;
  gameBoard.resetBoard();
  
  gameBoard.playCrazyhouse();
  currentRules=CRAZYHOUSE; 



  if(makeTranspositionTable(MIN_HASH_SIZE)) 
  {
	fprintf(stderr, "Not enough memory!\n");
	exit(1);
  }
  
    /* Try to find the initialization file. If it's found then send all of the
     strings to parseOption() */
    
  sprintf (fileName,"ss-%s.ini",VERSION); 
  ReadIniFile(fileName);

}