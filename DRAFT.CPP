/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   draft.cpp

   the Draft class, keeps track of the order, and who's turn to pick

   WARNING:
   when generating a new order, it is assumed that an entire league has
   a maximum of 64 teams

   GJW: 4-17-1992, original creation
   GJW: 10-6-1994, added random order
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// #include       <dir.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "draft.h"
#include       "fileutil.h"
#include       "fbmufunc.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

const MAX_TEAMS = 32;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static void NEAR makeDDData (DraftData *dd, const DYN_8 id, const DYN_8 enable);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_8   dftText [] = ".dft",
               DODText [] = "DOD:";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Draft

   FUNCTION:      Draft object CONSTRUCTOR
                  if path given, load in the draft order info

   INPUT:         pName-> string of path
                  fName -> string of filename

   RETURNS:       -> created Draft object
   ---------------------------------------------------------------------
*/

Draft::Draft ()
   :  draftOrder (MAX_TEAMS, (DYN_16) 0)
{
}

Draft::Draft (DYN_8 *pName, DYN_8 *fName)
   :  draftOrder (MAX_TEAMS, (DYN_16) 0)
{
   Load (pName, fName);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Draft

   FUNCTION:      Draft object DESTRUCTOR

   INPUT:         -> Draft object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Draft::~Draft ()
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
   NAME:       Reset

   FUNCTION:   prepare for a new draft:
               for each team, set the enable to TRUE
               (SetData modifies the list currency to that of current,
               so the for loop modifying curIndex sequentially is necessary
               as opposed to the normal:
                  while (TRUE)
                  {
                     if (++draftOrder)
                        break;
                  }

               then set current index to 0 and round to 1

   INPUT:      draftOrder

   RETURNS:    DraftData.enables = TRUE
               round, curIndex initialized
   ---------------------------------------------------------------------
*/

void
Draft::Reset ()
{
   DYN_16   numTeams = draftOrder.size ();

   for (curIndex = 0; curIndex < numTeams; curIndex++)
      SetData (DRAFTDATA_DDENABLE, TRUE);

   curIndex = 0;
   round = 1;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetList

   FUNCTION:   return the list of DraftData to the caller

   INPUT:      none

   RETURNS:    -> list of team ids & enable flags
   ---------------------------------------------------------------------
*/

GenList<DYN_16> *
Draft::GetList ()
{
   return &draftOrder;
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetData

   FUNCTION:   get or set:
               index (i.e. which slot is choosing 1, 2, 3 etc.)
               Team id (i.e. which actual team, 20, 5, 19 etc)
               round (i.e. which set of picks)

               DDID, DDENABLE return structure info contained within DYN_16

               when setting data and dData is DDENABLE, the CURRENT
               team is affected

   INPUT:      Draft object
               dData = type of data requested
               value = team id to find, or is a ->DraftData info

   RETURNS:    0-based index
               Team id for given index
               round number
   ---------------------------------------------------------------------
*/

DYN_16
Draft::GetData (draftData dData)
{
   DYN_16   value = 0;

   switch (dData)
   {
      case DRAFTDATA_CURINDEX:
         value = curIndex;
         break;

      case DRAFTDATA_CURID:
         SeekToCurIndex ();
         value = GetData (DRAFTDATA_DDID, draftOrder ());
         break;

      case DRAFTDATA_ROUND:
         value = round;
         break;

      case DRAFTDATA_NUMENABLED:
         if (!draftOrder.start ())
         {
            while (TRUE)
            {
               if (GetData (DRAFTDATA_DDENABLE, draftOrder ()))
                  value++;

               if (++draftOrder)
                  break;
            }
         }
         break;
   }

   return value;
}

DYN_16
Draft::GetData (draftData dData, DYN_16 tIndex)
{
   DYN_16   value = 0;

   if (!draftOrder.start ())
   {
      if (--tIndex)
         draftOrder += tIndex;

      switch (dData)
      {
         case DRAFTDATA_DDENABLE:
            value = GetData (DRAFTDATA_DDENABLE, draftOrder ());
            break;

         default:
            break;
      }
   }

   return value;
}

DYN_16
Draft::GetData (draftData dData, DYN_16 *ddPtr)
{
   DraftData   *dd = (DraftData *) &ddPtr;

   switch (dData)
   {
      case DRAFTDATA_DDID:
         return dd->id;

      case DRAFTDATA_DDENABLE:
         return dd->enable;

      default:
         return 0;
   }
}

void
Draft::SetData (draftData dData, DYN_16 value)
{
   switch (dData)
   {
      case DRAFTDATA_ROUND:
         round = (DYN_8)value;
         break;

      case DRAFTDATA_DDENABLE:
         DYN_16   dd;

         SeekToCurIndex ();
         makeDDData ((DraftData *) &dd,
                     (DYN_8)GetData (DRAFTDATA_DDID, draftOrder ()),
                     (DYN_8)value);
         draftOrder.update ((DYN_16 *) dd);
         break;

      default:
         break;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   open a file & write the "DOD:" block
               "DOD:"
               numIds               sizeof (DYN_8)
               curIndex             sizeof (DYN_8)
               round                sizeof (DYN_8)
               draftOrder           numIds * sizeof (DraftData)

   INPUT:      pName-> path
               fName-> base filename

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
Draft::Save (DYN_8 *pName, DYN_8 *fName)
{
   WriteBFile  wlf;
   if (wlf.Open (makeFileName (0, pName, fName, dftText)))
   {
      wlf.OpenBlock (DODText);

      DYN_16   numTeams = draftOrder.size ();
      wlf.Write (&numTeams, sizeof (DYN_8));
      wlf.Write (&curIndex, sizeof (DYN_8));
      wlf.Write (&round, sizeof (DYN_8));

      if (!draftOrder.start ())
      {
         while (TRUE)
         {
            DYN_16      dd = (DYN_16) draftOrder ();
            wlf.Write (&dd, sizeof (DraftData));
            if (++draftOrder)
               break;
         }
      }

      wlf.CloseBlock ();
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       NextPick

   FUNCTION:   move curIndex to the next enabled team to choose

   INPUT:      curIndex, draftOrder
               if curIndex == number of teams,
               move on to the next round

   RETURNS:    round number
               -1 if all teams are not enabled
   ---------------------------------------------------------------------
*/

DYN_16
Draft::NextPick ()
{
   DYN_16   numTeams = draftOrder.size ();
   DYN_16   i = 0;

   SeekToCurIndex ();

   // draft is terminated once ALL teams have become dis enabled
   while (i < numTeams)
   {
      // advance to next position in draft
      // if at end of list, go back to beginning
      ++curIndex;
      ++draftOrder;

      if (curIndex == numTeams)
      {
         draftOrder.start ();
         curIndex = 0;
         round++;
      }

      // determine if this team is still involved
      DYN_16   *dd = draftOrder ();

      if (GetData (DRAFTDATA_DDENABLE, dd))
         break;

      else
         i++;
   }

   // if while loop caused termination, no team is enabled
   if (i == numTeams)
      round = -1;

   return round;
}

/*
   ---------------------------------------------------------------------
   NAME:       Generate

   FUNCTION:   if this is a new season, determine the draft order by
               who had the best to worst records

   INPUT:      tList-> list of teams in this league
               sPtr-> schedule (the one who knows everything about all
                                 the teams)
                     if 0, then generate a RANDOM order of teams

   RETURNS:    draftOrder set
   ---------------------------------------------------------------------
*/

void
Draft::Generate (GenList<DYN_16> *tList, Schedule *sPtr)
{
   // first delete the old one
   Flush ();

   DYN_16   nT = tList->size ();
   DYN_8    tIds [MAX_TEAMS];

   // if schedule is provided, generate order based on the standings
   if (sPtr)
      sPtr->GetStandings (tIds, tList);

   // otherwise generate a random order
   else
   {
      DYN_16   numT = nT;
      DYN_8    tId = 1,
               *ptr = tIds;

      // first, fill the array with the valid team ids
      while (numT--)
         *ptr++ = tId++;

      // then mix them up
      DYN_16   numShuffles = getPosRange (20, 40);
      while (numShuffles--)
      {
         numT = nT;

         ptr = tIds;
         while (numT--)
         {
            DYN_8 *src = &tIds [getPosRange (0, nT - 1)],
                  temp = *src;
            *src = *ptr;
            *ptr++ = temp;
         }
      }
   }

   // now get a list of team id's in the order of last season's finish
   // (or random, whatever way it was generated)
   while (nT)
   {
      DYN_16   dd;
      makeDDData ((DraftData *) &dd, tIds [nT - 1], TRUE);
      draftOrder.linkin ((DYN_16 *) dd);
      nT--;
   }

   curIndex = 0;
   round = 1;
}

/*
   ---------------------------------------------------------------------
   NAME:       Generate

   FUNCTION:   Use the given array of team ids to generate a draft order
					for the initial common draft.  All teams will be enabled.

   INPUT:      tIds
						An array of team ids.  The first team id is the team
						with the first draft pick.
					tCount
						The number of team ids.  There should never be more
						than 28 team ids.

   RETURNS:    draftOrder set
   ---------------------------------------------------------------------
*/

void
Draft::Generate (DYN_16 FAR *tIds, DYN_16 tCount)
{
   Flush ();

 	for (DYN_16	index = 0; index < tCount; index++)
   {
      DYN_16   dd;
      makeDDData ((DraftData *) &dd, (DYN_8)(tIds [index]), TRUE);
      draftOrder.linkin ((DYN_16 *) dd);
   }

   curIndex = 0;
   round = 1;

   SeekToCurIndex ();
}









/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   if the file exists, read in the currently stored draft
               order

   INPUT:      pName-> string of path
               fName-> string of file to open & read

   RETURNS:    TRUE/FALSE if data was found and read
   ---------------------------------------------------------------------
*/

void
Draft::Load (DYN_8 *pName, DYN_8 *fName)
{
   ReadBFile   rlh;
   if (rlh.Open (makeFileName (0, pName, fName, dftText)) > 0)
   {
      if (rlh.Find (DODText, 1) != -1)
      {
         DYN_8    numTeams;
         rlh.Read (&numTeams, sizeof (DYN_8));
         rlh.Read (&curIndex, sizeof (DYN_8));
         rlh.Read (&round, sizeof (DYN_8));

         while (numTeams--)
         {
            DYN_16   dd;
            rlh.Read (&dd, sizeof (DraftData));
            draftOrder.linkin ((DYN_16 *) dd);
         }

         SeekToCurIndex ();
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SeekToCurIndex

   FUNCTION:   advance the list pointer in draftOrder to the position
               specified by curIndex

   INPUT:      curIndex = 0 based list index
               draftOrder = list of DraftData

   RETURNS:    draftOrder's currency set to curIndex
   ---------------------------------------------------------------------
*/

void
Draft::SeekToCurIndex ()
{
   if (!draftOrder.start ())
   {
      if (curIndex)
         draftOrder += curIndex;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   delete all the links that are the draft order

   INPUT:      draftOrder = linked list of DraftData info

   RETURNS:    draftOrder removed
   ---------------------------------------------------------------------
*/

void
Draft::Flush ()
{
   if (!draftOrder.start ())
   {
      while (TRUE)
      {
         DYN_16   dd = (DYN_16) draftOrder.linkout ();
         if (!dd)
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
   NAME:       makeDDData

   FUNCTION:   because of the type checking, need to put data into a
               DYN_16 somehow

   INPUT:      dd-> 2 bytes of where data will go
               id, enable = the members

   RETURNS:    dd->data updated
   ---------------------------------------------------------------------
*/

static void NEAR
makeDDData (DraftData *dd, const DYN_8 id, const DYN_8 enable)
{
   dd->id = id;
   dd->enable = enable;
}
