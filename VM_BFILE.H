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
* VM_BFILE.H
*
* Include file for block file i/o routines.
*
* MODIFICATION HISTORY
* NAME            DATE     WHAT
*
* Lincoln Hutton  03/28/89 Submitted.
*
*	MMQ	14-Feb-97	Added VM_BFILE_ACCESS to all function prototypes.
*
* * * * */


#ifndef VM_BFILE_H
#define VM_BFILE_H

#ifndef  FILE
#include "stdio.h"
#endif

#define VM_BFILE_ACCESS		PROCACCESS

#define  MAX_DEPTH			6
#define  MAX_BFILES			4
#define  PARENT				0x80000000     /* high bit of long */


/*
;  This record forms a state vector for open blocked files.  Available
;  records are flagged as such by fp == 0.  cur_path is the current path
;  string.  The '+ 1' is for NULL ending.  endoffs gives the ending 
;  absolute file offsets for each block in the current path.  These are
;  needed to know when to 'fall out' in the search.  The 0th end offset
;  always contains the file length, and is set in vm_bopen ().
;  
;  The bfile structures are not dynamically allocated at run time in 
;  order to avoid possible memory segmentation problems.
*/
struct bfile  {
      FILE *   fp;
      char     cur_path [ MAX_DEPTH * 4 + 1 ];
      long     endoffs [ MAX_DEPTH + 1];
      short    depth;
      short    N;               /* which occurance of cur path */
      long     cur_off;         /* current offset into file */
      long     cur_size;        /* size of current block */
   };

VM_BFILE_ACCESS struct bfile *			find_fp ( FILE * fp );
VM_BFILE_ACCESS void							reset_bfile ( struct bfile * b  );
VM_BFILE_ACCESS int							str_eq ( register char * s1, register char * s2, short  N );
VM_BFILE_ACCESS FILE * FAR					vm_bopen ( char * fname );
VM_BFILE_ACCESS long							bfind_error ( struct bfile * b ) ;
VM_BFILE_ACCESS long FAR					vm_bfind ( FILE * fp, char * path, short N );
VM_BFILE_ACCESS long FAR					vm_bsize ( FILE * fp );
VM_BFILE_ACCESS short FAR					vm_bclose ( FILE * fp  );
VM_BFILE_ACCESS short FAR					vm_isbfile ( void * fp );
VM_BFILE_ACCESS struct bfile * FAR		vm_bfile_get ( struct bfile *, FILE * );
VM_BFILE_ACCESS short FAR					vm_bfile_set ( struct bfile * );											

#endif
