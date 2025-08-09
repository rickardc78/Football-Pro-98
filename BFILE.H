/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   BFILE.H

   History:

   MMQ	14-Feb-97	Added BFILE_ACCESS to class definition and all function
							prototypes.

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef _BFILE_H
#define _BFILE_H

#include "dosfile.h"

/*
  cur_path is the current path string.  The '+ 1' is for NULL ending.
  endoffs gives the ending absolute file offsets for each block in
  the current path.  These are needed to know when to 'fall out' in
  the search.  The 0th end offset always contains the file length,
  and is set in open().
*/

#define BFILE_ACCESS			PROCACCESS


class BFILE_ACCESS BFILE : public DOSFILE
{
   enum { BF_MAX_DEPTH = 6 };

   long endoffs[ BF_MAX_DEPTH + 1 ];
   char cur_path[ BF_MAX_DEPTH * 4 + 1 ];
   short cur_depth;
   short cur_N;      /* which occurance of cur_path */
   long cur_off;     /* current offset into file */
   long cur_size;    /* size of current block */

   void init();
   void reset();
   long error();

public:
   short xopen( char *fname );
   short open( char *fname );

   long bsize() { return cur_size; }
   long bfind( char *path, short N );

   void get_context( BFILE *bf );
   void set_context( BFILE *bf );
};

BFILE_ACCESS	short open_block(char *block, AFILE *f);
BFILE_ACCESS	short close_block(AFILE *f);
BFILE_ACCESS	void FAR * load_block( char *block, char *fname );

BFILE_ACCESS	void BFileCleanup		( void );

#endif
