/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: variables.h                                                        *
 *  Purpose:  Has varables that Sunsetter uses.                              *
 *                                                                           *
 *************************************************************************** */

#ifndef _VARIABLES_
#define _VARIABLES_

#define MAX_MOVES 512    /* The size to make the array that moves() returns
							must be big enough to hold the most possible 
							moves */

#define MAX_GAME_LENGTH 1200 /* game will be max 600 moves long */

#define MAX_STRING 512  /* The bytes of the largest string the Sunsetter uses */

#define MAX_PIECES 20

#define MAX_ARG    5		/* The most number of arguments Sunsetter takes in a
							line */

/* The transposition table must be >= MIN_HASH_SIZE */

#define MIN_HASH_SIZE (0x10000 * sizeof(transpositionEntry) * 16)

/* The learn table is 4 MB in size */

#define LEARN_SIZE (0x10000 * sizeof(transpositionEntry) * 4)

/* define this to turn on logging */

// #define LOG

/* the usual "flag" NDEBUG is used for asserts debugging */

/* define this to turn hash collision test ON */

// #define DEBUG_HASH

/* define this to turn Win/XBoard commands debugging ON */

// #define DEBUG_XBOARD 

/* define this to turn learning info ON */

// #define DEBUG_LEARN 

/* define this to turn extra statistics ON  */
	
// #define DEBUG_STATS

/* define this to output a game tree */

// #define GAMETREE 500




#endif
