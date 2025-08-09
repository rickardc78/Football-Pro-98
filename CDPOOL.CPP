/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.

   cdpool.cpp

   the college draft pool, a linked-list of all unattached players
   the file always has an extension of .pyc

   GJW: 4-14-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// #include       <dir.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "cdpool.h"
#include       "fileutil.h"
#include       "team.h"
#include       "nameplyr.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

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

static DYN_8 pycText [] = ".pyc";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          CollegeDraftPool

   FUNCTION:      CollegeDraftPool object CONSTRUCTOR

   INPUT:         pName-> string of path
                  fName -> string of filename
                  pyrFile -> FILE with players in it

   RETURNS:       -> created CollegeDraftPool object
   ---------------------------------------------------------------------
*/

CollegeDraftPool::CollegeDraftPool ()
   :  RosPlayerList (0)
{
}

CollegeDraftPool::CollegeDraftPool (DYN_8 *pName, DYN_8 *fName,
                                    PlayerRdWr &prw)
   :  RosPlayerList (0)
{
   Load (pName, fName, prw);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~CollegeDraftPool

   FUNCTION:      CollegeDraftPool object DESTRUCTOR

   INPUT:         -> CollegeDraftPool object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

CollegeDraftPool::~CollegeDraftPool ()
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

   FUNCTION:   write the college draft pool to disk

   INPUT:      pName-> string of path
               fName-> string of file to open & read
               listChange = TRUE/FALSE if data needs to be written

   RETURNS:    college draft players info written
   ---------------------------------------------------------------------
*/

void
CollegeDraftPool::Save (DYN_8 *pName, DYN_8 *fName)
{
   if (listChange)
   {
      WriteBFile  wlt;
      sysAssert (wlt.Open (makeFileName (0, pName, fName, pycText)) > 0, EXITMSG_COLLEGE_SAVE);

      RosPlayerList::Save (wlt);
      listChange = FALSE;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       MakePool

   FUNCTION:   generate a number of players of the given position

   INPUT:      pos = type of players to make
               number = how many to make
               startId = id of first player to generate
               blessed = position that gets a little extra to their
                           ratings

   RETURNS:    id of next player to generate (i.e. startId + number)
               more players generated, and added to the linked list
   ---------------------------------------------------------------------
*/

DYN_16
CollegeDraftPool::MakePool (NamePlayer *NPPtr,
                                 RosPlayer::Pos pos, DYN_16 number,
                                 DYN_16 startId, RosPlayer::Pos blessed)
{
   listChange = TRUE;

   if (number < 0)
      number = 0;

   while (number--)
   {
         DYN_8    fnBuffer [PlayerRdWr::NAME_LEN + 1],
                  lnBuffer [PlayerRdWr::NAME_LEN + 1];
         NPPtr->GetName (fnBuffer, lnBuffer);

         RosPlayer   *rpPtr = new RosPlayer ();
         rpPtr->Generate (pos, startId++, fnBuffer, lnBuffer, blessed);
         players.linkin (rpPtr);
   }

   return startId;
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
               fName-> string of file to open & read (.pyc)
               pyrFile-> FILE with players in it

   RETURNS:    college draft players's info loaded
   ---------------------------------------------------------------------
*/

void
CollegeDraftPool::Load (DYN_8 *pName, DYN_8 *fName,
                        PlayerRdWr &prw)
{
   ReadBFile   rlf;
   if (rlf.Open (makeFileName (0, pName, fName, pycText)))
      RosPlayerList::Load (rlf, prw);
}








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
