/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   division.h

   the Division class

   GJW: 1-17-1992, original creation
   GJW: 9-15-1994, DivData added
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _DIVISION_H
#define  _DIVISION_H

#include    "genlist.h"
#include    "team.h"

struct FAR  DivData;

class Division
{
public:
      enum { NAME_LENGTH = 24, };

private:
   // member functions

   // data members


protected:
   // member functions
   void                 Load (char *planfilename, ReadBFile &rlf, const DYN_16 dId, const DYN_16 newFlag);
   void                 LoadData (ReadBFile &rlf, DYN_8 *tIds);
   void                 LoadTeams (char *planfilename, ReadBFile &rlf, const DYN_8 *tIds, const DYN_16 newFlag);
   void                 SaveSubs (char *planfile, WriteBFile &wfp);
   void                 SetTeamPtr (DYN_8);

   // data members
   static PoolAlloc     *pa;  // where to get memory from

   DivData              *data;
   GenList<Team>        teamList;
   DYN_8                name [NAME_LENGTH + 1];


public:
   // member functions
   static void          Init ();
   static void          DeInit ();

   void                 *operator new (size_t size);
   void                 operator delete (void *t, size_t size);

                        Division (char *planfilename, ReadBFile &rlf, const DYN_16 dId, const DYN_16 newFlag);
                        ~Division ();

      void              Save (char *planfile, WriteBFile &wlf);

      Team              *GetTeam (DYN_8);

      DYN_8             GetNumSub ();       // return numTeams

      DYN_16            GetId () const;

      DYN_8             *GetName () const;
      void              SetName (const DYN_8 *string);

   // data members
};

#endif      // _DIVISION_H
