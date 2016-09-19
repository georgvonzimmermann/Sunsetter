/* ***************************************************************************
 *                                Sunsetter                                  *
 *				 (c) Ben Dean-Kawamura, Georg v. Zimmermann                  *
 *   For license terms, see the file COPYING that came with this program.    *
 *                                                                           *
 *  Name: definitions.h                                                      *
 *  Purpose: Has the definitions I think make things easier.                 *
 *                                                                           *
 *************************************************************************** */

#ifndef _DEFINITIONS_
#define _DEFINITIONS_

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short int word;
typedef signed short int sword;
typedef unsigned int duword;
typedef signed int sdword;


#ifdef _win32_

	typedef unsigned __int64 qword;
	typedef signed __int64 sqword;


	#define strcasecmp(a,b) stricmp(a,b)   // subst
	#define strncasecmp(a,b,c) _strnicmp(a,b,c)



#endif

#ifndef _win32_

	typedef long long qword;
	typedef long long sqword;

	#define __forceinline inline

#endif

#ifndef _win32_
	#define qword(target) (unsigned long long)(target##ULL)
#endif


#endif
