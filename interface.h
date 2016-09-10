/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: interface.h                                                        *
 *  Purpose: Has the data structures and function prototypes relating to the *
 * user interface                                                            *
 *                                                                           *
 *  Comments: interface.h contatains prototypes, structures and global       *
 * variables for communication.  See DESIGN for a description of the         *
 * what happens.                                                             *
 *                                                                           *
 *************************************************************************** */

#ifndef _INTERFACE_
#define _INTERFACE_

#include "bughouse.h"

/* logFile is the file to log input and output if LOG is defined */

#ifdef LOG

extern FILE *logFile;

#endif

/* clientLogin is what the user sees when loging in.  */

extern char *clientLogin;


#include "board.h"



extern int partner;				/* If we have a partner in
								client mode */

void output(const char *str);              /* output() prints a string to the
											approprate location */


void parseHolding(const char *str);


void parseOption(const char *str);         /* parseOption() reads the input
											that is given to Sunsetter
											and handles it accordingly. */

int checkInput();				/* See's if input is waiting and
								checks other things, like
								flag falls */

void pollForInput();			/* it will
								see if there is input waiting and
								give it to xboardOption. */

void waitForInput();			/* Don't do anything until some kind
								of input comes in */
void giveMove(move m);

void reportResult (gameResult res);

long getSysMilliSecs();



/* Functions for the partner communication */




int parsePartnerCommand(const char *arg1, const char *arg2);  
									/* Used when Sunsetters partner 
									ptells it something */

void givePartnerHelp(const char *str);     /* Gives help about the
											commands Sunsetter understands
											from it's partner */

#endif
