/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   confer.h

   the Conference class

   the real flag indicates whether this is a 'true' conference (the 8 team
   league has a conference, with 2 divisions, but the conference is just
   an internal organizational unit, it doesn't have a name that the user
   can control)

   GJW: 1-17-1992, original creation
   GJW: 9-15-1994, ConfData added
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _CONFER_H
#define  _CONFER_H

#include    "genlist.h"
#include    "division.h"

struct FAR  ConfData;

class Conference
{
public:
      enum { NAME_LENGTH = 24, };

private:
   // member functions

   // data members


protected:
   // member functions
   void              Load (char *planfilename, ReadBFile &rlf, const DYN_16 cId, const DYN_16 newFlag);
   void              LoadData (ReadBFile &rlf, DYN_8 *dIds);
   void              LoadDivs (char *planfilename, ReadBFile &rlf, const DYN_8 *dIds, const DYN_16 newFlag);
   void              SaveSubs (char *planfile, WriteBFile &wfp);
   void              SetDivPtr (DYN_8);

   // data members
   static PoolAlloc  *pa;  // where to get memory from

   ConfData          *data;
   GenList<Division> divList;
   DYN_8             name [NAME_LENGTH + 1];


public:
   // member functions
   static void       Init ();
   static void       DeInit ();

   void              *operator new (size_t size);
   void              operator delete (void *t, size_t size);

                     Conference (char *planfilename, ReadBFile &rlf, const DYN_16 cId, const DYN_16 newFlag);
                     ~Conference ();

   void              Save (char *planfile, WriteBFile &wlf);

   Division          *GetDiv (DYN_8);
   Team              *GetTeam (DYN_8, DYN_8);

   DYN_8             GetNumSub ();       // return numDivs

   DYN_16            GetId () const;
   DYN_16            GetReal () const;
   void              SetReal (const DYN_16 value);

   DYN_8             *GetName () const;
   void              SetName (const DYN_8 *string);

// data members
};

#endif      // _CONFER_H
