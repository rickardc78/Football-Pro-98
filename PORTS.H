/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* * * * *
*
*  PORTS.H
*
*  This file should be included by all C files.  
*
*  It contains information several defines that help with 
*  porting the C files to the other machines.
*  
*  By Rich Rayl
*
*  Modification History:
*  ---------------------
*  07/17/91 RichR    File created
*
* * * * */

#ifndef PORTS_H
#define PORTS_H

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  One of the following defines should be set by the compiler.
 *  If not, you should define one at the beginning of this file.
 *
 *  __MSDOS__     If this is a PC based port.
 *  AMIGA         If this is an AMIGA based port.
 *  MACINTOSH     If this is a MACHINTOSH based port.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef WHATEVER

/* Setup the special defines for a PC based port */
#ifndef __MSDOS__
   #define __MSDOS__    0
   #define PC           0
#else
   #undef __MSDOS__
   #define __MSDOS__    1
   #define PC           1

   #define FAR          far
   #define HUGE         huge
   #define NEAR         near
   #define SWAP_DATA    0 

   typedef short        SCRN_PTR;
   typedef char far *   PAL_PTR;
   typedef short        FNT_PTR;
#endif


/* Setup the special defines for an AMIGA based port */
#ifndef AMIGA
   #define AMIGA        0
#else
   #undef AMIGA
   #define AMIGA        1

   #define FAR          
   #define HUGE         
   #define NEAR
   #define SWAP_DATA    1 

   typedef void *       SCRN_PTR;
   typedef char *       PAL_PTR;
   typedef short        FNT_PTR;
#endif



/* Setup the special defines for a MACHINTOSH based port */
#ifdef macintosh
   #define MACHINTOSH
#endif
#ifndef MACINTOSH
   #define MACINTOSH    0
#else
   #undef MACINTOSH
   #define MACINTOSH    1

   #define FAR          
   #define HUGE         
   #define NEAR
   #define SWAP_DATA    1 

   typedef short        SCRN_PTR;
   typedef void *       PAL_PTR;
   typedef short        FNT_PTR;
#endif

#endif	// WHATEVER

#endif // PORTS_H
