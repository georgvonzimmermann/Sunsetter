/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *																			 *
 *  Credit for the Linux input code goes to Angrim (thanks !). I wish        *
 *  I understood how it works.												 *
 *																			 *																			 *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: interface.cc                                                       *
 *  Purpose: Has the functions relating to the user interface.               *
 *                                                                           *
 *  Comments: interface.h contatains functions for communication.  See       *
 *   DESIGN for a description of what happens.                               *
 *                                                                           *
 *************************************************************************** */


#ifdef _win32_

	#include <windows.h>
	#include <io.h>
	#define _write write
	#define _read  read
	#define _close close

#endif

#ifndef _win32_ // unix/posix headers
	
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <sys/select.h> // from 7g, possibly needed

#endif


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "interface.h"
#include "variables.h"
#include "notation.h"
#include "bughouse.h"
#include "brain.h"
#include "board.h"

/* This is what is echoed when the user first logs on to Sunsetter in client mode.  */

#ifndef __EMSCRIPTEN__

char *clientLogin = 
"\n\n"
"            ^^                   @@@@@@@@@\n"
"       ^^       ^^            @@@@@@@@@@@@@@@\n"
"                            @@@@@@@@@@@@@@@@@@              ^^\n"
"                           @@@@@@@@@@@@@@@@@@@@\n"
"                           @@@@@@@@@@@@@@@@@@@@\n"
" ~         ~~   ~  ~       ~~~~~~~~~~~~~~~~~~~~ ~       ~~     ~~ ~\n"
"   ~      ~~      ~~ ~~ ~~  ~~~~~~~~~~~~~ ~~~~  ~     ~~~    ~ ~~~  ~ ~~\n"
"   ~  ~~     ~         ~      ~~~~~~  ~~ ~~~       ~~ ~ ~~  ~~ ~\n"
"\n"
" Sunsetter " VERSION " \n"
" (c) Ben Dean-Kawamura, Georg v. Zimmermann\n"
" See http://sunsetter.sourceforge.net/ for more info.\n\n";
#else
const char *clientLogin = "Sunsetter " VERSION " (c) Ben Dean-Kawamura, Georg v. Zimmermann\n";
#endif  // #ifndef __EMSCRIPTEN__

int ratingDiff = 0; 
int learning = 0;
int analyzeMode = 0; 
int forceMode = 0;

int xboardMode = 0; 

#ifdef LOG

FILE *logFile;

#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <queue>

static std::queue<char *> command_queue;

extern "C" void queue_command(const char *c_cmd) {
	command_queue.push(strdup(c_cmd));
}

void waitForInput() {
	while (!checkInput()) {
		emscripten_sleep_with_yield(30);
	}
}

int checkInput() {
	int wasInput = 0;

	if (gameBoard.timeToMove()) stopThought();

	if (analyzeMode && gameInProgress && !xboardMode) analyzeUpdate();

	while (!command_queue.empty()) {
		char *c_cmd = command_queue.front();
		command_queue.pop();
		parseOption(c_cmd);
		free(c_cmd);

		wasInput = 1;
	}

	return wasInput;
}

void output(const char *str) {
	printf("%s", str);
}
#else

#ifdef _win32_

void sleep(int n)
   {
   Sleep(n);
   }

/*
 *  THREADED NON-BLOCKING CONSOLE INPUT    -- WIN 32 --
 */


#define eStop    -1
static int state;      // number of requests available in the buffer


char buf[MAX_STRING];
char *pStart, *pEnd;

CRITICAL_SECTION critSec;
HANDLE hThread = NULL;

DWORD WINAPI RunInput(LPVOID pDummy)
   {

   char c;
   int numRead;
   while (state != eStop)
      {
      numRead = read(0, &c, 1);         // BLOCKING read of stdin

      if (numRead == 1)
         {
         EnterCriticalSection(&critSec);
         if (c == '\n')
            {
            c = 0;            // null terminate the input
            if (state != eStop)
               state++;
            }
         *pEnd++ = c;
         if (pEnd - buf >= MAX_STRING)
            {
            buf[MAX_STRING-1] = 0;
            pEnd = buf;
            }
         LeaveCriticalSection(&critSec);
         }
      }
   return 0;
   }


int Input(char *str)
   {

   if (state <= 0)
      return 0;

#ifdef LOG
      
   if (logFile)  
     fprintf(logFile, "< %s\n", pStart);
  
#endif

   EnterCriticalSection(&critSec);
   while ((*str++ = *pStart++) != 0)
      ;
   if (pStart == pEnd)
      pStart = pEnd = buf;
   else if (pStart - buf >= MAX_STRING)
      pStart = buf;
   state--;
   LeaveCriticalSection(&critSec);

   return 1;
   }


// InitInput         // start thread, init globals
void InitInput()
   {

   DWORD idThread;
   InitializeCriticalSection(&critSec);
   pStart = pEnd = buf;
   state = 0;

   hThread = CreateThread(NULL, 0, RunInput, NULL, 0, &idThread);
   }


void ShutdownInput()
   {

   DWORD code;

   // tell thread to quit
   state = eStop;

   // wake up thread (stuff a newline into input?)
   INPUT_RECORD inpRec;
   inpRec.EventType = KEY_EVENT;
   inpRec.Event.KeyEvent.bKeyDown = 1;
   inpRec.Event.KeyEvent.wRepeatCount = 1;
   inpRec.Event.KeyEvent.uChar.AsciiChar = '\r';      // Enter Key
   inpRec.Event.KeyEvent.dwControlKeyState = 0;
   inpRec.Event.KeyEvent.wVirtualKeyCode = 0;        
   inpRec.Event.KeyEvent.wVirtualScanCode = 0;       
   if (!WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inpRec, 1, &code))
      code = GetLastError();

   code = WaitForSingleObject(hThread, 1000);      // not needed?

   CloseHandle(hThread);

   DeleteCriticalSection(&critSec);
   }

/*
 * Function: waitForInput
 * Input:    None
 * Output:   None
 * Purpose:  Does nothing until some kind of input comes in.  Used when
 *           there's nothing to do (someone has forced mate for instance).
 */

void waitForInput()
   {
   while (!checkInput()){};
   }


/* 
 * Function: checkInput
 * Input:    None
 * Output:   1 if there was input, 0 if not
 * Purpose:  Used by pollForInput to see if Sunsetter has to handle anything.
 *           It calls select() to see if there is input waiting and if there
 *           is, it calls parseOption().  It also checks if Sunsetter has taken too much time on this move and 
 *           should stop thinking and move.
 */

int checkInput()
   {
   int wasInput = 0;
   char str[MAX_STRING];

   // this (as well as whole interface.cpp) was substantially changed in 7g
   // didnt have time yet to understand the changes


   if (gameBoard.timeToMove()) stopThought();

   if (analyzeMode && gameInProgress && !xboardMode) analyzeUpdate();

   if (!hThread)
      {
      InitInput();
      }
   while (Input(str))
      {
      wasInput = 1;
      parseOption(str);

      }

   return wasInput;
   }


#endif

#ifndef _win32_

/*
 *								    -- LINUX --
 */


// This function attempts to do a non-blocking read from stdin, and
// if a whole line is read then it returns this line in the memory
// pointed to by str.  str should point to at least MAX_STRING bytes
// of memory. --Angrim

int Input(char *str)
{
	static char buf[MAX_STRING];
	static int insert_pt=0;
	fd_set stdin_holder;
	struct timeval tv;
	int ret;
	char ch;

	FD_ZERO(&stdin_holder);
	FD_SET(0, &stdin_holder);
	// if no input, don't wait
	tv.tv_sec = 0; tv.tv_usec = 0;
	// while there is more input on stdin, and have not read a whole line
	while( select(1, &stdin_holder, NULL, NULL, &tv) > 0 ){
		ch=0;
		// read one byte
		ret = read(0, &ch, 1);
		// if the select says there is activity on stdin, and
		// read 0 bytes, then this is an end of file. handle it by quit.
		if( ret==0 ) {strcpy(str, "quit"); return 1;}
		if(ch=='\n') {
			// finished reading one line, time to return it.
			buf[insert_pt]='\0';
			strcpy(str, buf);
			insert_pt=0;
#ifdef LOG
			if (logFile)
			{
				fprintf(logFile, "< %s\n", buf);
				fflush(logFile);
			}
#endif
			return 1;
		}
		// got a general character, and there is room left in buf[]
		if( (ch != 0) && (insert_pt<MAX_STRING-1) ){
			buf[insert_pt]=ch;
			insert_pt++;
		}
	}
	return 0;
}

void waitForInput()
{
	// while no input, sleep for 1/1000 seconds
	while (!checkInput())usleep(1000);

}


/* 
 * Function: checkInput
 * Input:    None
 * Output:   1 if there was input, 0 if not
 * Purpose:  Used by pollForInput to see if Sunsetter has to handle anything.
 *           It calls select() to see if there is input waiting and if there
 *           is, it calls parseOption().  It also checks if Sunsetter has taken too much time on this move and 
 *           should stop thinking and move.
 */

int checkInput()
{
   int wasInput = 0;
   char str[MAX_STRING];
                     
   if(gameBoard.timeToMove()) stopThought();

   if (analyzeMode && gameInProgress && !xboardMode) analyzeUpdate();

   while (Input(str))
   {
      wasInput = 1;
      parseOption(str);
   }

   return wasInput;
}


#endif // not _win32_



/* 
 * Function: output
 * Input:    A string
 * Output:   None.
 * Purpose:  Used to print a string to the appropriate place.
 */

void output(const char *str)
   {
   int outputFD, value;
 
   outputFD = 1;
   
   do 
      { 
      value = write(outputFD, str, strlen(str));
      } while(value < 0 && errno == EINTR);

   if (value < 0) 
      {
      perror("output()");
      exit(1);
      }

#ifdef LOG
   static int startOfLine = 1;
   if (logFile) 
      {
      if (startOfLine) 
         {
         fprintf(logFile, "> %s", str);
         startOfLine = 0;
         } 
      else 
         {
         fprintf(logFile, "%s", str);
         }

      if (strchr(str, '\n') != NULL)  
         startOfLine = 1;
      fflush(logFile);
      }
#endif

   return;
   }


#endif  // #ifdef __EMSCRIPTEN__



/* 
 * Function: reportResult
 * Input:    A result
 * Output:   None
 * Purpose:  Used to report the result of a finished game.
 */

void reportResult(gameResult res)
   {



   switch (res) 
      {
      case WHITE_MATE:
 
            output("1-0 {White Mates}\n");

		 break;

  
      case BLACK_MATE:
       
            output("0-1 {Black Mates}\n");
		
		 break;
      
      case WHITE_RESIGNATION:
      
            output("0-1 {White Resigns}\n");
   
         break;
    
      case BLACK_RESIGNATION:
        
            output("1-0 {Black Resigns}\n");
       
         break;
    
      case WHITE_FLAG_FALL:

            output("0-1 {White forfeits on time}\n");
       
         break;

      case BLACK_FLAG_FALL:
      
            output("1-0 {Black forteits on time}\n");
      
         break;

      case BOTH_FLAG_FALL:
   
            output("1/2-1/2 {Draw because both players ran out of time}\n");

         break;
  
      default:
         output("The game ended mysteriously\n");
      }

}    


/* 
 * Function: parseHolding
 * Input:    A string
 * Output:   None
 * Purpose:  Used to add pieces to the player's hands
 */

void parseHolding(const char *str)
   {
   int i, hand[COLORS][PIECES];
   color c;
   piece p;

   i = 0;
   memset(hand, 0, sizeof(hand));
   for (c = WHITE; c <= BLACK; c = (color) (c + 1))
      {
      while (str[i++] != '[')
         if(str[i] == 0)
            return;
    
      while (str[i] != ']')
         {
         switch(str[i++])
            {
            case 'p':
            case 'P':
               hand[c][PAWN]++;
               break;
            case 'q':
            case 'Q':
               hand[c][QUEEN]++;
               break;
            case 'r':
            case 'R':
               hand[c][ROOK]++;
               break;
            case 'b':
            case 'B':
               hand[c][BISHOP]++;
               break;
            case 'n':
            case 'N':
               hand[c][KNIGHT]++;
               break;
            default:
               return;
            }
         }
      }
   for (c = WHITE; c <= BLACK; c = (color) (c + 1))
      for(p = PAWN; p <= QUEEN; p = (piece) (p + 1))
         gameBoard.setPieceInHand(c, p, hand[c][p]);
   }


/*
 * Function: parseOption
 * Input:    The line that was inputed
 * Output:   None
 * Purpose:  Used to understand what the user is trying to say.
 */

void parseOption(const char *str)
   {

   int n; 
   move m;
   char tmp[MAX_STRING], buf[MAX_STRING], arg[MAX_ARG][MAX_STRING];

   static char partnerName[MAX_STRING];
   int retval;
  
   /* Parse the arguments */
   arg[0][0] = arg[1][0] = arg[2][0] = arg[3][0] = arg[4][0] = '\0';
   sscanf(str, "%s %s %s %s %s", arg[0], arg[1], arg[2], arg[3], arg[4]);

   /* Ignore options xboard sends that don't mean much or 
       ones that we dont care about */
  
   if ((!strcmp(arg[0], "")) ||
   (!strcmp(arg[0], "beep")) || 
   (!strcmp(arg[0], "random")) ||
   (!strcmp(arg[0], "bogus")) ||
   (!strcmp(arg[0], "draw")) ||  /* Draws in bughouse are for wimps */
   (!strcmp(arg[0], "level")) || /* Incs in bughouse are for drawers */   
   (!strcmp(arg[0], "zchall")) ||
   (!strcmp(arg[0], "name")) ||
   (!strcmp(arg[0], "set")) ||
   (!strcmp(arg[0], "iset")) ||
   (!strcmp(arg[0], "accepted")) || // response to feature requests, no-op

   (!strcmp(arg[0], "post")) || /* Sending Illegal Move will cause trouble */
   (!strcmp(arg[0], "computer")))
   


   {
#ifdef DEBUG_XBOARD
		output ("//D: command we don't care about, ignoring\n"); 
#endif
	return; 
   }
   

   /* Process the other options */

	if (!strcmp(arg[0], "xboard")) 
		{
		xboardMode = 1; 
		}
	else if (!strcmp(arg[0], "protover"))
	{
		// Tell xboard which modern features we support.  short list so far.
		// Remember that "string" features must be quoted even when they do
		// not have any spaces in them.
		sprintf(buf, "feature ping=0 draw=0 sigint=0 setboard=1 analyze=1 memory=1 myname=\"Sunsetter%s%d%d\" variants=\"crazyhouse,bughouse\" done=1\n", VERSION, paramA, paramB);
		output(buf);
	}

	else if (!strcmp(arg[0], "learn")) 
		{
		learning = 1; 
		readLearnTableFromDisk(); 
		output("Learning is on.\n");
		}
	else if (!strcmp(arg[0], "memory"))
	{
		int totalram = atoi(arg[1]);
		int hashgoal = totalram - 6; // current estimate, 6meg for non-hash stuff
		if (hashgoal<16) {
			output("requested hash size too small, using default\n");
			hashgoal = 16;
		}
		makeTranspositionTable(hashgoal * 1024 * 1024); // hashgoal megabytes
}

	else if (!strcmp(arg[0], "analyze")) 
		{
		analyzeMode = 1;      
		forceMode = 0; 
		gameBoard.setDeepBugColor(gameBoard.getColorOnMove());
		gameBoard.setLastMoveNow();
		startSearchOver();
		// last 3 lines suggested by Angrim
		}
	else if (!strcmp(arg[0], "exit"))
	{
		analyzeMode = 0;
		forceMode = 1;
		resetAI();
	}
	else if (!strcmp(arg[0], "hard")) 
		{
		tryToPonder = 1;
		} 
   
	else if (!strcmp(arg[0], "easy")) 
		{
		tryToPonder = 0;
		} 
   
	else if (strstr(arg[0], "result") == arg[0]) 
		{      
#ifdef DEBUG_XBOARD
		output ("//D: result parsed, saving learn info, resetting AI\n"); 
#endif
	   
		if ((currentRules == CRAZYHOUSE) && (learning))

		{
		
		if (ratingDiff > 500) {ratingDiff = 500; }
		if (ratingDiff <-500) {ratingDiff =-500; }


		if (!strcmp(arg[1],"1-0"))
		{
		  
		  
		  if (gameBoard.getDeepBugColor() == WHITE)
		  {
		  gameBoard.saveLearnTable ((-ratingDiff / 5) + 120); 
		  }
		  else
		  {
		  gameBoard.saveLearnTable ((-ratingDiff / 5) - 120); 
		  }
		}
		if (!strcmp(arg[1],"0-1"))

		{
		  
		  if (gameBoard.getDeepBugColor() == BLACK)
		  {
		  gameBoard.saveLearnTable ((-ratingDiff / 5) + 120); 
		  }
		  else
		  {
		  gameBoard.saveLearnTable ((-ratingDiff / 5) - 120); 
		  } 
		}
		}

	  
	  
	  resetAI();
      gameInProgress = 0;

      } 
   
   else if (!strcmp(arg[0], "new") ||
            !strcmp(arg[0], "accept")) 
		
		{	  	


#ifdef DEBUG_XBOARD
		output ("//D: new or accept parsed, ignoring\n"); 
#endif
		}



   else if(!strcmp(arg[0], "variant") ||
           !strcmp(arg[0], "reset")) 
      {


	  if (xboardMode && !analyzeMode) 
	  {
		  output("tellics kibitz Hello from Sunsetter "); 
		  output (VERSION); 
	  }



	  if (PERSONALITY) 
	  {
	 
	  srand(time(NULL));
	  n = rand();   
	  n = (n % (PERSONALITY)) ; 
	  
	  setDefaultValues(); 
	  ReadIniFile(personalityIni[n]); 
	  
	  output(" Personality: "); output(personalityIni[n]);
	  
	  }

	  output (" \n");
		
	  stopThought(); 
	  gameBoard.resetBoard();
      zapHashValues();
      resetAI();
	  
      gameInProgress = 1;
	  soughtGame = 0; 

#ifdef DEBUG_XBOARD
output ("//D: variant parsed, board reset and set to bug or zh \n"); 
#endif
			if (!strcmp(arg[1], "bughouse")) 
				{				
				currentRules = BUGHOUSE;
				gameBoard.playBughouse();
				}
			else       
				{         
				currentRules = CRAZYHOUSE;
				gameBoard.playCrazyhouse();         
				}

	  if (analyzeMode) gameBoard.setDeepBugColor(gameBoard.getColorOnMove());         

      }
  
   else if (!strcmp(arg[0], "partner")) 
      {
      if (!strcmp(arg[1], "")) 
         {
         strcpy(partnerName, "");
         partner = 0;
         output("tellics set formula f4 && f2\n");
         }
      else if (strcmp(partnerName, arg[1]) != 0) 
         {
         if (arg[1][0] == '\a') 
            strcpy(partnerName, arg[1] + 1); /* fics sends an 
                       alarm to go with
                       the partner command
                       skip over it*/
         else 
         strcpy(partnerName, arg[1]);
         partner = 1;
         givePartnerHelp("intro");         
         output("tellics set formula f4 && f1 \n");
         output("tellics unseek\n");
         };
      } 
   else if (!strcmp(arg[0], "?")) 
      {
      unsit();
      stopThought();
      forceDeepBugToMove();
      }

   else if (!strcmp(arg[0], "resign")) 
      {
      if (gameBoard.getDeepBugColor() == WHITE)
         reportResult(BLACK_RESIGNATION);
      else 
         reportResult(WHITE_RESIGNATION);
      gameInProgress = 0;
      }
	
   else if(!strcmp(arg[0], "personality")) 
   {
   output("found personality\n"); 
   strcpy(personalityIni[PERSONALITY], arg[1]); 
   PERSONALITY ++; 
   }   
   
   else if(!strcmp(arg[0], "hash")) 
      {	  
      if (makeTranspositionTable(atoi(arg[1]) * 1024 * 1024) == -1)
         makeTranspositionTable(MIN_HASH_SIZE); 
		 /* There was an error, so make the
                  table the minimum size. */
      }

   else if (!strcmp(arg[0], "tellics"))
		{ output("\ntellics "); output(arg[1]); output("\n");  }
   

   else if (!strcmp(arg[0], "rating")) 
		{ 
			ratingDiff = atoi(arg[1]) - atoi(arg[2]); 

#ifdef DEBUG_XBOARD
			sprintf(buf, "//D: rating parsed, Diff: %d \n", ratingDiff); 
			output (buf); 
#endif 
		}
	
   else if(!strcmp(arg[0], "sd"))
   { 
		FIXED_DEPTH = atoi(arg[1]);
   }
   else if (!strcmp(arg[0], "snodes"))
   {
	   FIXED_NODES = atoi(arg[1]);
   }

   else if(!strcmp(arg[0], "nullreduct"))
	  { NULL_REDUCTION = atoi(arg[1]); }

   else if(!strcmp(arg[0], "capext"))
	  { CAPTURE_EXTENSION = atoi(arg[1]); }
   
   else if(!strcmp(arg[0], "checkext"))
	  { CHECK_EXTENSION = atoi(arg[1]); }

   else if(!strcmp(arg[0], "forceext"))
	  { FORCING_EXTENSION = atoi(arg[1]); }

   else if(!strcmp(arg[0], "qvalue"))
	  { pValue[QUEEN] = atoi(arg[1]); }
   
   else if(!strcmp(arg[0], "rvalue"))
	  { pValue[ROOK] = atoi(arg[1]); }

   else if(!strcmp(arg[0], "bvalue"))
	  { pValue[BISHOP] = atoi(arg[1]);}	
   
   else if(!strcmp(arg[0], "nvalue"))
	  { pValue[KNIGHT] = atoi(arg[1]);}	

   else if(!strcmp(arg[0], "bcfactor"))
	  { BC_FACTOR = atoi(arg[1]);}	   

   else if(!strcmp(arg[0], "nkfactor"))
	  { NK_FACTOR = atoi(arg[1]);}	
   
   else if(!strcmp(arg[0], "defactor"))
	  { DE_FACTOR = atoi(arg[1]);}	

   else if (!strcmp(arg[0], "paramA"))
   {
	   paramA = atoi(arg[1]);
   }

   else if (!strcmp(arg[0], "paramB"))
   {
	   paramB = atoi(arg[1]);
   }
   
   else if(!strcmp(arg[0], "time"))
      gameBoard.setTime(gameBoard.getDeepBugColor(), atoi(arg[1]) *10);

   else if(!strcmp(arg[0], "otim"))
      gameBoard.setTime(otherColor(gameBoard.getDeepBugColor()), atoi(arg[1]) *10);
    
   else if(!strcmp(arg[0], "white")) 
      {
      gameBoard.setColorOnMove(WHITE);
      gameBoard.setDeepBugColor(BLACK);
      } 
   else if (!strcmp(arg[0], "black")) 
      {
      gameBoard.setColorOnMove(BLACK);
      gameBoard.setDeepBugColor(WHITE);
      }

   else if (!strcmp(arg[0], "go")) 
      {
      if (gameInProgress) 
         {
         forceMode = 0;
		 analyzeMode = 0; 
		 gameBoard.setDeepBugColor(gameBoard.getColorOnMove());         
		 gameBoard.setLastMoveNow();
         startSearchOver();
         }
      }

   else if (!strcmp(arg[0], "quit")) 
      {
	  if (learning) saveLearnTableToDisk(); 

#ifdef _win32_      

	  ShutdownInput();

#endif
	  
	  exit(0);
      }
  
   else if (!strcmp(arg[0], "undo")) 
      {
      if (gameBoard.unplayMove()) 
       output("Cannot undo move\n");
	  stopThought();
	  if (analyzeMode) gameBoard.setDeepBugColor(gameBoard.getColorOnMove());         

      }
  
   else if (!strcmp(arg[0], "remove")) 
      {
      if (gameBoard.getMoveNum() < 3) 
         {
         output("Cannot remove last move\n");
         } 
      else 
         {
         gameBoard.unplayMove();
         gameBoard.unplayMove();
		 stopThought();
         }

      }

   else if (!strcmp(arg[0], "force")) 
      {
   	  forceMode = 1; 
	  

	  if (gameInProgress) 
	  {
      if (!analyzeMode) startSearchOver();
	  else stopThought(); 
	  }

	  analyzeMode = 0; 

      }
#ifdef DEBUG_STATS
   else if (!strcmp(arg[0], "debug")) 
      gameBoard.showDebugInfo();
#endif
   else if (!strcmp(arg[0], "setboard")) 
   {
	   stopThought();
	   currentRules = CRAZYHOUSE;
	   gameBoard.playCrazyhouse();
	   gameBoard.setBoard(arg[1], arg[2], arg[3], arg[4]);
	   zapHashValues();
	   resetAI();

	   gameInProgress = 1;
	   soughtGame = 0;

#ifndef NDEBUG
	   gameBoard.showDebugInfo();
#endif

	   if (analyzeMode) gameBoard.setDeepBugColor(gameBoard.getColorOnMove());
   }

   else if (!strcmp(arg[0], "holding")) 
      {
      if (currentRules != CRAZYHOUSE) 
         {  /* In crazyhouse we should already know
                what is in each player's hand */
		 
		 strcpy(tmp, arg[1]);
         strcat(tmp, arg[2]);
         parseHolding(tmp);

		}
      }
   else if (!strcmp(arg[0], "."))
   {
	   if (analyzeMode && gameInProgress) analyzeUpdate();
   }

   else if (!strcmp(arg[0], "ptell")) 
      {
      partner = 1;                 /* We should have gotten "partner" before,
									but just to make sure */
      if ((retval = parsePartnerCommand(arg[1], arg[2])) == -1) 

         {
         if ((strstr(arg[1], "Sorry") == NULL) && (strstr(arg[1], "OK,") == NULL))
            { /* This will be an endless loop
                   with other Sunsetter's and SkySharks */
            sprintf(buf, "tellics ptell Sorry , I didn't understand %s %s\n",
                     arg[1], arg[2]);
            output(buf);
            }
         }

      /* If it doesn't fit any thing else see if it's a move */
      } 
   else 
      {
       
	   if (!gameInProgress) 
         {
         sprintf(buf, "Illegal move: %s\n", str);
         output(buf);
         return;
         }
    
      m = gameBoard.algebraicMoveToDBMove(arg[0]);
      if (!gameBoard.isLegal(m)) 
         {
         sprintf(buf, "Illegal move: %s\n", str);
         output(buf);
         return;
         }
      if ((gameBoard.getColorOnMove() == gameBoard.getDeepBugColor()) 
            && !forceMode && !analyzeMode)
			output("It is not your move");
	     
	  if (gameBoard.playMove(m,0)) 
         {
         sprintf(buf, "Tried to play illegal move: %s\n", str);
         output(buf);
         } 
      else 
         {
         stopThought(); /* Interrupt the pondering */
		 
		 // if (analyzeMode) gameBoard.setDeepBugColor(gameBoard.getColorOnMove());  
		/* Instead of the line above 7g Angrims Code (havnt yet checked why) */
		 if (analyzeMode) {
			 extern int stats_positionsSearched;
			 stats_positionsSearched = 0;
			 gameBoard.setDeepBugColor(gameBoard.getColorOnMove());
			 gameBoard.setLastMoveNow();
			 startSearchOver();
		 }

         }
      }
   }


   /*
   * Function: pollForInput
   * Input:    None
   * Output:   None
   * Purpose:  Used by search to poll for input.  Every 20,000 times it is called
   *           it calls checkInput() (about 10 times a second depending on the machine).
   */

   void pollForInput()
   {
	   static int i;

#ifndef __EMSCRIPTEN__
	   if (i++ > 20000)
	   {
		   checkInput();
		   i = 0;
	   }
#else
	   if (i++ > 20000)
	   {
		   i = 0;
		   checkInput();
		   emscripten_sleep_with_yield(10);
	   }
#endif  // #ifndef __EMSCRIPTEN__
   }


/*
 * Function: giveMove
 * Input:    A Move pointer
 * Output:   None
 * Purpose:  Tells what move Sunsetter wants to play.
 */

void giveMove(move m)
   {
   char str[MAX_STRING], buf[MAX_STRING];

   DBMoveToRawAlgebraicMove(m, str);
   if (str[0] == '\0')
      {
      sprintf(buf, "Illegal move: %02X to %02X in giveMove()\n", 
                m.from(), m.to());
      output(buf);
      return;
      }

      sprintf(buf, "move %s\n", str);
      output(buf);

   }

/*
 * Function: getSysMilliSecs
 * Input:    None
 * Output:   long
 * Purpose:  Returns the system time in Milliseconds, independent from the OS.
 */

long getSysMilliSecs()
{
double tmp; 

tmp=clock();
tmp*=1000; 
return long((tmp / CLOCKS_PER_SEC)) ; 

}


