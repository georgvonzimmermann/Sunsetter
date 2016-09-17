Sunsetter V.8  09/12/16

README

           ^^                   @@@@@@@@@
      ^^       ^^            @@@@@@@@@@@@@@@
                           @@@@@@@@@@@@@@@@@@              ^^
                          @@@@@@@@@@@@@@@@@@@@
                          @@@@@@@@@@@@@@@@@@@@
~         ~~   ~  ~       ~~~~~~~~~~~~~~~~~~~~ ~       ~~     ~~ ~
  ~      ~~      ~~ ~~ ~~  ~~~~~~~~~~~~~ ~~~~  ~     ~~~    ~ ~~~  ~ ~~
  ~  ~~     ~         ~      ~~~~~~  ~~ ~~~       ~~ ~ ~~  ~~ ~

(c) Ben Dean-Kawamura, Georg v. Zimmermann

I. Change notes
II. Thanks
III. Compiling notes

I. Change notes
---------------
This version is based on the last code I wrote more than a decade ago (version 7e) and tries to integrate as far as possible the improvements and bugfixes made by the excellent programmers mentioned below, and some bugfixes by me.

Some improvements I have *not* yet merged into this version - my appologies:
- Some compiler specific code (please also see III. below). Reason: I am only using Microsoft MSVC as a compiler. Therefore I would appreciate if all compiler/platform specific improvements could be put into #ifdefs so as to avoid changing behaviour using MSVC.
- The opening book code by Angrim. This is excellent and I definitely want to merge it, just need to find the time to move it into seperate source and headers files (book.h, book.cpp) and make it easy to switch on/off with #ifdefs
- UPDATE: mate scores now fixed as far as I can see.   

II. Thanks
----------
- angrim for a lot of bugfixes and general code improvement (i.e. fixing undefined behaviour), as well as improvements to i/o-code and support for different platforms (please also see III. below)
- the great people of lichess.org - the smoothest chess playing platform I have ever seen - for all the improvements to Sunsetter and reviving my interest in crazyhouse programming,  niklasf, isaacl and others 
- duropo and Toadofsky from the lichess.org crazyhouse forum and others for the very helpfull error reports and improvements     

III. Compiling Notes
-------------------- 

To make Sunsetter compile on Compilers ==other than Microsoft MSVC== please try this:

1) Make your compiler (windows compiler other than MSVC) understand _win32_
Maybe like this:
#ifdef _WIN32
#ifndef _win32_
#define _win32_ 1
#endif
#endif

2) Make your compiler treat __forceinline as inline, possibly like this:
#ifndef _win32_
#define __forceinline inline
#endif


Please test as I am only familiar with MSVC. Thanks!
