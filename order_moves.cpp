/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: orderMoves.cc                                                      *
 *  Purpose: Has orderCaptures() which orders the captures based on material *
 *           gain and the functions related to history move ordering.        *
 *           Note history in Sunsetter isnt what mst people mean by history  *
 *                                                                           *
 *************************************************************************** */

#include <stdio.h>

#include "board.h"
#include "brain.h"
#include "interface.h"
#include "notation.h"




qword valueToSquares [SQUARES][PIECES][COLORS][2]; 

bitboard generateToFirst[PIECES][COLORS][2]; 


/*
 * Function: updateHistory
 * Input:    A move, and the depth to horizon at current node
 * Output:   none
 * Purpose:  used for the move generation in AIMoves to generate moves to fields that used to be good first. 
 *           
 */

void updateHistory(move m, int depth )

{
	piece p2;
	square sq; 


	color c = AIBoard.getColorOnMove(); 
	piece p = m.moved();
	square sqto = m.to(); 	
	int isDrop = 0; 

	if (m.from() == IN_HAND) { isDrop = 1; }



	valueToSquares [sqto][p][c][isDrop]+=depth; 


	assert (valueToSquares[sqto][p][c][isDrop] < 10000000); 

	// if this square is already one of the "good" ones, we can return now. 
	
	if (generateToFirst[p][c][isDrop].squareIsSet(sqto)) return; 

	
	for (p2 = FIRST_PIECE; p2 <= QUEEN ; p2 = (piece) (p2 + 1))

	{

		bitboard bb = generateToFirst[p2][c][0]; 

		while (bb.hasBits())
		{					
			
			sq = firstSquare(bb.data);
			bb.unsetSquare(sq);  

			if (valueToSquares [sqto][p][c][isDrop] > valueToSquares[sq][p2][c][0])
			{				
				
				assert (generateToFirst[p2][c][0].squareIsSet(sq)) ;
				assert (!generateToFirst[p][c][isDrop].squareIsSet(sqto));
								
				generateToFirst[p2][c][0].unsetSquare(sq);
				generateToFirst[p][c][isDrop].setSquare(sqto); 				 

				goto breakloop; 

			}
		}

		bb = generateToFirst[p2][c][1]; 

		while (bb.hasBits())
		{					
			
			sq = firstSquare(bb.data);
			bb.unsetSquare(sq); 


			if (valueToSquares [sqto][p][c][isDrop] > valueToSquares[sq][p2][c][1])
			{
				
				assert (generateToFirst[p2][c][1].squareIsSet(sq)) ;
				assert (!generateToFirst[p][c][isDrop].squareIsSet(sqto));
				
				generateToFirst[p2][c][1].unsetSquare(sq);
				generateToFirst[p][c][isDrop].setSquare(sqto); 

				goto breakloop; 

			}
		}
	}

breakloop: 



return; 
}


/*
 * Function: initializeHistory
 * Input:    none
 * Output:   none
 * Purpose:  used to set the history values to zero at start of the game
 *           
 */

void initializeHistory()

{

	square sq; 
	piece p; 
	color c; 

	for (p = FIRST_PIECE; p <= LAST_PIECE; p = (piece) (p + 1))
	{
		for (c = FIRST_COLOR; c <= LAST_COLOR; c = (color) (c + 1))
		{
			generateToFirst[p][c][0] = qword(0); 
			generateToFirst[p][c][1] = qword(0); 


			generateToFirst[p][c][0].setSquare(E4); 
			generateToFirst[p][c][0].setSquare(D4); 
			generateToFirst[p][c][0].setSquare(E5); 
			generateToFirst[p][c][0].setSquare(D5); 

			generateToFirst[p][c][0].setSquare(E3); 
			generateToFirst[p][c][0].setSquare(D3); 
			generateToFirst[p][c][0].setSquare(E6); 
			generateToFirst[p][c][0].setSquare(D6); 


			for (sq = 0; sq < SQUARES; sq++)
			{
				valueToSquares[sq][p][c][0] = qword(0); 
				valueToSquares[sq][p][c][1] = qword(0); 				

			}

		}
	}


return; 
}

/*
 * Function: makeHistoryOld
 * Input:    none
 * Output:   none
 * Purpose:  make sure the history values get decreased between moves
 *           
 */

void makeHistoryOld()

{

	square sq; 
	piece p; 
	color c; 

	for (p = FIRST_PIECE; p <= LAST_PIECE; p = (piece) (p + 1))
	{
		for (c = FIRST_COLOR; c <= LAST_COLOR; c = (color) (c + 1))
		{
			for (sq = 0; sq < SQUARES; sq++)
			{
				valueToSquares[sq][p][c][0] /= 5; 
				valueToSquares[sq][p][c][1] /= 5; 
			}						

		}
	}


return; 
}


/*
 * Function: orderCaptures
 * Input:    An array of moves
 * Output:   The first move that isn't a material gaining capture
 * Purpose:  Used by aiMoves() to put the captures in the decending order so
 *           that the best ones are looked at first, used in quiesce to order moves.
 */

move *boardStruct::orderCaptures(move *m)
   {
   move tmpMove;
   int values[MAX_MOVES], done, count, tmpVal, i;
   values[0] = 0; // needed for the case of 0 capture moves

   for (count = 0; !m[count].isBad(); count++)
      {
       if ( position[m[count].to()] != NONE )
	   {
		   values[count] =  captureGain(onMove, m[count]);
	   }
	   else 
	   {
		  // either a promotion, or an e.p. capture 
		   // this estimation will do 
		   // TODO FIX this
		   values[count] = pValue[KNIGHT];
	   }
      }


   // @georg : make sure this isnt triggered when there are 2 moves, which are then not ordered ... 
   if (count < 2)
   {

	   // 20 is not a magic number :) 
	   // it just makes sure that capturing a minor with a minor 
	   // (R should be called minor in crazyhouse) is not considered 
	   // a winning capture. 

     bestCaptureGain[moveNum] = max (0, values[0]);
	   
	 if (values[0] < +20) return m;
	 else return m + count;
   }

   /* Sort the captures */

   do
      {
      done = 1;
      for (i = 0; i < count - 1; i++)
         if ((values[i]) < values[i + 1])
            {
            done = 0;
            tmpVal = values[i];
            values[i] = values[i + 1];
            values[i + 1] = tmpVal;
            tmpMove = m[i];
			m[i] = m[i + 1];
            m[i + 1] = tmpMove;

            }
      } while(!done);

   bestCaptureGain[moveNum] = max (0, values[0]);

   if (values[0] < +20)
      return m;
   while (values[count - 1] < +20)
      count--;
   return m + count;
   }

