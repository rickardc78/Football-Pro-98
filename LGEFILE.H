/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lgefile.h

   the BFile classes:
   ReadBFile:
      blockified files, kept in emm, with pointers to each of the blocks
      (L00:, C00:, D00:, T00:, R00:) already stored away, so file loads
      go a little faster
   WriteBFile:
      blockified writing of the data (to emm 1st, then 1 big block out)

   GJW:	4-6-93		original creation
	MMQ	14-Feb-96	Added LGEFILE_ACCESS to all class definitions.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _LGEFILE_H
#define  _LGEFILE_H

#include    <stdio.h>
#include <fbwindef.h>

#define	LGEFILE_ACCESS			PROCACCESS

class LGEFILE_ACCESS BlockFile
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      FILE              *fp;

public:
//    member functions
      virtual           ~BlockFile () {};

      virtual DYN_16    Open (const DYN_8 *fName) = 0;
      virtual void      Close () = 0;
      virtual DYN_32    Size () const = 0;

//    data members
};

class LGEFILE_ACCESS ReadBFile : public BlockFile
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
                        ReadBFile ();
      virtual           ~ReadBFile ();

      virtual DYN_16    Open (const DYN_8 *fName);
      virtual void      Close ();
      virtual DYN_32    Size () const;

      virtual DYN_32    Find (DYN_8 *string, DYN_16 occurance);
      DYN_16            Read (void *dest, const DYN_16 size);

//    data members
};

class LGEFILE_ACCESS WriteBFile : public BlockFile
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      DYN_8             blockCount;

public:
//    member functions
                        WriteBFile ();
      virtual           ~WriteBFile ();

      virtual DYN_16    Open (const DYN_8 *fName);
      virtual void      Close ();
      virtual DYN_32    Size () const;

      DYN_16            Write (const void *src, const DYN_16 size);
      DYN_16            OpenBlock (const DYN_8 *block);
      DYN_16            CloseBlock ();

//    data members
};


class LGEFILE_ACCESS LeagueReadBFile : public ReadBFile
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      struct BlockInfo {
         enum {
            NUM_CONF = 2,
            NUM_DIV = 3,
            NUM_TEAM = 5,
         };

         DYN_16   L00Offset,
                  C00Offset [NUM_CONF], C00Index,
                  D00Offset [NUM_CONF * NUM_DIV], D00Index,
                  T00Offset [NUM_CONF * NUM_DIV * NUM_TEAM], T00Index,
                  R00Offset [NUM_CONF * NUM_DIV * NUM_TEAM], R00Index,
                  S00Offset;
      }                 bInfo;

public:
//    member functions
                        LeagueReadBFile ();
      virtual           ~LeagueReadBFile ();

      virtual DYN_16    Open (const DYN_8 *fName);
      virtual DYN_32    Find (DYN_8 *string, DYN_16 occurance);

//    data members
};

#endif   // _LGEFILE_H
