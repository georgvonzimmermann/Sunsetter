/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: partner.cc                                                         *
 *  Purpose: Has functions related to partner communication.                 *
 *                                                                           *
 *************************************************************************** */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "brain.h"
#include "interface.h"
#include "notation.h"



/* partner is 1 if we have a partner  */

int partner;

/* Function: givePartnerHelp
 * Input:    An argument
 * Output:   None
 * Purpose:  Used to give a ptell to Sunsetter's partner telling how to
 *           communicate with Sunsetter
 */

void givePartnerHelp(const char *str)
{
  char buf[MAX_STRING];

  if(!strcasecmp(str, "intro")) {
    output("tellics ptell Hi.  Thanks for being my partner.  Tell me \"help\" for the commands I understand and please visit http://www.d2d4.de/sunsetter.asp\n");
  
  } else if(!strcasecmp(str, "sitting") || !strcasecmp(str, "frozen")) {
    output("tellics ptell if you tell me that you are sitting I know that no stuff comes for me or my opponent. \n");
  } else if(!strcasecmp(str, "sit") || !strcasecmp(str, "go")) {
    output("tellics ptell sit means to sit, go means to stop sitting. \n");
  } else if(!strcasecmp(str, "flag")) {
    output("tellics ptell Use flag to tell me to flag my opponent (I usually have autoflag on, but sometimes I forget).\n");
  } else if(!strcasecmp(str, "abort")) {
    output("tellics ptell Use abort to make me offer or accept an abort request\n");
  } else if(str[0] == '\0') {
    output("tellics ptell I understand the following commands:  sitting/frozen, sit, go/move, flag and abort. Also If you tell me a move then I'll play it. Tell me help <the command> for help on that command.\n");

  } else {
    sprintf(buf, "tellics ptell Sorry I don't have help on %s\n", str);
    output(buf);
  }
}

/* Function: parsePartnerCommand
 * Input:    *arg1 , *arg2
 * Output:    0 if successfull and we should give back some acknowledgement,
 *            1 if sucsessfull and we don't need to say anything,
 *           -1 if not sucsessfull
 * Purpose:  Used to interpret a ptell from Sunsetter's partner.  It gets the
 *           second word of the message.
 */

int parsePartnerCommand(const char *arg1, const char *arg2)
{

  move toldMove;

  if(!strcasecmp(arg1, "help")) {
    givePartnerHelp(arg2);
    return 1;
  }

  if((currentRules == CRAZYHOUSE) && (gameInProgress)) {
    return 1;
  }

  toldMove = gameBoard.algebraicMoveToDBMove(arg1);

  if(!toldMove.isBad()) doMove(toldMove);  
  else if(!strcasecmp(arg1, "sitting") || !strcasecmp(arg1, "frozen")) { partsitting = 1;  unsit();}
  else if(!strcasecmp(arg1, "sit")) { sit(); } 
  else if(!strcasecmp(arg1, "go")) { parttoldgo = 1; unsit(); stopThought(); } 
  else if(!strcasecmp(arg1, "flag")) output("tellics flag\n");
  else if(!strcasecmp(arg1, "cancel")){ partsitting = 0;   sitting = 0; toldpartisit = 0; }
  else if(!strcasecmp(arg1, "abort")) output("tellics abort\n");
  else if(!strcasecmp(arg1, "hi") || !strcasecmp(arg1, "hello") ||
	  !strcasecmp(arg1, "hiya") || !strcasecmp(arg1, "hi!")) { output("tellics ptell hi!\n");
    return 1; } 
  
  else return -1;
  return 0;
}

