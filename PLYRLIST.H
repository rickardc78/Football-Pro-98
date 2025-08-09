/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   plyrlist.h

   the RosPlayerList class (a linked list of RosPlayers)

   GJW: 4-14-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _PLYRLIST_H
#define  _PLYRLIST_H

#include    "genlist.h"
#include    "lgefile.h"
#include    "rosplyr.h"

class RosPlayerList
{
private:
   // member functions

   // data members


protected:
   // member functions
      void                 Load (ReadBFile &rlf, PlayerRdWr &prw);
      void                 Flush ();

   // data members
      GenList<RosPlayer>   players;
      DYN_16               listChange;

public:
   // member functions
//                           RosPlayerList ();
                           RosPlayerList (const DYN_16 numPlayers = 0);
                           RosPlayerList (ReadBFile &rlf, PlayerRdWr &prw);
                           ~RosPlayerList ();

      static void          SortPlayerList (GenList<RosPlayer> *pList);
      static void          SortPlayerList (GenList<RosPlayer> *pList,
                                             RatingWeights *rw);

      void                 XferPlayerList (GenList<RosPlayer> *);
      DYN_16               BuildPlayerList (GenList<RosPlayer> *, RosPlayer::Pos);
      RosPlayer            *GetPlayer (DYN_16);
      DYN_16               AddPlayer (RosPlayer *);
      DYN_16               AddPlayers (RosPlayer **pPtrs, DYN_16 nP);
      DYN_16               RemovePlayer (RosPlayer *);
      DYN_16               SavePlayers (PlayerRdWr &prw);

      void                 Save (WriteBFile &wlf);

   // data members
};

#endif      // _PLYRLIST_H
