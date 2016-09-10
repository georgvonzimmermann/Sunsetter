/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *                                                                           *
 *  Name: notation.cc                                                        *
 *  Purpose: Has the functions for converting the algebraic notation that    *
 * humans use to and from the one that Sunsetter uses.                        *
 *                                                                           *
 *************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bughouse.h"
#include "notation.h"
#include "interface.h"
#include "board.h"
#include "brain.h"
#include "variables.h"

/*
 * Function: colorToChar
 * Input:    A color
 * Output:   The character it corresponds to.
 * Purpose:  Used to convert what Sunsetter uses to text
 */

char colorToChar(color c)
   {
   if (c == WHITE)
      return 'W';
   if (c == BLACK)
      return 'B';
   output("Bad color in colorToChar()\n");
   return ' ';
   }

/*
 * Function: pieceToChar
 * Input:    A piece
 * Output:   The character it corresponds to.
 * Purpose:  Used to convert what Sunsetter uses to text
 */

char pieceToChar(piece p)
   {
   switch(p)
      {
      case KNIGHT:
         return 'N';
      case BISHOP:
         return 'B';
      case ROOK:
         return 'R';
      case QUEEN:
         return 'Q';
      case KING:
         return 'K';
      case PAWN:
         return 'P';
      default:
         output("Bad piece in pieceToChar()\n");
         return ' ';
      }   
   }


/*
 * Function: charToPiece
 * Input:    A char
 * Output:   The piece it corresponds to.
 * Purpose:  Used to convert text to what Sunsetter uses
 */

piece charToPiece(char ch)
   {
   switch(ch)
      {
      case 'N':
      case 'n':
         return KNIGHT;
      case 'B':
      case 'b':
         return BISHOP;
      case 'R':
      case 'r':
         return ROOK;
      case 'Q':
      case 'q':
         return QUEEN;
      case 'P':
      case 'p':
         return PAWN;
      case 'k':
      case 'K':
         return KING;
      default:
         return NONE;
      }
   }


/*
 * Function: DBMoveToRawAlgebraicMove
 * Input:    A move in Sunsetter's Notation and a string to write it to
 * Output:   None.
 * Purpose:  Used to convert the notation.  It fills the str with the converted
 *          move in raw (g1f3 instead of Nf3) notation.  If an error occurs 
 *          then it returns an empty string.
 */

void DBMoveToRawAlgebraicMove(move m, char *str)
   {

    str[0] = 0;

   /* Check to see if it is a drop move */

   if (m.from() == IN_HAND)
      {
      assert (m.to() < SQUARES);

#pragma warning ( disable : 4244 )
      
	  str[0] = pieceToChar(m.moved());
      str[1] = '@';
      str[2] = 'a' + file(m.to());
      str[3] = '1' + rank(m.to());
      str[4] = '\0';
      return;
      }

   /* Check to make sure the move is on the board */
   if (m.to() < SQUARES && m.from() < SQUARES)
      {
      str[0] = 'a' + file(m.from());
      str[1] = '1' + rank(m.from());
      str[2] = 'a' + file(m.to());
      str[3] = '1' + rank(m.to());
      if (m.promotion() != NONE)
         {
         str[5] = 0;
         switch (m.promotion())
            {
            case KNIGHT:
               str[4] = 'n';
               break;
            case BISHOP:
               str[4] = 'b';
               break;
            case ROOK:
               str[4] = 'r';
               break;
            case QUEEN:
               str[4] = 'q';
               break;
            default:
               assert (0); 
            }      
         }
      else
         str[4] = '\0';
      return;
      }
   else
      {
	   assert (0); 
      
   }

#pragma warning ( default : 4244 )

   }
  

/*
 * Function: rawAlgebraicMoveToDBMove
 * Input:    An move in algebraic notation and a Move pointer convert it to.
 * Output:   None.
 * Purpose:  Used to convert from raw (g1f3 instead of Nf3) algrebraic moves to
 *           Sunsetter's internal moves.  It converts the string it gets to a 
 *           Move.
 */
 
move boardStruct::rawAlgebraicMoveToDBMove(const char *notation)
   {
   char ch;
   square from, to;
   piece promotion;
   move m;

   /* See if the move is castles */
   if (!strncmp(notation, "0-0-0", 5) || !strncmp(notation, "o-o-o", 5) ||
      !strncmp(notation, "O-O-O", 5) ||
      (position[E1] == KING && !strncasecmp(notation, "e1c1", 4)) ||
      (position[E8] == KING && !strncasecmp(notation, "e8c8", 4)))
      {
      if (onMove == WHITE) 
         return move(E1, C1, KING);
      else 
         return move(E8, C8, KING);
      }

   if (!strncmp(notation, "0-0", 3) || !strncmp(notation, "o-o", 3) ||
         !strncmp(notation, "O-O", 3) ||
         (position[E1] == KING && !strncasecmp(notation, "e1g1", 4)) ||
         (position[E8] == KING && !strncasecmp(notation, "e8g8", 4)))
      {
      if (onMove == WHITE) 
         return move(E1, G1, KING);
      else 
         return move(E8, G8, KING);
      }

   /* Make sure the to square is on the board */

   if ((notation[2] < 'a' || notation[2] > 'h') || 
      (notation[3] < '1' || notation[3] > '8'))
      { 
      m.makeBad();
      return m;
      }

   /* See if it is a dropping move */
   if (notation[1] == '@')
      {
      if (charToPiece(notation[0]) != NONE)
         {
         return move (IN_HAND, ((notation[2] - 'a') * ONE_FILE +
                (notation[3] - '1') * ONE_RANK), 
                 charToPiece(notation[0]));
         }
      else
         {
         m.makeBad();
         return m;
         }
      }
      
   /* Make sure the from square is on the board */
  
   if ((notation[0] < 'a' || notation[0] > 'h') || 
     (notation[1] < '1' || notation[1] > '8'))
      {
      m.makeBad();
      return m;
      }

   if (isalpha(notation[4]) || notation[4] == '=')
      {
      if (notation[4] == '=')
         ch = notation[5];
      else
         ch = notation[4];
      promotion = charToPiece(ch);
      if (promotion == NONE)
         {
         m.makeBad();
         return m;
         }
      }
   else
      promotion = NONE;
  
   from = (notation[0] - 'a') * ONE_FILE + (notation[1] - '1') * ONE_RANK;
   to = (notation[2] - 'a') * ONE_FILE + (notation[3] - '1') * ONE_RANK;
   return move(from, to, position[from], promotion);
   }


/*
 * Function: algebraicMoveToDBMove
 * Input:    An move in algebraic notation and a Move pointer convert it to.
 * Output:   None.
 * Purpose:  Used to convert from regular (Nf3 instead of g1f3) algrebraic
 *           moves to Sunsetter's internal moves.  It converts the string it
 *           gets to a move.
 */

move boardStruct::algebraicMoveToDBMove(const char *notation)
   {
   const char *ptr;
   move legalMoves[MAX_MOVES], m;
   piece p, promotion;
   int toFile, toRank, fromFile, fromRank, moveCount, toSquare, i, matchedOne;

   /* First try to see if it's in raw algebraic notation (g1f3) */

   m = rawAlgebraicMoveToDBMove(notation);
   if (!m.isBad())
      return m;

   moveCount = moves(legalMoves);
  
   /* "Moves" consisting of only 1 character can't be valid: */
   
   if (strlen(notation)<2) {
	   m.makeBad();
	   return m;
   }

   /* The format is [B,K,N,P,R,Q][a-h][1-8][X]<a-h>[1-8][extra]
   this function parses the string from the right to the left.
   The first thing it does is skip over the extra at the end (the +s and #s)
   */

   ptr = notation + strlen(notation) - 1; // start at the end
   while (*ptr == '+' || *ptr == '#') if (--ptr == notation)
   {
	   m.makeBad();
	   return m;
   }

   /* See if the move is a promotion */

   if (*(ptr - 1) == '=')
      {
      promotion = charToPiece(*ptr);
      if (promotion == NONE)
         {
         m.makeBad();
         return m;
         }
      ptr -= 2;
      }
   else
      promotion = NONE;

   /* Now we're on the to square, parse the rank (if there is one)
      then the file */

   if (isdigit(*ptr))
      {
      if (*ptr == '0' || *ptr == '9')
         {
         m.makeBad();
         return m;
         }
      toRank = *(ptr--) - '1';
      p = NONE;
      }
   else
      {
      toRank = -1;      /* -1 is the magic number that tells us the rank
                         wasn't specified */
      p = PAWN;
      }
   if (*ptr < 'a' || *ptr > 'h')
      {
      m.makeBad();
      return m;
      }
   toFile = tolower(*ptr) - 'a';
  

  /* If this is the begining of the string the move was a pawn move
     without a 'P' (for instance e4) figure out which pawn can move there
     without a capture and return that move */

   if (ptr == notation)
      {
      if (toRank == -1)
         {
         m.makeBad();
         return m;
         }
      toSquare = toFile * ONE_FILE + toRank * ONE_RANK;
      if (position[toSquare] != NONE)
         {
         m.makeBad();
         return m;
         }
      for (i = 0; i < moveCount; i++)
         {
         if (legalMoves[i].to() == toSquare &&
               legalMoves[i].moved() == PAWN &&
               legalMoves[i].promotion() == promotion &&
               legalMoves[i].from() != IN_HAND)
            {        /* rawAlgebraicToDBMove
                      handles drop moves */
            return legalMoves[i];
            }
         }
      m.makeBad();
      return m;
      }
   else
      ptr--;

   /* Maybe there's a X, if so skip over that, making sure that there is
      a piece to be captured */

   if (*ptr == 'X' || *ptr == 'x')
      {
      if (toRank == -1)
         {
         m.makeBad();
         return m;
         }
      toSquare = toFile * ONE_FILE +  toRank * ONE_RANK;
      if (position[toSquare] == NONE)
         {
         if (toSquare == enPassant)
            {
            p = PAWN;
            }
         else
            {
            m.makeBad();
            return m;
            }
         }
      if (--ptr < notation)
         {
         m.makeBad();
         return m;
         }
      }

   /* See if the notation told us what rank the piece came from */

   if (*ptr >= '1' && *ptr <= '8')
      {
      fromRank = *ptr - '1';
      if (ptr-- <= notation)
         {
         m.makeBad();
         return m;
         }
      }
   else
      {
      fromRank = -1;
      }

   /* See if the notation told us what file the piece came from */

   if (*ptr >= 'a' && *ptr <= 'h')
      {
      fromFile = tolower(*ptr) - 'a';
      ptr--;
      }
   else
      {
      fromFile = -1;
      }

   /* Now translate the letter of the piece to find the piece that moved */

   if (ptr == notation && p == NONE)
      p = charToPiece(*ptr);
   else if (ptr == notation - 1)
      {
      p = PAWN; /* The move specified the file the pawn moved from,
                instead of 'P' */
      }
   else
      {
      m.makeBad();
      return m;
      }
  
   if (p == NONE)
      {
      m.makeBad();
      return m;
      }

   /* We've parsed the move, try to match the move with only one legal
      move */

   matchedOne = 0;
   for (i = 0; i < moveCount; i++)
      {
      if (file(legalMoves[i].to()) == toFile &&
         (rank(legalMoves[i].to()) == toRank || toRank == -1) &&
         (rank(legalMoves[i].from()) == fromRank || fromRank == -1) &&
         (file(legalMoves[i].from()) == fromFile || fromFile == -1) &&
         legalMoves[i].promotion() == promotion &&
         legalMoves[i].from() != IN_HAND && /* rawAlgebraicMoveToDBMove handles
                    drop moves */
         legalMoves[i].moved() == p)
         {
         if (matchedOne == 1)
            {
            m.makeBad();
            return m;
            }
         m = legalMoves[i];
         matchedOne = 1;
         }
      }
   if (matchedOne)
      return m;
   m.makeBad();
   return m;
   }



/*
 * Function: printHtmlBoard
 * Input:    a file
 * Output:   None.
 * Purpose:  writes the part of a html file that shows the actual board position. 
 *           
 *           
 */

#ifdef GAMETREE

void printHtmlBoard (FILE *fi)

{
  int n, n3; 
  char buf[MAX_STRING];  
  
  fprintf(fi, "<TABLE><TR><TD>");  
  
  n3 = AIBoard.getPieceInHand(BLACK,PAWN); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/bpx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(BLACK,BISHOP); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/bbx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(BLACK,KNIGHT); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/bnx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(BLACK,ROOK); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/brx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(BLACK,QUEEN); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/bqx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  

  fprintf(fi,"<br><br>\n");
  
  for (n = 7; n != 64; n += 8)
  {		
	  if (n>63) {n -= 65; fprintf(fi,"<br>\n");}
	switch  (AIBoard.showColor(n))
	{
	case WHITE:
	   switch (AIBoard.showPiece(n))
            {			   
			 
	   
			case PAWN:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wpw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, PAWN));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wpb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, PAWN));	}
			   break;
			case KNIGHT:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wnw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, KNIGHT));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wnb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, KNIGHT));	}
               break;
            case BISHOP:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wbw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, BISHOP));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wbb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, BISHOP));	}
               break;
            case ROOK:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wrw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, ROOK));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wrb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, ROOK));	}
               break;
            case QUEEN:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wqw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, QUEEN));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wqb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, QUEEN));	}
			   break;
			case KING:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/wkw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, KING));	}
				else { fprintf(fi,"<img src=\"../images/diagr/wkb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(WHITE, n, KING));	}	
			   break;
			}
			break;
	   case BLACK:
	   switch (AIBoard.showPiece(n))
            {
			  
			case PAWN:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/bpw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(BLACK, n, PAWN));	}
				else { fprintf(fi,"<img src=\"../images/diagr/bpb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, PAWN));	}
			   break;
			case KNIGHT:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/bnw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, KNIGHT));	}
				else { fprintf(fi,"<img src=\"../images/diagr/bnb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, KNIGHT));	}
               break;
            case BISHOP:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/bbw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, BISHOP));	}
				else { fprintf(fi,"<img src=\"../images/diagr/bbb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, BISHOP));	}
               break;
            case ROOK:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/brw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, ROOK));	}
				else { fprintf(fi,"<img src=\"../images/diagr/brb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, ROOK));	}
               break;
            case QUEEN:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/bqw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, QUEEN));	}
				else { fprintf(fi,"<img src=\"../images/diagr/bqb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">",AIBoard.getPieceDevOnSquare(BLACK, n, QUEEN));	}
               break;
			case KING:
				if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/bkw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(BLACK, n, KING));	}
				else { fprintf(fi,"<img src=\"../images/diagr/bkb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"%d\">", AIBoard.getPieceDevOnSquare(BLACK, n, KING));	}
               break;
			}
			break;
	   default:
			{
			if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/efw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
			else { fprintf(fi,"<img src=\"../images/diagr/efb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
			}
			break;
	}
		
  } 
  
  fprintf(fi,"<br><br>\n");

  n3 = AIBoard.getPieceInHand(WHITE,PAWN); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/wpx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(WHITE,BISHOP); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/wbx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(WHITE,KNIGHT); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/wnx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(WHITE,ROOK); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/wrx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  n3 = AIBoard.getPieceInHand(WHITE,QUEEN); 
  while (n3 > 0) {fprintf(fi,"<img src=\"../images/diagr/wqx.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">"); n3--;}
  

  fprintf(fi, "</TD></TR></TABLE>\n<br>\n"); 	

  if (AIBoard.getColorOnMove() == WHITE) {fprintf (fi, "<img src=\"../images/wtm.gif\" width=\"31\" height=\"26\" border=\"0\" alt=\"\">");}
  else {fprintf (fi, "<img src=\"../images/btm.gif\" width=\"31\" height=\"26\" border=\"0\" alt=\"\">"); }

  fprintf(fi,"<br><br>\n");

  fprintf(fi,"Static Piece Tables Eval <a href=\"start.html#piecetable\">(*)</a> <hr><br><br>\n");

  

  for (n = 7; n != 64; n += 8)
  {		
    if (n>63) {n -= 65; fprintf(fi,"<br>\n");}

	n3= AIBoard.oneSquareBoardControl(n, 1); 

	if (n3 == 1) {fprintf(fi,"<img src=\"../images/green.gif\" width=\"36\" height=\"35\" border=\"0\">");}
	else if (n3 == 2) {fprintf(fi,"<img src=\"../images/red.gif\" width=\"36\" height=\"35\" border=\"0\">");}
	else if (n3 == 3) {fprintf(fi,"<img src=\"../images/yellow.gif\" width=\"36\" height=\"35\" border=\"0\">");}
	else 
	{
			if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/efw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
			else { fprintf(fi,"<img src=\"../images/diagr/efb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
	}
	
	
	
		
  } 
  
  fprintf(fi,"<br><br>\n");

  fprintf(fi,"Material dependent Control Eval <a href=\"start.html#nearking\">(*)</a> <hr><br><br>\n");
  
  

  for (n = 7; n != 64; n += 8)
  {		
    if (n>63) {n -= 65; fprintf(fi,"<br>\n");}

	n3= AIBoard.oneSquareBoardControl(n, 2); 

	if (n3 <= -2) {fprintf(fi,"<img src=\"../images/bl1.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");}
	else if (n3 == -1) {fprintf(fi,"<img src=\"../images/bl2.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");}
	else if (n3 == 0) {fprintf(fi,"<img src=\"../images/bl3.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");}
	else if (n3 == 1) {fprintf(fi,"<img src=\"../images/bl4.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");}
	else if ((n3 >= 2) && (n3 != 99)) {fprintf(fi,"<img src=\"../images/bl5.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");}
	
	
	else 
	{
			if ( (((n) / 8) + ((n) % 8) ) % 2) { fprintf(fi,"<img src=\"../images/diagr/efw.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
			else { fprintf(fi,"<img src=\"../images/diagr/efb.gif\" width=\"36\" height=\"35\" border=\"0\" alt=\"\">");	}
	}
	
	
	
		
  } 
  
  fprintf(fi,"<br><br>\n");

  fprintf(fi,"Board Control Eval <a href=\"start.html#boardcontrol\">(*)</a> <hr><br><br>\n");
  
  AIBoard.setColorOnMove(AIBoard.getColorOffMove());
  AIBoard.getEval(buf);
  fprintf(fi,"Eval <a href=\"start.html#eval\">(*)</a>: %s<br>\n", buf);
  AIBoard.setColorOnMove(AIBoard.getColorOffMove());




}

#endif /* GAMETREE */
