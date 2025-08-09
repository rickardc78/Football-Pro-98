/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   draft.h

   the Draft class

   keeps track of the drafting order, and who's choice it is

   The methods:
   Reset       put the draft order back to its initial state
                  (i.e. curIndex = 0, and all teams enabled)
   GetList     return pointer to list of DRAFT_DATA structures
   GetData     return curIndex (i.e. 0, 1, 2), or id (i.e. 8, 21, 15)
                  of team currently selecting, or draft round
               or return portion of structure DRAFT_DATA contained with
                  a short
   SetData     set draft round explicitly
   Save        write draft info to disk
   NextPick    move curIndex to next enabled team

   GJW: 4-17-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _DRAFT_H
#define  _DRAFT_H

#include    "genlist.h"
#include    "lgefile.h"
#include    "schedule.h"

//
// dlist uses a near pointer (i.e. 16 bits) to the body of its node
// rather than store a pointer to the data, I will just store the data.
// So to fit in 16 bits, the following structure is kept at each node:
//
struct DraftData
{
   DYN_8    id,
            enable;
};

enum draftData
{
      DRAFTDATA_CURINDEX,
      DRAFTDATA_CURID,
      DRAFTDATA_ROUND,
      DRAFTDATA_DDID,
      DRAFTDATA_DDENABLE,
      DRAFTDATA_NUMENABLED,
};

class Draft
{
private:
//    member functions

//    data members


protected:
//    member functions
      void              Load (DYN_8 *, DYN_8 *);
      void              SeekToCurIndex ();
      void              Flush ();

//    data members
      GenList<DYN_16>   draftOrder;
      DYN_8             curIndex,
                        round;

public:
//    member functions
                        Draft ();
                        Draft (DYN_8 *, DYN_8 *);
                        ~Draft ();

      void              Reset ();
      GenList<DYN_16>   *GetList ();
      DYN_16            GetData (draftData);
      DYN_16            GetData (draftData, DYN_16);
      DYN_16            GetData (draftData, DYN_16 *);
      void              SetData (draftData, DYN_16);
      void              Save (DYN_8 *, DYN_8 *);
      DYN_16            NextPick ();
      void              Generate (GenList<DYN_16> *, Schedule *);
      void              Generate (DYN_16 FAR *tIds, DYN_16 tCount);

//    data members
};

#endif      // _DRAFT_H
