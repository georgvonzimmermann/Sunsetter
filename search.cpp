/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: search.cc                                                          *
 *  Purpose: Has the functions to search the game tree                       *
 *                                                                           *
 *  Comments: search.cc has three main functions, findMove(), searchRoot ()  *
 * and search().  findMove() is called by the game loop to get the           *
 * (hopefully) best move in the position.  It sets up the search and calls   *
 * searchRoot() which for every move in the position calls search().         *
 * search() recursively searches and returns the value relating to how good  *
 * it thinks Sunsetter is doing in that position.  searchRoot() chooses the  *
 * move that causes search() to be the highest.                              *
 *                                                                           *
 * search() uses an Negamax search with alpha-beta pruning, NullMove Pruning *
 * plus other search extensions to find its evaluation of the position.   *
 *																			 *			
 * Because the Branching Factor in Crazyhouse and Bughouse is so big, and    *
 * deep mates so common we use a 4-steps PMG ( Plausible Move Generator )    *
 * compared to most chess programs which only do 2 steps, normal search and  *
 * quiesce search.															 *
 *																			 *
 *************************************************************************** */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "brain.h"
#include "bughouse.h"
#include "notation.h"
#include "interface.h"


PrincipalVariation pv;					/* The principal variation, there needs
                                         to be a separate one for each ply that
                                         is searched */

move searchMoves[DEPTH_LIMIT][MAX_MOVES]; /* Where to store the moves.  They
                                             used to be in a local array, but
                                             that blew up the stack */
boardStruct AIBoard;                 /* The board that the AI uses */
volatile int stopThinking;           /* If the search should be stopped */
volatile int reSearch;               /* If the search should be restarted */
volatile int forceMove;              /* Make a move, even if you get mated*/
double millisecondsPerMove;             /* How many millisecs to take on a move */
int stats_positionsSearched;               /* # of search() done */
int stats_quiescensePositionsSearched;     /* # of quieses() done */
int stats_transpositionHits;               /* # of success for transposition lookups*/
int stats_hashFillingUp; 
int stats_hashSize;


const int FractionalDeep[MAX_SEARCH_DEPTH + 1] = { 0, 0, ONE_PLY, ONE_PLY * 2, ONE_PLY * 3, ONE_PLY * 4, ONE_PLY * 5, ONE_PLY * 6, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72 };
		/* The implementation allows to experiment with fractional deepening, for example smaller steps at higher depths*/



int initialTime;
int currentDepth; 
int movesSearched;




#ifdef DEBUG_STATS

int stats_forceext, stats_checkext, stats_capext;  
int stats_NullTries[DEPTH_LIMIT], stats_NullCuts[DEPTH_LIMIT]; 
int stats_RazorTries, stats_Razors; 
int stats_MakeUnmake[MOVEGEN_TYPES]; 

#endif

/* Time */
clock_t startClockply, endClockply, startClockAnalyze; 
clock_t startClockTime, endClockTime; 


#ifdef GAMETREE

	char filename[MAX_STRING][DEPTH_LIMIT]; FILE *fi[DEPTH_LIMIT];

#endif


move overideMove;						/* If legal, this move is played instead
                                        of what the search found */

int pondering;							/* If we're pondering */
int tryToPonder;						/* If we should be pondering */


/* Function: PrincipalVariation::save
 * Input:    A move and the old PV
 * Output:   None.
 * Purpose:  Used to store the PV.
 */


__forceinline void savePrincipalVar(move m, int ply) 
{
	
assert ( pv.depth[ply] <= DEPTH_LIMIT );


    pv.moves[ply-1][0] = m;
    for(int i = 0; i < pv.depth[ply]; i++) pv.moves[ply-1][i + 1] = pv.moves[ply][i];
    pv.depth[ply-1] = pv.depth[ply] + 1;

}




/* Function: PrincipalVariation::print
* Input:    The board that the principal variation is from, the depth searched and the value.
* Output:   None.
* Purpose:  Used to print out information about the principal variation.
*/

static void printPrincipalVar(int valueReached)
{
	int n, timeUsed;
	int variationLength = 0;
	char buf[MAX_STRING], emoticon[MAX_STRING];
	char pvtxt[MAX_STRING];
	

	timeUsed = (getSysMilliSecs() - startClockTime) / 10; // time in centiseconds 
	if ((timeUsed < 2) && !analyzeMode) return;
	
	// translate mate values to xboard standard
	/*
	if (valueReached > MATE)
	{
		valueReached -= MATE_IN_ONE;
		valueReached *=2;
		valueReached += 32766;
	}

	if (valueReached < -MATE)
	{
		valueReached += MATE_IN_ONE;
		valueReached *=2;
		valueReached -= 32768;
	}
	*/

	if ((gameBoard.getColorOnMove() == BLACK) && analyzeMode)
		valueReached = -valueReached; // kinda a kludge, but it works

	strcpy(pvtxt, "");

	for (n = 0; n < pv.depth[0]; n++)
	{
		assert(!AIBoard.badMove(pv.moves[0][n]));

		AIBoard.changeBoard(pv.moves[0][n]); variationLength++;

		DBMoveToRawAlgebraicMove(pv.moves[0][n], buf);
		strcat(pvtxt, buf); strcat(pvtxt, " ");
	}

	if (!xboardMode) 
	{
		sprintf(buf, "%3d  %6d  %5d %8d ", currentDepth, valueReached,
			timeUsed, stats_positionsSearched);

		for (n = variationLength; n < 9; n++)
		{
			strcat(pvtxt, "     ");
		}

		output(buf); output(pvtxt); output("\n");

		if ((analyzeMode) && (timeUsed <20)) output("\r");
		else output("\n");
	}
	else 
	{ // xboard mode, and not done searching yet
		if ((timeUsed>15) || (currentDepth > 6))
		{ // only output after first 0.15 seconds OR at least depth 6
			sprintf(buf, "%d %d %d %d ",
				currentDepth, valueReached,
				timeUsed, stats_positionsSearched);
			output(buf); output(pvtxt); output("\n");
		}
	}

	for (n = variationLength - 1; n >= 0; n--) {
		AIBoard.unchangeBoard();
	}

	startClockply = getSysMilliSecs();
	startClockAnalyze = startClockply;
}

/* Function: analyzeUpdate
* Input:    None.
* Output:   None.
* Purpose:  If 4 secs or more are passed since we displayed any new
*			 info in analyze mode, do so now.
*/

void analyzeUpdate()

{
	char buf[MAX_STRING], buf2[MAX_STRING];

	if (xboardMode)
	{
		if (!searchMoves[0][movesSearched].isBad())
		{
			DBMoveToRawAlgebraicMove(searchMoves[0][movesSearched], buf2);
			// the 100 is in place of "total moves" because we don't have that here
			sprintf(buf, "stat01: %ld %d %d %d 100 %s \n",
				(getSysMilliSecs() - startClockTime) / 10,
				stats_positionsSearched,
				currentDepth,
				movesSearched, buf2);
			output(buf);
		}
	}
	else
		if (((getSysMilliSecs() - startClockAnalyze) > 4000) && ((startClockAnalyze - startClockTime) > 200))
		{
			if (!searchMoves[0][movesSearched].isBad())
			{
				DBMoveToRawAlgebraicMove(searchMoves[0][movesSearched], buf2);
				sprintf(buf,
					"             %5ld %8d  searching: %s ..   ( HT: %2d percent )\r",
					((getSysMilliSecs() - startClockply) / 10), stats_positionsSearched,
					buf2, (stats_hashFillingUp * 100 / stats_hashSize));
				output(buf);
			}
			startClockAnalyze = getSysMilliSecs();
		}
	return;
}


/* Function: calcTimeToSpend
 * Input:    What depth we're on currently
 * Output:   None,
 * Purpose:  Used by searchRoot to set millisecondsPerMove to the correct amount
 *           of time.
 */

void calcTimeToSpend()

{

	int mytime, opptime;

	mytime = AIBoard.getTime(AIBoard.getDeepBugColor());
	opptime = AIBoard.getTime(otherColor(AIBoard.getDeepBugColor()));

	if ((FIXED_DEPTH) || (analyzeMode))

	{
		millisecondsPerMove = 100000000; 
		return; 
	}

	if (currentRules == BUGHOUSE) 

	{
		if (opptime < mytime) mytime = opptime; // if opp is low on time, play fast too 
												// he and my part may have sat 
												// so maybe my part is low too 
		millisecondsPerMove = ( initialTime / 360 ) + ( mytime / 360 ); 
		if (mytime <= 20000) { millisecondsPerMove = mytime / 180; }
		if (mytime <= 4000) { millisecondsPerMove = 40; }
		if (mytime <= 800) { millisecondsPerMove = 5; }
	}

	else 
	{											   
		millisecondsPerMove = ( mytime / 25 ); 
		if (mytime <= 20000) { millisecondsPerMove = mytime / 40; }
		if (mytime <= 4000) { millisecondsPerMove = 100; }
		if (mytime <= 800) { millisecondsPerMove = 20; }
	
	}

}


/* Function: searchFirstMove
 * Input:    What the first move is, how deep to search, and a guess at what
 *           the value for the position is.
 * Output:   That value of the position after the first move.
 * Purpose:  Used by searchRoot to find the value of the first move.  This
 *           is done differently then all of the other moves because it has
 *           to find and exact value.  searchFirstMove() sets alpha to be
 *           the guess and beta to be the guess + 1.  This will either fail
 *           high or low, then searchFirstMove() sees if the value of the
 *           position is a little away from the guess.  If the search fails
 *           high or low again then searchFirstMove() gets the exact value,
 *           no matter how high/low it is.
 */

int searchFirstMove(move m, int depth, int guess)
{
  int learnValue; 
  int alpha, beta, value;

  alpha = guess ;
  beta = guess +1;

assert (depth < MAX_SEARCH_DEPTH * ONE_PLY);
assert (guess >= -INFINITY);
assert (guess <= INFINITY); 
assert (!AIBoard.badMove(m));

  AIBoard.changeBoard(m);

  learnValue = AIBoard.checkLearnTable(); 
  
  // the idea is to play faster or slower in the opening depending on
  // whether we did well or bad in the past  in this position
  
  millisecondsPerMove -= learnValue *4; 

#ifdef DEBUG_LEARN
  if ((learnValue) && (currentDepth == 4))
  {
	  char buf[MAX_STRING], buf2[MAX_STRING]; 
	  DBMoveToRawAlgebraicMove(m, buf);
	  sprintf (buf2,"Learn: %s has push Value of %d\n", buf, learnValue); 
	  output (buf2); 
  }
#endif

  
  value = learnValue -search(-beta+ learnValue, -alpha+ learnValue, depth - ONE_PLY, 1, 0);

   AIBoard.unchangeBoard();

   if(stopThinking) return -INFINITY;

   if(value >= beta) {   

    // The null window search failed high  

    alpha = value;
    beta = INFINITY;

assert (!AIBoard.badMove(m));    
	
	AIBoard.changeBoard(m);

	// not -beta + learnValue, since -beta = -INFINITY and we can't allow that to get smaller !
	value = learnValue -search(-beta, -alpha+learnValue, depth - ONE_PLY, 1, 0);

    AIBoard.unchangeBoard();

    if(stopThinking) return -INFINITY;

  
    else return value;
  } else {

    // The null window search failed low  

    beta = value;
    alpha = -INFINITY; 
    
assert (!AIBoard.badMove(m));	
	AIBoard.changeBoard(m);
	
	// not -alpha + learnValue
	value = learnValue -search(-beta+learnValue, -alpha, depth - ONE_PLY, 1, 0);

    AIBoard.unchangeBoard();

    if(stopThinking) return -INFINITY;
   
    else return value;
  }



}

/* Function: searchMove
 * Input:    What the move is, how deep to search, and alpha
 * Output:   That value of the position after the move.
 * Purpose:  Used by searchRoot to find the value of
 *           a move other then the first.  It first sees if the move
 *           is worse then the best one already found by setting beta
 *           to be alpha + 1.  If this move is better then it sees how
 *           much better it is, geting the exact value, no matter how high
 *           it is.  This is basically the same as searchFirstMove(),
 *           except if it fails low, then it doesn't search anymore,
 *           since it already knows that there is a better move.  
 */

int searchMove(move m, int depth, int alpha)
{

#ifdef GAMETREE
char buf[MAX_STRING];
char buf3[MAX_STRING];
#endif
	
  int learnValue, razor; 
  int beta, value;

  beta = alpha +1; 

assert (alpha >= -INFINITY);
assert (alpha <= INFINITY);
assert (depth <= MAX_SEARCH_DEPTH * ONE_PLY);  
assert (!AIBoard.badMove(m));  
  
  AIBoard.changeBoard(m);

  learnValue = AIBoard.checkLearnTable(); 
  millisecondsPerMove -= learnValue *4; 

#ifdef DEBUG_LEARN
  if ((learnValue) && (currentDepth == 4))
  {
	  char buf[MAX_STRING], buf2[MAX_STRING]; 
	  DBMoveToRawAlgebraicMove(m, buf);
	  sprintf (buf2,"Learn: %s has push Value of %d\n", buf, learnValue); 
	  output (buf2); 
  }
#endif

  
  // We don't razor if



	  if ( (AIBoard.isInCheck(AIBoard.getColorOnMove()))   
			// a) we are checking the opp			
			 ||  (AIBoard.highestAttacked(m.to()))
			// b) we are attacking something with our move thats worth more than or the same as our moved piece, or less defendet. 
			 ||  (AIBoard.escapingAttack(m.from(), m.to())) )
			// c) we are escaping with the piece that got attacked in the move before

		  // what about blocking attack ?
	  {
		  razor = 0; 

#ifdef GAMETREE
	  DBMoveToRawAlgebraicMove(m, buf);
	  sprintf (buf3,"<a href=\"=%s-%d.html\"><b>%s</b></a> <br>\n",buf, currentDepth,buf); 
	  fprintf (fi[0], buf3);
#endif
	  }
	  else
	  {
			  if (depth - ONE_PLY < 6 * ONE_PLY)
				  razor = -4;
			  else if (depth - ONE_PLY < 8 * ONE_PLY)
				  razor = -3;
			  else
				  razor = -2;	  

#ifdef GAMETREE
	  DBMoveToRawAlgebraicMove(m, buf);
	  sprintf (buf3,"<a href=\"=%s-%d.html\">%s</a> <br>\n",buf, currentDepth,buf); 
	  fprintf (fi[0], buf3);
#endif
	  }

 
  value = learnValue -search(-beta+learnValue, -alpha+learnValue, depth - ONE_PLY + razor , 1, 0);

  AIBoard.unchangeBoard();

  if(stopThinking) return -INFINITY;

  if(value < beta) return value;

  // This move is better then the best so far  

  alpha = value;
  beta = INFINITY;

assert (!AIBoard.badMove(m));
  AIBoard.changeBoard(m);

  // not -beta + learnValue, since -beta = -INFINITY and we can't allow that to get smaller !
  value = learnValue -search(-beta, -alpha+learnValue, depth - ONE_PLY + razor, 1, 0);

  AIBoard.unchangeBoard();

  if(stopThinking) return alpha;  


  return value;
}



/* Function: searchRoot
 * Input:    How many ply to search and a pointer to a move to fill with the
 *           found move.
 * Output:   None.
 * Purpose:  Uses an iterative alpha-beta search to assign a value to the
 *           position.  searchRoot() is what is called from the first ply.  It
 *           starts by doing a 1 ply search and then a 2 ply search up until it
 *           gets to the maximum depth.
 */

void searchRoot(int depth, move *rightMove, int *bestValue )
{
    
  int n, done;				/* needed for sorting moves */
  int bestValueEver, count, startDepth, searchedFirstMove;
  int value = -INFINITY; 

  move tmp, bestMoveLastPly;
  int values[MAX_MOVES];
  
  char buf[MAX_STRING];
  

  transpositionEntry *te;

  startClockTime = getSysMilliSecs();
  startClockply = getSysMilliSecs();

  *bestValue = -INFINITY;
  bestValueEver = -INFINITY; 
  values[1] = -INFINITY;

  bestMoveLastPly.makeBad();
  
  searchedFirstMove = 0; 
  startDepth = 1;

  AIBoard.setCheckHistory(0);
  AIBoard.setBestCapture();
 
  count = AIBoard.moves(searchMoves[0]);  
  
				// if there is only 1 legal move play it
  if ((count == 1) && (currentRules == CRAZYHOUSE) &&  (!analyzeMode)) 
  {
    *rightMove = searchMoves[0][0];
    return;
  }
  if (count == 0) 
  {
	movesSearched = 0;
	searchMoves[0][0].makeBad(); 
	*rightMove = searchMoves[0][0];
	output("  0 -32000       0        0  #-0   \n");
	waitForInput();
	return; 
  }
 
  if((te = AIBoard.lookup()) != NULL) { // We've searched this position before
                                        // and can remember some information
                                        // about it 
    stats_transpositionHits++;


    if(te->type == EXACT && !te->hashMove.isBad()) {
   	  *rightMove = te->hashMove;
	  bestMoveLastPly = *rightMove;
      *bestValue = te->value;  // no adjustment for mate values needed, because this is ply 0.
	  value = *bestValue; 
	  pv.depth[0] = 0; pv.depth[1] = 0;
      n = 0;
	  
	  // these 4 lines sort the hash move to the top of moves to search
	  while(searchMoves[0][n] != te->hashMove) n++;
      tmp = searchMoves[0][n];
      searchMoves[0][n] = searchMoves[0][0];
      searchMoves[0][0] = tmp;
      
	  searchedFirstMove = 1;  
      startDepth = (te->depth / ONE_PLY) +1;

assert (n <= count); // this checks that the hash move we 
					 // got was in the moves we generated
      
	  sprintf(buf, "%3d  %6d      0%8d  ", startDepth, value, 0); 
	  output(buf);
	  DBMoveToRawAlgebraicMove(searchMoves[0][0], buf);
	  output(buf);
	  output(" <already searched>\n");		

    } else if(te->type != WORTHLESS) {
      *bestValue = te->value;	  
    }
  } 
  

  /* this is an easy and nice repetition detection :) 
   * the current position is stored into the hash with how much we'd like a draw (not very much if we higher rated than opp) and a  depth so big it 
   * will never be overwritten
   */
  tmp.makeBad(); 
  // commented out for now, needs further testing (or use version from chess sunsetter) @georg
  // if (!analyzeMode) AIBoard.store(MAX_SEARCH_DEPTH, tmp, -ratingDiff, -INFINITY, +INFINITY);

  calcTimeToSpend();

  // do the searches with increasing ply 

  for(currentDepth = startDepth;
      currentDepth < depth || sitting; currentDepth++) {

  movesSearched = 0; 

#ifdef GAMETREE
  char buf3[MAX_STRING];

  sprintf(filename[0],"Start");
  sprintf(buf3,"treemoves/%s-%d.html",filename[0],currentDepth);
  fi[0] = fopen(buf3,"wt");
  fprintf(fi[0],"<link rel=\"stylesheet\" href=\"../style/style.css\" type=\"text/css\">\n");
  
  
  fprintf(fi[0],"<html><table cellpadding=10><tr><td valign=top>\n"); 
  
  printHtmlBoard(fi[0]); 
  fprintf(fi[0],"</td><td valign=top>\n");
  
#endif

  if ((currentRules == CRAZYHOUSE) && (*bestValue <= -EXTREME_EVAL) && (currentDepth > 5))

  {	  
	  millisecondsPerMove *= 2; 
  }


  if (((gameBoard.timeToMove() && (currentDepth >= 2))|| ((FIXED_DEPTH) && (currentDepth >= FIXED_DEPTH))) && (!sitting)) 
  { 
	  stopThought(); 	  
	  break;
  }

  
  if(!searchedFirstMove) 
  {	
	  
	  value = searchFirstMove(searchMoves[0][0], FractionalDeep[currentDepth], *bestValue);

	

#ifdef GAMETREE
	  DBMoveToRawAlgebraicMove(searchMoves[0][0], buf);
	  sprintf (buf3,"<b><a href=\"=%s-%d.html\">%s</a></b>  Return Value: %d <br>\n",buf, currentDepth,buf,value); 
	  fprintf (fi[0], buf3);
#endif
	
    
  } 
  else 
  {			
	   searchedFirstMove = 0;
  } 
	 
  if(stopThinking) break;

  *bestValue = values[0] = value;
  *rightMove = searchMoves[0][0];

  if (value > bestValueEver)
  {
	  bestMoveLastPly = *rightMove;
	  bestValueEver = value; 
  }
  

  savePrincipalVar(*rightMove, 1);    
  printPrincipalVar(*bestValue);

  /* The following doesnt do what I thought it does, but it gives good results anyway */
  
  if ((currentRules == CRAZYHOUSE) 
	  && ((value +40) < values[1]) 
	  && (currentDepth > 7) 
	  && (millisecondsPerMove * 8 < gameBoard.getTime(AIBoard.getColorOnMove())) )
  {
	  millisecondsPerMove = (millisecondsPerMove /2) * 3;
  }   
  
  while(movesSearched < (count-1)) 
  
  {	
	  movesSearched++;
	  
	  value =  searchMove(searchMoves[0][movesSearched], FractionalDeep[currentDepth], *bestValue );	 	  
	
      values[movesSearched] = value;

	  // we are out of time, and are not failing high  

	  if ((stopThinking) && (values[movesSearched]) == -INFINITY)
		  break;

	  //  We have a new best move, let's save it 
	  
	  if(value > (*bestValue)) 
	  {
			*rightMove = searchMoves[0][movesSearched];
			*bestValue = value ; 
			savePrincipalVar(*rightMove, 1);
			printPrincipalVar(*bestValue);	  
	  }
	  // use this to print a variation for each move tried.
	  // savePrincipalVar(searchMoves[0][movesSearched], 1);
	  // printPrincipalVar(value);	  

	  // we are out of time, but the last move from searchMoves() 
	  // is already failing high. Play that even though we dont know 
	  // just how good it is

	  if (stopThinking)
		  break;
  
  }	

    // Sort the moves based on the new values 

    do {
      done = 1;
      for(n = 0; n < count - 1; n++) {
        if(values[n + 1] > values[n]) {
          tmp = searchMoves[0][n];
          searchMoves[0][n] = searchMoves[0][n + 1];
          searchMoves[0][n + 1] = tmp;
          value = values[n];
          values[n] = values[n + 1];
          values[n + 1] = value;
          done = 0;
        }
      }
    } while(!done);

  if (((*bestValue > MATE) || (*bestValue < -MATE) || (values[1] < -MATE_IN_ONE + 3)) && (!analyzeMode)) stopThought();

  // if we mate, are mated or have only 1 move to escape a short mate, then move now in zh.

  if (stopThinking) break;


#ifdef GAMETREE
  fprintf(fi[0],"<br><br>Return: end of searchRoot<br></td></tr></table></html>\n");
  fclose(fi[0]); 
#endif

} // end of iterative deepening

if ((*bestValue <= -EXTREME_EVAL) && (! bestMoveLastPly.isBad()) && (!analyzeMode))
 
{	// Entering swindle mode ... ;)
 
		*rightMove = bestMoveLastPly;

}

 

  if(!reSearch && !analyzeMode && !forceMode) {

   printPrincipalVar(*bestValue);   // whisper the last ply searched 
   endClockTime = getSysMilliSecs();

	output("\n");
    output("Found move: ");
    DBMoveToRawAlgebraicMove(*rightMove, buf);
    output(buf);
    sprintf(buf," %+d fply: %d  searches: %d quiesces: %d \n            T-hits: %d T-full: %d (percent)\n", *bestValue, currentDepth - 1, stats_positionsSearched, stats_quiescensePositionsSearched, stats_transpositionHits, (stats_hashFillingUp * 100 / stats_hashSize) );
    output(buf);

#ifdef DEBUG_STATS

    sprintf(buf,"Extensions: C-ext: %d F-ext: %d X-ext: %d (in fractional ply)\n", stats_checkext,stats_forceext,stats_capext);
	output(buf);
	sprintf(buf,"NullCuts  : depth-1: %d d-2: %d d-3: %d d-4: %d d-5: %d d-6: %d (percent)\n", (stats_NullCuts[CC_DEPTH+1] * 100 / (stats_NullTries[CC_DEPTH+1] +1)), (stats_NullCuts[CC_DEPTH+2] * 100 / (stats_NullTries[CC_DEPTH+2] +1)),(stats_NullCuts[CC_DEPTH+3] * 100 / (stats_NullTries[CC_DEPTH+3] +1)),(stats_NullCuts[CC_DEPTH+4] * 100 / (stats_NullTries[CC_DEPTH+4] +1)),(stats_NullCuts[CC_DEPTH+5] * 100 / (stats_NullTries[CC_DEPTH+5] +1)),(stats_NullCuts[CC_DEPTH+6] * 100 / (stats_NullTries[CC_DEPTH+6] +1)));
	output(buf);
	sprintf(buf,"Razor     : Tries: %d Success: %d\n",stats_RazorTries,stats_Razors);
	output(buf);
	sprintf(buf,"Make/Unm  : Hash: %d  All-Captures: %d Winning-Captures: %d \n            MateTries: %d Full: %d \n", (stats_MakeUnmake[HASH_MOVE]), (stats_MakeUnmake[ALL_CAP]), (stats_MakeUnmake[WINNING_CAP]), (stats_MakeUnmake[MATE_TRIES]), (stats_MakeUnmake[ALL_NON_CAP]) );
	output(buf);

#endif

	sprintf(buf,"Time      : Time Alloc: %d Clock Ticks Used (in Thousands): %d\n", (int)millisecondsPerMove, (int)(endClockTime - startClockTime));    
	output(buf); 
	
#ifdef DEBUG_HASH
	
	sprintf(buf,"Debug	  : Hash Collisions: %d\n", debug_allcoll); 
	output(buf); 
	assert (debug_allcoll == 0);

#endif
	
	output("\n\n"); 
 	stats_overallticks += (int) (endClockTime - startClockTime); 	
  }  

  if ((analyzeMode || forceMode) && ((currentDepth >= MAX_SEARCH_DEPTH) || ((FIXED_DEPTH) && (currentDepth >= FIXED_DEPTH))))
  {
    waitForInput();
  }

  return;

}

/* Function: ponder
 * Input:    None.
 * Output:   None.
 * Purpose:  Called when it's the opponents move.  It just searches
 *           until the opponents move.  This doesn't generate a move, but it
 *           puts information in the transposition tables.
 *			 Only used in Crazyhouse. 
 */

void ponder()
{

  move m[MAX_MOVES], tmp;

  int values[MAX_MOVES],  n, count, value, done;
  int extensions = 0;
  char buf[MAX_STRING], buf2[MAX_STRING];

  pondering = 1;

  gameBoard.copy(&AIBoard);
  AIBoard.setCheckHistory(0);
  AIBoard.setBestCapture();

  stopThinking = 0;
  currentDepth = 1;
  stats_hashFillingUp = stats_positionsSearched = 0;
  millisecondsPerMove = 100000000;
  count = AIBoard.moves(m);
  memset(values, 0, sizeof(values));
  startClockAnalyze = getSysMilliSecs();

  while(!stopThinking && currentDepth < MAX_SEARCH_DEPTH) 
  {

	  if (currentDepth > 4 && !xboardMode)
	  {
		  DBMoveToRawAlgebraicMove(m[0], buf2);
		  sprintf(buf, "pondering %s %6d [%2d-%2d]\n", buf2, -values[0],
			  currentDepth, currentDepth + (extensions / ONE_PLY));
		  output(buf);
	  }
	  else if (currentDepth > 4) 
	  {
		  DBMoveToRawAlgebraicMove(m[0], buf);
		  sprintf(buf2, "%d %d %ld %d pondering %s(hashfill %%%5.2f)\n",
			  currentDepth, -values[0],
			  (getSysMilliSecs() - startClockAnalyze) / 10,
			  stats_positionsSearched,
			  buf, stats_hashFillingUp * 50.0 / stats_hashSize);
		  output(buf2);
	  }

	  extensions = 0; 

	  for(n = 0; n < count; n++) 
	  {

		// Old version 
		// This was (actually intentionally :) ) comparing the current move to the one before. If there was a 0.8 pawn gap
		// in valuation, the current move was searched less deep. I guess extensive tests would be needed here (georg)
		// if (n && (currentDepth > 3) && (values[n-1] > values[n]+80)) extensions -= ONE_PLY;
	  
		// Angrims version
		// moves that are 0.8 pawns worse than the best get searched less
		// and if they are 1.5 pawns worse, they get even less
		
		if (n && (currentDepth > 3)) {
			if (values[0] > values[n] + 80)
				extensions = -ONE_PLY;
			else if (values[0] > values[n] + 150)
				extensions = -ONE_PLY * 2;
		}
		

		AIBoard.changeBoard(m[n]);

		values[n] = -search(-INFINITY, +INFINITY, FractionalDeep[currentDepth - 1] + extensions, 1, 1);

		AIBoard.unchangeBoard();

		if(stopThinking) break;

	  }


    
  if(stopThinking) break;

    /* Sort the moves based on the new values */

    do {
      done = 1;
      for(n = 0; n < count - 1; n++) {
        if(values[n + 1] > values[n]) {
          tmp = m[n];
          m[n] = m[n + 1];
          m[n + 1] = tmp;
          value = values[n];
          values[n] = values[n + 1];
          values[n + 1] = value;
          done = 0;
        }
      }
    } while(!done);
    currentDepth++;

  }
  
  pondering = 0;

  output("\n");
  if(currentDepth >= MAX_SEARCH_DEPTH) {
    /* we've gone as far as we can, wait for it to be our move */
    while(gameBoard.getColorOnMove() != gameBoard.getDeepBugColor())
    waitForInput();
  }
  return;
}

/* |-------------| ==> |------------------|
 * |             |     | searchFirstMove  | ==> |--------|      |------------------------|
 * |             |     |------------------|     |        | ==>  | recursiveCheckEvasion  |
 * | searchRoot  |                              | search | ==>  | recursiveFullSearch ...|
 * |             | ==> |------------------|     |        | ==>  |------------------------|
 * |             | ==> | searchMove       | ==> |--------|          ||
 * |-------------| ... |------------------|         ^=================
 */                                                 


/* Function: recursiveCheckEvasion()
 *
 *
 */

inline void recursiveCheckEvasion(int *alpha, int *beta,int *bestValue, move *bestMove,int depthWithExtensions,int ply,move hashMove)

{

	int n, value;
	int realcount;
	int count = 0; 
	move *m; 
	m = searchMoves[ply]; 


	if (!hashMove.isBad())
	{
			m[0] = hashMove; 
			count = 1;
	}
	
	count += AIBoard.checkEvasionCaptures(m+count); 		
	count += AIBoard.checkEvasionOthers(m+count);			

	realcount = count;
	if (!hashMove.isBad()) realcount--;

	// if there is only 1 legal move 
	if (realcount == 1)
	{
		depthWithExtensions += FORCING_EXTENSION; 
		
		#ifdef DEBUG_STATS
		stats_forceext+= FORCING_EXTENSION; 
		#endif
	}
	else if ((realcount > 1) && (realcount < 4))
	{
		depthWithExtensions += 1;
	}
	// testing is unclear about this, keeping it because it seems right.
	else if (realcount > 8)
	{
		depthWithExtensions -= 1;
	}




	for(n = 0; n < count; n++) 
	
	{
		if ((m[n] == hashMove) && (n != 0) && (!hashMove.isBad())) continue; 


		assert (!AIBoard.badMove(m[n]));
		AIBoard.changeBoard(m[n]);
	
		// Recursive Search call 
	
		value = -search(-(*beta), -(*alpha), depthWithExtensions,  ply + 1, 0);

		AIBoard.unchangeBoard();	
	
	
#ifdef GAMETREE
		
		char buf[MAX_STRING], buf2[MAX_STRING], buf3[MAX_STRING];  
		
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{
		DBMoveToRawAlgebraicMove(m[n], buf);
		strcpy(buf2, filename[ply]); strcat(buf2, buf);
		sprintf (buf3,"<a href=\"%s-%d.html\"><b>%s</b></a>  Return Value: %d<br>\n",buf2,currentDepth, buf,value); 		
		
		fprintf (fi[ply], buf3); 
		}
#endif			
	

		
		if (value > *bestValue) 
		{	
			*bestValue = value;  
			*bestMove = m[n];
			savePrincipalVar(*bestMove,ply + 1);

		}
	
		if (*bestValue > *alpha) *alpha = *bestValue; 
	
		if (*bestValue >= *beta) return; 	
	
	}

}

/* Function: recursiveFullSearch()
 *
 *
 */

inline int recursiveFullSearch(int *alpha, int *beta, int *bestValue, move *bestMove, int depthWithExtensions, int  ply, move hashMove)

{
		int n, value, count; 


#ifdef GAMETREE
		char buf[MAX_STRING], buf2[MAX_STRING], buf3[MAX_STRING];  
		
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{ 
			fprintf(fi[ply],"<br>Full Search <a href=\"start.html#razor\">(*)</a>:<br><br>\n");
		}
#endif 
		

		move *m; 

		m = searchMoves[ply]; 
		
		count = AIBoard.aiMoves(m); 

		
		for ( n = 0; n < count; n++)
		{
			
			if ((m[n] == hashMove) && (!hashMove.isBad())) continue; 

	assert (!AIBoard.badMove(m[n]));

			AIBoard.changeBoard(m[n]);	

			// We don't razor if

			#ifdef DEBUG_STATS
			stats_RazorTries++;
			stats_MakeUnmake[ALL_NON_CAP]++;
			#endif
		
			if ( (AIBoard.isInCheck(AIBoard.getColorOnMove()))   
				// a) we are checking the opp
				 ||  (AIBoard.highestAttacked(m[n].to())) 
				// b) we are attacking something with our move thats worth more than or the same as our moved piece, or less defended. 
				 ||  (AIBoard.escapingAttack(m[n].from(), m[n].to())) )
				// c) we are escaping with the piece that got attacked in the move before
			

			{			

				// Recursive Search call 
				value = -search(-(*beta), -(*alpha), depthWithExtensions ,  ply + 1, 0);		
				AIBoard.unchangeBoard();

	#ifdef GAMETREE			
		
				if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
				{								
					DBMoveToRawAlgebraicMove(m[n], buf);
					strcpy(buf2, filename[ply]); strcat(buf2, buf);
					sprintf (buf3,"<a href=\"%s-%d.html\"><b>%s</b></a>  Return Value: %d<br>\n",buf2,currentDepth, buf,value); 				
					fprintf (fi[ply], buf3); 
				}
	#endif	


		}

		else	// razor


		{			
			#ifdef DEBUG_STATS
			stats_Razors++;
			#endif


			if (depthWithExtensions < 6 * ONE_PLY )
				value = -search(-(*beta), -(*alpha), depthWithExtensions - 4, ply + 1, 0);
			else if (depthWithExtensions < 8 * ONE_PLY)
				value = -search(-(*beta), -(*alpha), depthWithExtensions - 3, ply + 1, 0);
			else
				value = -search(-(*beta), -(*alpha), depthWithExtensions - 2, ply + 1, 0);

			

			AIBoard.unchangeBoard();

			#ifdef GAMETREE					
			if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
			{								
				DBMoveToRawAlgebraicMove(m[n], buf);
				strcpy(buf2, filename[ply]); strcat(buf2, buf);
				sprintf (buf3,"<a href=\"%s-%d.html\">%s</a> Return Value: %d<br>\n",buf2,currentDepth, buf,value); 				
				fprintf (fi[ply], buf3); 
			}
			#endif	
		}
			
	
			if (value > *bestValue) 
			{		
				*bestValue = value;  
				*bestMove = m[n];							
				savePrincipalVar(*bestMove, ply + 1);
			}						
			
			if (*bestValue > *alpha) *alpha = *bestValue; 
			

			if (*bestValue >= *beta) return 1;			
			
			
		}		

return 0; 
}

//  searching all checks
int recursiveChecks(int *alpha, int *beta, int *bestValue, move *bestMove, int depthWithExtensions, int ply, move hashMove)

{

	int n, value, count, captureGain;
	move *m;
	m = searchMoves[ply];
	
	value = -INFINITY;

	count = AIBoard.aiMoves(m);



	for (n = 0; n < count; n++)

	{
		if ((m[n] == hashMove) && (!hashMove.isBad())) continue;


		assert(!AIBoard.badMove(m[n]));


		AIBoard.changeBoard(m[n]);

		if (AIBoard.isInCheck(AIBoard.getColorOnMove()))
		{
			value = -search(-(*beta), -(*alpha), depthWithExtensions, ply + 1, 0);
		}
		AIBoard.unchangeBoard();

		if (value > *bestValue)
		{
			*bestValue = value;
			*bestMove = m[n];
			savePrincipalVar(*bestMove, ply + 1);

		}

		if (*bestValue > *alpha) *alpha = *bestValue;

		if (*bestValue >= *beta) return 1;

	}
	return 0;
}

/* Function: recursiveSearch()
 *
 *
 */

int recursiveSearch(int *alpha, int *beta,int *bestValue, move *bestMove,int depthWithExtensions,int ply,move hashMove,int  searchType)

{

	int n, value, count; 
	move *m; 
	m = searchMoves[ply]; 


#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{ 
				switch (searchType)
				{
					case WINNING_CAP: 
						fprintf(fi[ply],"<br>Winning Captures:<br><br>\n");
						break;
					case ALL_CAP:
						fprintf(fi[ply],"<br>All Captures:<br><br>\n");
						break;
					case MATE_TRIES:
						fprintf(fi[ply],"<br>Mate Tries:<br><br>\n");
						break;
				}
		}
			
#endif 

	switch (searchType)
	{
	case WINNING_CAP: 
		count = AIBoard.captureMoves(m);		
		count = AIBoard.orderCaptures(m)- m;    
		break;
	case ALL_CAP:
		count = AIBoard.captureMoves(m);		
		AIBoard.orderCaptures(m);
		break;
	case MATE_TRIES:
		count = AIBoard.mateTries(m); 
		break;
	default:
		count = 0; 
		assert (0);
		break; 
	}

	
	for(n = 0; n < count; n++) 
	
	{
		if ((m[n] == hashMove) && (!hashMove.isBad())) continue; 


assert (!AIBoard.badMove(m[n]));
		AIBoard.changeBoard(m[n]);
	
		#ifdef DEBUG_STATS
		stats_MakeUnmake[searchType]++;
		#endif

		// Recursive Search call 
	
		value = -search(-(*beta), -(*alpha), depthWithExtensions,  ply + 1, 0);

		AIBoard.unchangeBoard();	
	
	
#ifdef GAMETREE
		
		char buf[MAX_STRING], buf2[MAX_STRING], buf3[MAX_STRING];  
		
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{
		DBMoveToRawAlgebraicMove(m[n], buf);
		strcpy(buf2, filename[ply]); strcat(buf2, buf);
		sprintf (buf3,"<a href=\"%s-%d.html\"><b>%s</b></a>  Return Value: %d<br>\n",buf2,currentDepth, buf,value); 		
		
		fprintf (fi[ply], buf3); 
		}
#endif			
	

		
		if (value > *bestValue) 
		{	
			*bestValue = value;  
			*bestMove = m[n];
			savePrincipalVar(*bestMove,ply + 1);

		}
	
		if (*bestValue > *alpha) *alpha = *bestValue; 
	
		if (*bestValue >= *beta) return 1; 	
	
	}
	return 0; 
}

/* Function: recursiveHash()
 *
 * @georg: Better describe what this is doing with the drawing above, also clarify what "isBad" does in Relation to the hash move.
 */

inline int recursiveHash(int *alpha,int *beta, int *bestValue , move *bestMove,int depthWithExtensions,int ply,move  hashMove)

{
	int value; 


	if (!hashMove.isBad())
	
	{

		#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{ 
			fprintf(fi[ply],"<br>Hash Move:<br><br>\n");
		}
		#endif 


assert (!AIBoard.badMove(hashMove));
		AIBoard.changeBoard(hashMove);


		#ifdef DEBUG_STATS
		stats_MakeUnmake[HASH_MOVE]++;
		#endif
	
		// Recursive Search call 
	
		value = -search(-(*beta), -(*alpha), depthWithExtensions,  ply + 1, 0);

		AIBoard.unchangeBoard();	
	
	
#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{
		char buf[MAX_STRING], buf2[MAX_STRING], buf3[MAX_STRING];  

		DBMoveToRawAlgebraicMove(hashMove, buf);
		strcpy(buf2, filename[ply]); strcat(buf2, buf);
		sprintf (buf3,"<a href=\"%s-%d.html\">%s</a>  Return Value: %d<br>\n",buf2,currentDepth, buf,value); 		
		fprintf (fi[ply], buf3); 
		}
#endif			
	

		
		if (value > *bestValue) 
		{	
	
			*bestValue = value;  
			*bestMove = hashMove;
			savePrincipalVar(*bestMove, ply + 1);

		}
	
		if (*bestValue > *alpha) *alpha = *bestValue; 
	
		
	
	}

	if (*bestValue >= *beta) return 1; 

return 0; 
}


/* Function: search
 * Input:    alpha, beta,how far to search+ search extensions are left 
 *			 and how far we've searched currently            
 * Output:   None
 * Purpose:  Uses an recursive alpha-beta search to assign a value to the 
 *           position.
 */
 
int search(int alpha, int beta, int depth, int ply, int wasNullMove)
{
  int orgBeta, orgAlpha;	//  set to alpha and beta since those will be adjusted 
  int extensions = -ONE_PLY; 
  int bestValue = -INFINITY; 
  int HashValue;
  int NullValue;            //  NullValue of the position 
  int Currenteval;          //  Current Static evaluation      



  move bestMove, hashMove;
  
  transpositionEntry *te;

  
assert (alpha <= INFINITY);
assert (beta <= INFINITY);
assert (alpha >= -INFINITY);
assert (beta >= -INFINITY);
	
assert (ply <= DEPTH_LIMIT);
  
  
#ifdef GAMETREE

  
  char buf [MAX_STRING] ; 
  char buf2 [MAX_STRING] ; 
  char buf3 [MAX_STRING] ;
  int n;
  

  if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH -1) )
  {
  tree_positionsSaved++;
  sprintf(filename[ply],"=");
  sprintf(buf2,"Moves:");
  for (n=0; n<ply; n++) 
  {
	  DBMoveToRawAlgebraicMove(AIBoard.getMoveHistory(n), buf); 
	  strcat(filename[ply],buf);
	  strcat(buf2," "); strcat (buf2, buf); 
  }
  sprintf(buf3,"treemoves/%s-%d.html",filename[ply],currentDepth);
  fi[ply] = fopen(buf3,"wt");

  fprintf(fi[ply],"<link rel=\"stylesheet\" href=\"../style/style.css\" type=\"text/css\">\n");
  fprintf(fi[ply],"<html><table cellpadding=10><tr><td valign=top>\n"); 
  
  printHtmlBoard(fi[ply]); 
  fprintf(fi[ply],"</td><td valign=top>\n"); 
  
  fprintf(fi[ply],"%s <br><br><hr><br>", buf2);
  fprintf(fi[ply],"<img src=\"../images/alpha.gif\" width=\"11\" height=\"9\" border=\"0\" alt=\"alpha\" hspace=\"10\"> : <b>%d</b> <img src=\"../images/beta.gif\" width=\"8\" height=\"18\" border=\"0\" alt=\"beta\" hspace=\"10\"> : <b>%d</b> <br><br>\n",alpha, beta);
  }

#endif  

  pollForInput();
  
  stats_positionsSearched++;

  if (FIXED_NODES && (stats_positionsSearched + stats_quiescensePositionsSearched > FIXED_NODES))
  {
	  stopThought();
  }

assert ( stats_positionsSearched < 1000000000 );  // hoping for the day when 
												  // this one fails :)

  pv.depth[ply] = 0;

  if(stopThinking) { 
					 #ifdef GAMETREE
					 if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
					 {
						fprintf(fi[ply],"<br>Return: out of time<br></td></tr></table></html>\n");
						fclose(fi[ply]); 
					 }
					 #endif   
					// @georg testing needed
					// the return value here should be irrelevant, because in the search on root level "stopThinking" is checked too
					// and the tree currently searched is not used.
					return -INFINITY; }

  if(AIBoard.isInCheck(AIBoard.getColorOffMove())) { 
													#ifdef GAMETREE
													if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
														{	
														fprintf(fi[ply],"<br>Return: Illegal Position: other side in check<br></td></tr></table></html>\n");
														fclose(fi[ply]); 
														}
													#endif 

													return INFINITY; }

  

  bestMove.makeBad();

  orgAlpha = alpha;
  orgBeta = beta;
  

  if ((te = AIBoard.lookup()) != NULL) 
  
  { 
					// We've searched this position before
					// and can remember some information
					// about it.
  
	  stats_transpositionHits++;

    /* If we searched to the same depth before as we're aiming at now, then
       we can use the value we got last time.

       If the search returned an exact value (higher than alpha and lower
       then beta) then we can return that value here too.

       If the search failed high (The position was at least as good as beta)
       then if the current beta isn't higher then the value returned last time
       we return the value, otherwise we set best and alpha to the value
       returned last time since we know we can get at least that. 
       
       If the search failed low (All of it's decendants were at least as bad as
       alpha)then if the current alpha isn't lower than the value returned last
       time we return the value, otherwise we set beta to the value returned
       since we know we can't get better then that. */

    if(te->depth >= depth) 
	{
		HashValue = te->value;

		if (HashValue >= MATE)
			HashValue -= ply;
		else if (HashValue <= -MATE)
			HashValue += ply;

		if(te->type == EXACT) 
		{ 
								#ifdef GAMETREE
								if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
									{
									fprintf(fi[ply],"<br><br>Return: exact hash value: %d<br></td></tr></table></html>\n", HashValue);
									fclose(fi[ply]); }
								#endif 

								return HashValue;
		}
      else if(te->type == FAIL_HIGH) 
	  {
			if(beta <= HashValue) {
								  #ifdef GAMETREE
									if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
									{	
									fprintf(fi[ply],"<br><br>Return: fail high hash value<br></td></tr></table></html>\n");
									fclose(fi[ply]); }
								  #endif

								  return HashValue; }
			if(HashValue > alpha)		
			{
				bestValue = alpha = HashValue;
			}
									}
			else if(te->type == FAIL_LOW) 
			{
				if(HashValue <= alpha) 
				{
								   #ifdef GAMETREE
										if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
										{
										fprintf(fi[ply],"<br><br>Return: fail low hash value<br></td></tr></table></html>\n");
										fclose(fi[ply]);  }
								   #endif 

					return HashValue;
				}
				if(beta > HashValue)		
				{
					beta = HashValue;
				}	
			}
		}


  /* If we had remembered this position from before then try to 
     use the best move we found last time and look at it first. */
  
    hashMove = te->hashMove;
	

  } else hashMove.makeBad();


 
#ifdef GAMETREE
	if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
	{	
	fprintf(fi[ply]," depth to horizon: <font size=+2>%d</font> (%d ply) at depth %d<br><br>\n",depth + extensions, (depth + extensions) / ONE_PLY, ply); 	
	}
#endif 
  

  /* Capture extensions. 
   * Conditions: 
   * Last move was a capture of the piece that just moved and it was the only way to capture it
   */
  
	if (!wasNullMove)
	{
		if (AIBoard.captureExtensionCondition())

		{
		extensions += CAPTURE_EXTENSION;
		/*
		// Tried the following 2 versions, both worse (= unchanged)

		if (depth >= (FractionalDeep[currentDepth] ) - (ONE_PLY *2))
		{
			extensions += 4;
		}
	
		if (ply < currentDepth / 2)
		    extensions += 2;
		*/

#ifdef DEBUG_STATS
		stats_capext += CAPTURE_EXTENSION;
#endif

#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1))
		{
			fprintf(fi[ply], "Capture extension: depth+ %d<br>\n", CAPTURE_EXTENSION);
		}
#endif	 
		}

	}

  /* In check.  */

  if(AIBoard.isInCheck(AIBoard.getColorOnMove())) 
  {   
	AIBoard.setCheckHistory(1);

	if (depth > (5 * ONE_PLY))
	{
		extensions += 3;
	}
	else if (depth > (3 * ONE_PLY))
	{
		extensions += 2;
	}
	else
	{
		extensions += 1;
	}


		#ifdef DEBUG_STATS
		stats_checkext+= CHECK_EXTENSION; 
		#endif
	 	
		#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1))  { fprintf(fi[ply],"Check extension: depth + %d<br><br><hr><br>\n", CHECK_EXTENSION); }
		#endif	 
	

	recursiveCheckEvasion(&alpha, &beta,&bestValue, &bestMove, depth+extensions, ply, hashMove); 				 
  } 
  
  /* Not in Check */

  else  
  {	  	 
	 AIBoard.setCheckHistory(0);
	 AIBoard.setBestCapture(); // this is probably not needed, since before this point orderCaptures has already been called. But just to be sure.
	 
	 if ((depth < ONE_PLY) || (ply>MAX_SEARCH_DEPTH)) 

	 {
	
#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{ 
			fprintf (fi[ply]," <br><br> Return: Quiesce = %d <br></td></tr></table></html>\n", bestValue); 
			fclose(fi[ply]); 
		}
#endif 
		
		
		
		bestValue = quiesce(alpha, beta, ply); 

		
		AIBoard.store(ONE_PLY-1, bestMove, bestValue, orgAlpha, orgBeta, ply);

		return bestValue; 
		
	    }
	
	
	if (depth > CC_DEPTH * ONE_PLY)
		
	{
	
	 /* NullMove : passing should be worse than any other move. */
	
	if ((!wasNullMove) && (NULL_REDUCTION))
		
	  //  no Null moves for black + white after another.
	  //  no Null move try if depth allows standpat
	{
	 
		//TODO: do we really need to try nullmove if eval() is already worse than i.e. alpha?

		AIBoard.makeNullMove();

		NullValue =  -search(-beta, -beta+1, depth - ((NULL_REDUCTION +1) * ONE_PLY), ply + 1, 1);	
		
		/*
		
		if (NullValue < beta)
		{
			NullValue = -search(-beta, -beta + 1, depth - ((NULL_REDUCTION + 1) * ONE_PLY), ply + 1, 1);
		}
		*/
		AIBoard.unmakeNullMove(); 

		#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{fprintf (fi[ply]," Null Move Value : %d <br><br>\n", NullValue); }
		#endif
		
		#ifdef DEBUG_STATS
		stats_NullTries[depth/ONE_PLY]++;   	
		#endif
	

		if (NullValue >= beta) //	fail high even without making a move, this must
						   //	be a very good position
		{			                     
       
			#ifdef GAMETREE
			if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
			{ fprintf(fi[ply],"<br><br>Return: Null Move Cut<br></td></td></tr></table></html>\n");
			fclose(fi[ply]); }
			#endif
	 
			#ifdef DEBUG_STATS
			stats_NullCuts[depth/ONE_PLY]++;		
			#endif
			
			AIBoard.store((max (depth, 0)), bestMove, NullValue, orgAlpha, orgBeta, ply);

			return NullValue;

		}	// End of successful NullMove try

	
	}	// End of NullMove try

#ifdef GAMETREE
	if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
	{ fprintf(fi[ply],"<br><hr><br>\n"); }
#endif


	if ( (! recursiveHash(&alpha, &beta,&bestValue, &bestMove, depth+extensions, ply, hashMove)) && 
		 (! recursiveSearch(&alpha, &beta,&bestValue, &bestMove, depth+extensions, ply, hashMove, ALL_CAP)) )
		 recursiveFullSearch(&alpha, &beta,&bestValue, &bestMove, depth+extensions, ply, hashMove); 
	


	} // End of > depth CC_DEPTH left

		
	else 
	{ 			
			/* Stand pat 
			 * Conditions : the last move was not a sack 
			 * ( piece we last move did capture one worth as much as itself or
			 * it was not taken )
			 */

			// This needs further testing
			// if ((wasNullMove) || (AIBoard.standpatCondition()))
			// {
				
				Currenteval = AIBoard.eval() ; 
				bestValue = Currenteval; 
				if (bestValue > alpha) alpha = bestValue; 

#ifdef GAMETREE
				if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
				{ 
					fprintf (fi[ply],"Standpat allowed<a href=\"start.html#standpat\">(*)</a> : bestValue  = %d <br><br><hr><br>\n", bestValue); 
				}
#endif 
				
				if (Currenteval >= beta) 
				{
				
					#ifdef GAMETREE
					if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
					{ fprintf(fi[ply],"<br><br><hr><br>Return:  bestValue > beta<br></td></td></tr></table></html>\n");
					fclose(fi[ply]); }
					#endif

					AIBoard.store((max (depth, 0)), bestMove, bestValue, orgAlpha, orgBeta, ply);
										

					return bestValue; 
				}
			/*
			}

			else 
			{
			// if we are not allowed to standpat (like in quiesce) or search all moves 
			// (like in full width) it can happen that we find no best move even when not
			// mated. That position will be terrible, but still don't think its mate !
			
			bestValue = -EXTREME_EVAL; 


#ifdef GAMETREE
		if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
		{ 
			fprintf (fi[ply],"Standpat not allowed<a href=\"start.html#standpat\">(*)</a> : bestValue  = %d <br><br><hr><br>\n", bestValue); 
		}
#endif 
			
			
			}
			*/

		
			if ((!recursiveHash(&alpha, &beta, &bestValue, &bestMove, depth + extensions, ply, hashMove)) &&
				(!recursiveSearch(&alpha, &beta, &bestValue, &bestMove, depth + extensions, ply, hashMove, WINNING_CAP)))
				recursiveSearch(&alpha, &beta, &bestValue, &bestMove, depth + extensions, ply, hashMove, MATE_TRIES);
				
		} // End of <= depth * CC_DEPTH left


	} // End of in check/ not in check branch 	

  if (bestValue == -INFINITY) 
	{  
	 
	 // None of the moves were legal.  See if it's checkmate or the person has to sit 
    

	 if (currentRules == CRAZYHOUSE)  
	 {
		 bestValue = -MATE_IN_ONE + ply / 2 + 1;
     } 
	 else // current rules = bughouse
	 {
		 if (AIBoard.cantBlock())
		 {
			 bestValue = -MATE_IN_ONE + AIBoard.bughouseMateEval();
		 }
		 else
		 {
			 bestValue = -ALMOST_MATE;
		 }
	 }
  }
 


  /* Now that the search is over, save information to transposition tables */

   
  if (!stopThinking)
  {
	
	AIBoard.store((max (depth, 0)), bestMove, bestValue, orgAlpha, orgBeta, ply);	

	if (! bestMove.isBad())
	{
	
		if  (AIBoard.pieceOnSquare(bestMove.to()) == NONE) 
		{
			updateHistory(bestMove, (max (depth, 0))); 
		}
	}

  } 
#ifdef GAMETREE
  if ((tree_positionsSaved < GAMETREE) && (currentDepth == FIXED_DEPTH - 1)) 
  {				
	fprintf(fi[ply],"<br><br><hr><br>Return: end of search<br></td></tr></table></html>\n");
	fclose(fi[ply]); 
  }
#endif
  
  return bestValue;
}


/* Function: findMove
 * Input:    A pointer to move
 * Output:   None
 * Purpose:  Fills rightMove with the move that Sunsetter thinks is best
 */

void findMove(move *rightMove)
{

  int bestValue; 
  
  char buf[MAX_STRING];
  char buf2[MAX_STRING];

  pondering = 0;


  
  stats_overallsearches += stats_positionsSearched; stats_overallqsearches += stats_quiescensePositionsSearched;
  stats_hashFillingUp = stats_transpositionHits = stats_quiescensePositionsSearched = stats_positionsSearched = 0; 
  
#ifdef DEBUG_STATS
  stats_checkext = stats_forceext =  stats_capext = stats_RazorTries = stats_Razors =  0;

  int i; 
  
  for (i = 0; i<MOVEGEN_TYPES; i++) { stats_MakeUnmake [i] = 0; }
  for (i = 0; i<DEPTH_LIMIT; i++) { stats_NullTries [i] = stats_NullCuts [i] = 0; }

#endif

#ifdef DEBUG_HASH

  debug_allcoll = 0; 
  
#endif 

#ifdef GAMETREE

  tree_positionsSaved = 0; 

#endif

    if (gameBoard.getMoveNum() < 3) 
  {
		initialTime =  gameBoard.getTime(BLACK);
  }	

  do {
		gameBoard.copy(&AIBoard);
   		overideMove.makeBad();
	

	if ((currentRules == BUGHOUSE) && (!partsitting) && (!parttoldgo))
	
	
	// we search with "ghost pieces" in Bughouse
	
	{

	AIBoard.addPieceToHand(WHITE, ROOK, 1);
	AIBoard.addPieceToHand(BLACK, ROOK, 1);

    AIBoard.addPieceToHand(WHITE, KNIGHT, 1);
	AIBoard.addPieceToHand(BLACK, KNIGHT, 1);
	
	AIBoard.addPieceToHand(WHITE, PAWN, 1);
	AIBoard.addPieceToHand(BLACK, PAWN, 1);
	
	}
	
	stopThinking = reSearch = forceMove = 0;
    parttoldgo = 0; // he tells us to go always for 1 move only
	


    searchRoot(MAX_SEARCH_DEPTH, rightMove, &bestValue);
	
	/* Only for Bughouse */
	if (currentRules == BUGHOUSE) 
	
	{

	if ((!gameBoard.isLegal(*rightMove)) || ( (bestValue > MATE) && (bestValue < (MATE_IN_ONE - 90)) ))
	
	{ 		
		if (!toldpartisit) 
					{
					DBMoveToRawAlgebraicMove(*rightMove, buf2); 
					if (bestValue >MATE) {strcat(buf2," with mate"); }
					sprintf(buf,"tellics ptell sitting (%s). Tell me \"sitting\" if no stuff comes for me or \"go\" to make one move .\n", buf2); 
					output(buf); 
					toldpartisit =1; 
					}
					waitForInput();
					reSearch = 1; 
					gameBoard.setLastMoveNow();

					
	}				


	// the move is not legal => we tried to drop a ghost-piece => sit for it
	
	if ((bestValue > -MATE) && (toldparttosit)) 
	
		{ 
		output ("tellics ptell go (I am ok)\n"); 
		toldparttosit = 0; 
		}
	
	// we told our part to sit, its the next move and now we are not mated anymore => he can go again 
		
	if (bestValue <= -MATE_IN_ONE+90) 
	
	{ 
		if (!sitting) 
					{ 
					output ("tellics ptell sitting (I am mated)\ntellics ptell go\n"); 
					sitting =1; 
					while ((sitting) && (!reSearch)) {pollForInput();} 
					} 
	}
	
	// we are mated in 1 => sit 

    else 
	
	{ 
		
		if ((bestValue <= -MATE) && (!partsitting)) 
					{ 
					reSearch =1; 
					gameBoard.setLastMoveNow();
					output ("tellics ptell sit (I am in trouble)\n"); 
					partsitting =1; toldparttosit =1; 
					}  
		}
	
	// we are mated soon, tell our partner to sit and then research, maybe it helps. Else just play.
    
	} /* End of Only for Bughouse */ 

  } while(reSearch && !gameBoard.isLegal(overideMove) && !forceMove );
 
  
  /* Only for Bughouse */
  if (currentRules == BUGHOUSE)

  {

  if (partsitting && !toldparttosit) { psittinglong ++; if (psittinglong > 5) {output ("tellics ptell go (watch our time)\n"); psittinglong = 0; }}
  else { psittinglong = 0; }

  // our part is sitting on his own. If he does that for more than 5 moves I suggest to go 
	  
  if (toldpartisit) {unsit ();}

  // we were waiting for a piece. We got that now. Or our partner told us to go anyway.

  if (toldparttosit) {partsitting = 0;}

  // we told part to sit. Don't expect him to sit more than 1 move     
  
  } /* End of only for Bughouse */

  checkInput();

  makeHistoryOld(); 

  

  return;
}

/* Function: stopThought
 * Input:    None.
 * Output:   None.
 * Purpose:  If Sunsetter is thinking it makes it stop and make a move.  It
 *           doesn't do anything if Sunsetter is sitting while thinking about
 *           it's move to play.
 */

void stopThought()
{
 if(!sitting || pondering ) stopThinking = 1;
}

/* Function: startSearchOver
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to make Sunsetter start the search again from scratch.  This
 *           should be used when a value that affects the search is changed,
 *           like the piece values or new piece in the hand.
 */

void startSearchOver()
{
  stopThinking = 1;
  reSearch = 1;

  /* Somethings changed the position by a lot,
     the old hash values are no good */

  zapHashValues();
 

}

/* Function: sit
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to make Sunsetter sit
 */

void sit()
{
  sitting = 1;
  output("tellics ptell sitting\n");
}

/* Function: unsit
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to make Sunsetter stop sitting
 */

void unsit()
{
  sitting = 0;
  toldpartisit = 0; 
}

/* Function: doMove
 * Input:    A move.
 * Output:   None.
 * Purpose:  Used to make Sunsetter play a move instead of what the search
 *           finds.
 */

void doMove(move m)
{
  overideMove = m;
  forceDeepBugToMove();
  unsit();
  stopThought();
}

/* Function: forceDeepBugMove
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to make Sunsetter play a move even if it thinks it gets it
 *           mated
 */

void forceDeepBugToMove()
{
  forceMove = 1;
}



/* Function: resetAI
 * Input:    None.
 * Output:   None.
 * Purpose:  Used to make Sunsetter stop all trains of thought, good for things
 *           like ending and begining a game
 */

void resetAI()
{
  sitting = 0;
  toldpartisit = 0;
  stopThought();
  forceDeepBugToMove();
  reSearch = 0;
}
