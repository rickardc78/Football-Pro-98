/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   fileutil.h

   GJW: 1-25-1991, original creation
		
	History:
	
	MMQ	14-Feb-97	Added FILEUTIL_ACCESS to all function prototypes.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _FILETUTIL_H
#define  _FILETUTIL_H

#include <stdio.h>
#include "bfile.h"

#define FILEUTIL_ACCESS			PROCACCESS

//    FILEUTIL_ACCESSal functions
FILEUTIL_ACCESS DYN_16 copyFile (DYN_8 *dPath, DYN_8 *sPath);
FILEUTIL_ACCESS DYN_16 copyFile (DOSFILE *dFP, DOSFILE *sFP);
FILEUTIL_ACCESS DYN_16 copyFile (FILE *dFP, FILE *sFP);

FILEUTIL_ACCESS DYN_16 getShort (AFILE *fp);
FILEUTIL_ACCESS DYN_16 getShort (FILE *fp);
FILEUTIL_ACCESS DYN_16 putShort (DYN_U_16 value, FILE *fp);
FILEUTIL_ACCESS void getString (DYN_8 *string, FILE *fp);
FILEUTIL_ACCESS void putString (DYN_8 *string, FILE *fp);


FILEUTIL_ACCESS DYN_8 *makeFileName (DYN_8 *dest, DYN_8 *inPath, DYN_8 *file, DYN_8 *ext);

FILEUTIL_ACCESS void updateFile (DYN_8 *newBuff, DYN_8 *oldBuff);

//    FILEUTIL_ACCESSal variables
#endif   // _FILETUTIL_H

