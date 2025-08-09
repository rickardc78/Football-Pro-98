/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   plyrlist.cpp

   generic linked list of players

   GJW: 4-14-1992, original creation
   GJW: 3-12-1993, user can specify 'ratings multipliers' to be used
                     during sorting
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <ports.h>
#include       "dyndefs.h"

#include       "plyrlist.h"
#include       "lcdtutil.h"
#include       "fbmu.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

const PID_BUFFER_SIZE = 32;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_16 compareFunc (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType);
static DYN_16 NEAR compare (RosPlayer *p1, RosPlayer *p2);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_8 PPDText [] = "PPD:";
static RatingWeights *rwPtr;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          RosPlayerList

   FUNCTION:      RosPlayerList object CONSTRUCTOR

   INPUT:         fp-> file with ids in it
                  pyrFile-> file with players data in it

   RETURNS:       -> created RosPlayerList object
   ---------------------------------------------------------------------
*/

//RosPlayerList::RosPlayerList ()
//   :  listChange (FALSE)
//{
//}

RosPlayerList::RosPlayerList (const DYN_16 numPlayers)
   :  players (numPlayers, (DYN_16) 0),
      listChange (FALSE)
{
}

RosPlayerList::RosPlayerList (ReadBFile &rlf, PlayerRdWr &prw)
{
   Load (rlf, prw);
   listChange = FALSE;
}

/*
   ---------------------------------------------------------------------
   NAME:          ~RosPlayerList

   FUNCTION:      RosPlayerList object DESTRUCTOR

   INPUT:         -> RosPlayerList object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

RosPlayerList::~RosPlayerList ()
{
   Flush ();
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       SortPlayerList

   FUNCTION:   given a list of RosPlayers,
               sort into descending abr order

   INPUT:      pList -> where list goes

   RETURNS:    list sorted based on total abrs
   ---------------------------------------------------------------------
*/

void
RosPlayerList::SortPlayerList (GenList<RosPlayer> *pList)
{
   SortPlayerList (pList, ratWeights);
}

void
RosPlayerList::SortPlayerList (GenList<RosPlayer> *pList, RatingWeights *rw)
{
   rwPtr = rw;
   ListShellSort ((zDList *) pList, 0, compareFunc);
}

/*
   ---------------------------------------------------------------------
   NAME:       XferPlayerList

   FUNCTION:   transfer all the players in this list to the destination list,
               removing the players from this list

   INPUT:      pList->destination list

   RETURNS:    this list empty, destination with the players
   ---------------------------------------------------------------------
*/

void
RosPlayerList::XferPlayerList (GenList<RosPlayer> *pList)
{
   DYN_16   numPlayers = players.size ();

   if (!players.start ())
   {
      while (numPlayers--)
      {
         RosPlayer   *pPtr = (RosPlayer *) players.linkout ();
         pList->linkin (pPtr);
      }

      listChange = TRUE;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildPlayerList

   FUNCTION:   fill in another list with a subset of the list contained
               within this object

   INPUT:      pList->destination list
               pos = what position to search for in this list and append
                     to destination list

   RETURNS:    number of players, of the given position,
               in the source list
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayerList::BuildPlayerList (GenList<RosPlayer> *pList,
                                 RosPlayer::Pos pos)
{
   DYN_16   numPlayers = players.size (),
            nP = 0;

   if (!players.start ())
   {
      while (numPlayers--)
      {
         RosPlayer   *pPtr = players ();
         if ((pPtr->GetPosGen () == pos) || (pPtr->GetPosSpec () == pos))
         {
            nP++;
            if (pList)
               pList->linkin (pPtr);
         }

         ++players;
      }
   }

   return nP;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetPlayer

   FUNCTION:   return pointer to the player in the list that has the
               given id

   INPUT:      id = player to find

   RETURNS:    -> the found player (NULL if id not found)
   ---------------------------------------------------------------------
*/

RosPlayer *
RosPlayerList::GetPlayer (DYN_16 id)
{
   if (!players.start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = players ();
         if (pPtr->GetId () == id)
            return pPtr;

         if (++players)
            break;
      }
   }

   return 0;
}

/*
   ---------------------------------------------------------------------
   NAME:       AddPlayer(s)

   FUNCTION:   add a player to the end of the list

               GJW: 10-15-1994, or dumping their entire team,
               in which case, don't error check ever id

   INPUT:      pPtr-> player to add

   RETURNS:    sizeof list after player is added
               listChange indicates Save should write to disk
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayerList::AddPlayer (RosPlayer *pPtr)
{
   DYN_16   pId = pPtr->GetId ();
   if (pId >= PlayerRdWr::BASE_PLAYER_ID)
   {
      // don't put a player in twice
      DYN_16   addOk = TRUE;
      if (!players.start ())
      {
         while (TRUE)
         {
            RosPlayer   *pPtr = players ();
            if (pId == pPtr->GetId ())
            {
               addOk = FALSE;
               break;
            }

            // at end of list, stop searching
            if (++players)
               break;
         }
      }

      // if player not already in last, add to the end
      if (addOk)
      {
         players.linkin (pPtr);
         listChange = TRUE;
      }
   }

   return players.size ();
}

DYN_16
RosPlayerList::AddPlayers (RosPlayer **pPtr, DYN_16 nP)
{
   if (nP)
   {
      while (nP--)
         players.linkin (*pPtr++);

      listChange = TRUE;
   }

   return players.size ();
}

/*
   ---------------------------------------------------------------------
   NAME:       RemovePlayer

   FUNCTION:   remove a player from the list

   INPUT:      pPtr-> player to remove

   RETURNS:    sizeof list after player is removed
               listChange indicates Save should write to disk
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayerList::RemovePlayer (RosPlayer *pPtr)
{
   DYN_16   numPlayers = players.size ();

   while (numPlayers--)
   {
      RosPlayer   *thisRPPtr = players ();

      if (thisRPPtr == pPtr)
      {
         players.linkout ();
         break;
      }

      ++players;
   }

   listChange = TRUE;

   return players.size ();
}

#if   (LCDT == 0) || (LCDT == 4) || (LCDT == 1)
/*
   ---------------------------------------------------------------------
   NAME:       SavePlayers

   FUNCTION:   write all the players to disk

   INPUT:      the linked list of player pointers

   RETURNS:    player ids written to file
   ---------------------------------------------------------------------
*/

DYN_16
RosPlayerList::SavePlayers (PlayerRdWr &prw)
{
   DYN_16   numPlayers = players.size ();

   if (!players.start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = players ();

         pPtr->Save (prw);

         if (++players)
            break;
      }
   }

   return numPlayers;
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write the list of players to disk
               write out blocks of 32 ids at a time, rather than for
               each one found

   INPUT:      fp-> destination FILE

   RETURNS:    players' info written
   ---------------------------------------------------------------------
*/

void
RosPlayerList::Save (WriteBFile &wlf)
{
   DYN_16   numPlayers = players.size (),
            numIds,
            pIds [PID_BUFFER_SIZE];

   wlf.OpenBlock (PPDText);
   wlf.Write (&numPlayers, sizeof (DYN_16));

   if (!players.start ())
   {
      while (numPlayers)
      {
         if (numPlayers >= PID_BUFFER_SIZE)
            numIds = PID_BUFFER_SIZE;
         else
            numIds = numPlayers;

         DYN_16   nIds = numIds,
                  *pIdPtr = pIds;
         while (nIds--)
         {
            RosPlayer   *pPtr = players ();
            *pIdPtr++ = pPtr->GetId ();
            ++players;
         }

         wlf.Write (pIds, numIds * sizeof (DYN_16));
         numPlayers -= numIds;
      }
   }

   wlf.CloseBlock ();
}







/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the players' data from file
               if idxPtr is valid (non-0), then read in a block of offsets,
               and create a block of players, rather than 1 at a time

   INPUT:      fp -> FILE to get data from
               pyrFile -> FILE with players in it

   RETURNS:    players' info loaded
   ---------------------------------------------------------------------
*/

void
RosPlayerList::Load (ReadBFile &rlf, PlayerRdWr &prw)
{
   // first, get all the player ids of this player list
   if (rlf.Find (PPDText, 1) != -1L)
   {
      DYN_U_16 maxPId = prw.GetMaxId ();
      DYN_16   numPlayers,
               numIds,
               pIds [PID_BUFFER_SIZE];
      rlf.Read (&numPlayers, sizeof (DYN_16));

      while (numPlayers)
      {
         if (numPlayers >= PID_BUFFER_SIZE)
            numIds = PID_BUFFER_SIZE;
         else
            numIds = numPlayers;

         rlf.Read (pIds, numIds * sizeof (DYN_16));

         DYN_16   nIds = numIds,
                  *pIdPtr = pIds;

         while (nIds--)
         {
            DYN_16   id = *pIdPtr++;
            if ((id >= PlayerRdWr::BASE_PLAYER_ID) && (id < maxPId))
               players.linkin (new RosPlayer (prw, id));
         }

         numPlayers -= numIds;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   delete the list of players
               (called when the entire group is freed (either by
               destructor or deleting the file))

   INPUT:      players = list of players to remove from system

   RETURNS:    players = empty list
   ---------------------------------------------------------------------
*/

void
RosPlayerList::Flush ()
{
   if (!players.start ())
   {
      while (TRUE)
      {
         RosPlayer   *pPtr = (RosPlayer *) players.linkout ();

         if (pPtr)
            delete pPtr;

         else
            break;
      }
   }
}








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       compareFunc

   FUNCTION:   the function called by the generic shell sort to determine
               if the 2 list elements should be swapped

   INPUT:      iPtr, plusgPtr-> 2 players
               sortType = which sort is happening
               rwPtr-> RatingsWeights (multiply each rating by its
                        corresponding 'value' to the user)

   RETURNS:    TRUE/FALSE if 2 teams should be swapped
   ---------------------------------------------------------------------
*/

//#pragma  argsused

static DYN_16
compareFunc (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType)
{
   return compare ((RosPlayer *) iPtr, (RosPlayer *) plusgPtr);
}

/*
   ---------------------------------------------------------------------
   NAME:       compare

   FUNCTION:   return which player comes first based on total abrs
               if rwPtr != 0, use it to multiply each rating
               if tied, used conventional sum of abrs

   INPUT:      p1, p2 -> RosPlayers

   RETURNS:    0 if p1 comes before p2
               !0 if p2 comes before p1
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
compare (RosPlayer *p1, RosPlayer *p2)
{
   // first compare on KEY RATINGS
   DYN_16   p1Abrs = p1->SumAbrs (rwPtr),
            p2Abrs = p2->SumAbrs (rwPtr);

   // if tied, compare ALL
   if (p1Abrs == p2Abrs)
   {
      p1Abrs = p1->SumAbrs (0);
      p2Abrs = p2->SumAbrs (0);
   }

   return p2Abrs > p1Abrs;
}


// PlyrList.cpp