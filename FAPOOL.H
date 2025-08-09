/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   fapool.h

   the FreeAgentPool class
   derived from the RosPlayerPool class (which is just a linked list
   of RosPlayers)

   GJW: 4-14-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _FAPOOL_H
#define  _FAPOOL_H

#include    "plyrlist.h"

class FreeAgentPool : public RosPlayerList
{
private:
   // member functions

   // data members


protected:
   // member functions
   void        Load (DYN_8 *, DYN_8 *, PlayerRdWr &);

   // data members

public:
   // member functions
               FreeAgentPool ();
               FreeAgentPool (DYN_8 *, DYN_8 *, PlayerRdWr &);
               ~FreeAgentPool ();

   void        Save (DYN_8 *, DYN_8 *);

   void        Age ();
   void        Retire ();
   DYN_16      AddPlayer (RosPlayer *);
   void        Flush (DYN_8 *, DYN_8 *);
   RosPlayer   *GeneratePlayer (RosPlayer::Pos, DYN_16, DYN_8 *, DYN_8 *);

   // data members
};

#endif      // _FAPOOL_H
