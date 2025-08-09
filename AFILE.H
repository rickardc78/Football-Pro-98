/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   AFILE.H

   History:

   MMQ	14-Feb-97 Added AFILE_ACCESS to class definition.

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef _AFILE_H
#define _AFILE_H

#include <fbwindef.h>

#ifdef SEEK_SET
#undef SEEK_SET
#undef SEEK_CUR
#undef SEEK_END
#endif


/* Constants to be used as 2rd argument for "seek" function
*/
enum { SEEK_SET, SEEK_CUR, SEEK_END };

#define AFILE_ACCESS			PROCACCESS

class AFILE_ACCESS AFILE
{
public:
   virtual ~AFILE() {}

   // open & create methods vary from one file type to another
   virtual DYN_8 isopen() = 0;
   virtual void close() = 0;
   virtual long size() = 0;
   virtual long seek( long pos, short whence=SEEK_SET ) = 0;
   virtual DYN_U_16 read( void FAR *buffer, DYN_U_16 size ) = 0;
   virtual DYN_U_16 write( void FAR *buffer, DYN_U_16 size ) = 0;
   virtual long hugeread( void FAR *buffer, long size ) = 0;
   virtual long hugewrite( void FAR *buffer, long size ) = 0;

   long tell() { return seek(0L,SEEK_CUR); }
   void rewind() { seek(0L); }
   DYN_8 eof() { return (tell() >= size()); }
};

#endif
