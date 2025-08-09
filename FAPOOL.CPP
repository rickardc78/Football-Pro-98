/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   fapool.cpp

   the free-agent pool, a linked-list of all unattached players
   the file always has an extension of .pyf

   GJW: 4-14-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

//#include       <dir.h>
#include    <io.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "fapool.h"
#include       "fileutil.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// GJW: 10-30-1995, need more for 30 team leagues,
// upped from 2000
const DYN_16   numNodes = 3000;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
INCLUDE_ASSERT;

static DYN_8 pyfText [] = ".pyf";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:       FreeAgentPool

   FUNCTION:   FreeAgentPool object CONSTRUCTOR

   INPUT:      pName-> string of path
               fName -> string of filename
               pyrFile-> FILE with players in it

   RETURNS:    -> created FreeAgentPool object
   ---------------------------------------------------------------------
*/

FreeAgentPool::FreeAgentPool ()
   :  RosPlayerList (numNodes)
{
}

FreeAgentPool::FreeAgentPool (DYN_8 *pName, DYN_8 *fName,
                              PlayerRdWr &prw)
   :  RosPlayerList (numNodes)
{
   Load (pName, fName, prw);
}

/*
   ---------------------------------------------------------------------
   NAME:       ~FreeAgentPool

   FUNCTION:   FreeAgentPool object DESTRUCTOR

   INPUT:      -> FreeAgentPool object

   RETURNS:    none
   ---------------------------------------------------------------------
*/

FreeAgentPool::~FreeAgentPool ()
{
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write the free agent pool to disk

   INPUT:      pName-> string of path
               fName-> string of file to open & read
               listChange = TRUE/FALSE if data needs to be written

   RETURNS:    free agents info written
   ---------------------------------------------------------------------
*/

void
FreeAgentPool::Save (DYN_8 *pName, DYN_8 *fName)
{
   if (listChange)
   {
      WriteBFile  wlf;
      sysAssert (wlf.Open (makeFileName (0, pName, fName, pyfText)) > 0, EXITMSG_FREEAGENT_SAVE);

      RosPlayerList::Save (wlf);
      listChange = FALSE;
   }
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       Age

   FUNCTION:   at the start of each season, everybody gets
               healed up
               a little older

   INPUT:      freeAgentList

   RETURNS:    all the players have 1 more year of experience, with
               possible degradation of skills
   ---------------------------------------------------------------------
*/

void
FreeAgentPool::Age ()
{
   DYN_16   numPlayers = players.size ();

   players.start ();
   while (numPlayers--)
   {
      RosPlayer   *pPtr = players ();

      if (pPtr)
         pPtr->Age ();

      ++players;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Retire

   FUNCTION:   at the start of each season, every player that has enough
               'points', quits the game

   INPUT:      free agent pool

   RETURNS:    all the players are evaluated, and removed if they qualify
   ---------------------------------------------------------------------
*/

void
FreeAgentPool::Retire ()
{
   DYN_16   numPlayers = players.size ();

   players.start ();
   while (numPlayers--)
   {
      RosPlayer   *pPtr = players ();

      if (pPtr->Retire ())
      {
         delete pPtr;
         players.linkout ();
         listChange = TRUE;
      }

      else
         ++players;
   }
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       AddPlayer(s)

   FUNCTION:   Some team is putting an extra player into the free agent
               pool

               GJW: 10-15-1994, or dumping their entire team,
               in which case, don't error check ever id

   INPUT:      pPtr-> the player

   RETURNS:    if player is not empty (i.e. position is not NONE) add player to
               the list
   ---------------------------------------------------------------------
*/

DYN_16
FreeAgentPool::AddPlayer (RosPlayer *pPtr)
{
   if (pPtr->GetPosGen () != RosPlayer::POS_NONE)
      return RosPlayerList::AddPlayer (pPtr);

   // destroy empty players as they're passed in
   delete pPtr;

   return players.size ();
}

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   delete all the players in the free agent pool

   INPUT:      pName-> string of path
               fName -> string of filename

   RETURNS:    free agent file removed
   ---------------------------------------------------------------------
*/

void
FreeAgentPool::Flush (DYN_8 *pName, DYN_8 *fName)
{
   RosPlayerList::Flush ();
   unlink (makeFileName (0, pName, fName, pyfText));
}

/*
   ---------------------------------------------------------------------
   NAME:       GeneratePlayer

   FUNCTION:   computer managed teams sometimes NEED a player to fill
               a roster slot, but if there are no free agents left,
               make one to fit the bill

   INPUT:      pos = position of player to make (SPECIFIC or GENERAL)
               id = id of the new player
               fName, lName-> first & last name strings

   RETURNS:    -> newly created player (unblessed)
   ---------------------------------------------------------------------
*/

RosPlayer *
FreeAgentPool::GeneratePlayer (RosPlayer::Pos pos, DYN_16 id, DYN_8 *fName, DYN_8 *lName)
{
   RosPlayer   *pPtr = new RosPlayer ();
   pPtr->Generate (pos, id, fName, lName, RosPlayer::POS_NONE);

   // if player is added specifically for a team,
   // that player has the ability of a gomer
   pPtr->ModRatings (60, 60);

   // if the user needs a player to fill a specific position,
   // make sure that player is converted
   // (since the ratings have been whacked upside the head so badly,
   // it's not a problem that a HB becomes an FB, or vice-versa)
   if (pos < RosPlayer::POS_MAX_SPEC)
      pPtr->SetPosSpec (pos);

   RosPlayerList::AddPlayer (pPtr);

   return pPtr;
}










/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the free agent's data from file
               the file openned will contain player ids, the players
               are located elsewhere

   INPUT:      pName-> string of path
               fName-> string of file to open & read (.pyf)
               pyrFile-> FILE with players in it

   RETURNS:    free agent's info loaded
   ---------------------------------------------------------------------
*/

void
FreeAgentPool::Load (DYN_8 *pName, DYN_8 *fName, PlayerRdWr &prw)
{
   ReadBFile   rlf;
   if (rlf.Open (makeFileName (0, pName, fName, ".pyf")) > 0)
      RosPlayerList::Load (rlf, prw);
}








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
