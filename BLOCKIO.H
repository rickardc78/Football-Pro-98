/*	컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   blockio.h

   function definitions for those in blockio.c

   This file contains code that helps in writing blockified files.

   By Rich Rayl
		
	History:
	
	MMQ	14-Feb-97	Added BLOCKIO_ACCESS to all function prototypes.

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴	*/

#ifndef  _BLOCKIO_H
#define  _BLOCKIO_H

#include    <stdio.h>

#define BLOCKIO_ACCESS		PROCACCESS

//    external functions
BLOCKIO_ACCESS short open_block (char *block, FILE *fh);
BLOCKIO_ACCESS short close_block (FILE *fh);

//    external variables

#endif   // _BLOCKIO_H
