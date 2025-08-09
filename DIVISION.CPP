/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   division.cpp

   GJW: 1-16-1992, original creation
   GJW: 3-24-1993, new format on disk (D00:)
   GJW: 4-6-1993, Read/WriteBFile
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "division.h"
#include       "lcdtutil.h"
#include       "poolaloc.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

struct DivDiskData
{
   DYN_8    id,
            confIndex,
            divIndex,
            numTeams,
            name [Division::NAME_LENGTH + 1];
};

struct FAR DivData
{
   void              FAR * operator new (size_t size);
   void              operator delete (void FAR *t, size_t size);

   static PoolAlloc  *pa;  // where to get memory from

   DYN_8             id,
                     confIndex,
                     divIndex,
                     numTeams;
   DYN_8             name [Division::NAME_LENGTH + 1];
   Team              *team;
};

const NUM_ALLOC = 12;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

// where to get memory from
PoolAlloc   *Division::pa,
            *DivData::pa;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Division

   FUNCTION:      Division object CONSTRUCTOR

   INPUT:         inFp -> vm_bfile with data to extract
                  dId = division id
                  newFlag = new league creation identifier

   RETURNS:       -> created Division object
   ---------------------------------------------------------------------
*/

Division::Division (char *planfilename, ReadBFile &rlf, const DYN_16 dId, const DYN_16 newFlag)
   // GJW: 9-15-1994, I know the maximum numbers,
   // and this is the last product, so pre-allocate the nodes
   :  teamList (5, (DYN_16) 0)
{
   data = new DivData;

   data->id = 0;
   Load (planfilename, rlf, dId, newFlag);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Division

   FUNCTION:      Division object DESTRUCTOR

   INPUT:         -> Division object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Division::~Division ()
{
   teamList.start ();
   while (TRUE)
   {
      delete teamList ();
      if (++teamList)
         break;
   }

   delete data;
}

/*
   ---------------------------------------------------------------------
   NAME:       new/delete

   FUNCTION:   the memory allocation/deallocation overloading functions
               (I have overloaded so that memory can come from a block rather
               than piece by piece)

   INPUT:      size & ptr = mandatory parameters for overloading the
               new/delete operators

   RETURNS:    -> memory allocations
   ---------------------------------------------------------------------
*/

void *
Division::operator new (size_t size)
{
   // if the size if wrong (indicating a derived class),
   // allocate out of conventional memory
   vmAssert (pa && (size == sizeof (Division)));
   void  *t = pa->New ();
   vmAssert (t);

   memset (t, 0, sizeof (Division));
   return t;
}

void
Division::operator delete (void *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (Division)));
   pa->Delete (t);
}





void FAR *
DivData::operator new (size_t size)
{
   // if there is no allocation object, or the size if wrong (indicating
   // a derived class), allocate out of conventional memory
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (DivData)));

   void  FAR *t = pa->New ();
   vmAssert (t);

   // return a 'clean slate' to the constructor
   memset (t, 0, sizeof (DivData));
   return t;
}

void
DivData::operator delete (void FAR *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (DivData)));
   pa->Delete (t);
}



/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Init

   FUNCTION:   initialize the pointer that operators new & delete
               use for allocating Division object memory
               initialize pointer for DivData memory management

   INPUT:      none

   RETURNS:    PoolAlloc object for Division created
               PoolAlloc object for DivData created
   ---------------------------------------------------------------------
*/

void
Division::Init ()
{
   pa = new PoolAlloc (sizeof (Division), NUM_ALLOC);
   DivData::pa = new PoolAlloc (sizeof (DivData), NUM_ALLOC);
   vmAssert (pa && DivData::pa);
}
void
Division::DeInit ()
{
   delete pa;
   delete DivData::pa;
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write a "D00:" block to the file
               "D00:"
               id                   sizeof (DYN_8)
               confIndex            sizeof (DYN_16)
               divIndex             sizeof (DYN_16)
               numTeams             sizeof (DYN_16)
               team block numbers   (numTeams * sizeof (DYN_16))
               name                 strlen (name)

   INPUT:      this -> Division

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
Division::Save (char *planBuff, WriteBFile &wlf)
{
   wlf.OpenBlock (D00Text);

   DivDiskData ddd;
   memcpy (&ddd.id, &data->id, sizeof (DivDiskData));
   wlf.Write (&ddd.id, sizeof (DivDiskData));

   SaveSubs (planBuff, wlf);
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTeam

   FUNCTION:   return pointer to indicated team

   INPUT:      indexes to team

   RETURNS:    -> class desired
   ---------------------------------------------------------------------
*/

Team *
Division::GetTeam (DYN_8 tIndex)
{
   Team  *tPtr = 0;

   if (tIndex < data->numTeams)
   {
      SetTeamPtr (tIndex);
      tPtr = data->team;
   }

   return tPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetNumSub

   FUNCTION:   get the number of Teams in this Division

   INPUT:      none

   RETURNS:    numTeams
   ---------------------------------------------------------------------
*/

DYN_8
Division::GetNumSub ()
{
   return data->numTeams;
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetData

   FUNCTION:   return data of Division/Team based on
               number of parameters

   INPUT:      divIndex (return Division Data)
               teamIndex (return Team Data)

   RETURNS:    ->name desired
   ---------------------------------------------------------------------
*/

DYN_16
Division::GetId () const
{
   return data->id;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetData

   FUNCTION:   return name of Division/Team based on number of
               parameters

               (first parameter is string info wanted)

   INPUT:      none (return Division Name)
               teamIndex (return Team Name)

   RETURNS:    ->name desired
   ---------------------------------------------------------------------
*/

DYN_8 *
Division::GetName () const
{
   strcpy ((DYN_8 *) name, data->name);
   return (DYN_8 *) name;
}

void
Division::SetName (const DYN_8 *string)
{
   if (strlen (string) >= NAME_LENGTH)
   {
      strncpy (data->name, string, NAME_LENGTH);
      data->name [NAME_LENGTH - 1] = 0;
   }

   else
      strcpy (data->name, string);
}










/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a Division file
               I assume a division DOES NOT HAVE MORE THAN 16 Teams!

   INPUT:      rlf -> ReadBFile to get info from
               dId = which "D00:" block to read in
               newFlag = how much/what kind of data to read in

   RETURNS:    new one loaded
   ---------------------------------------------------------------------
*/

void
Division::Load (char *planfilename, ReadBFile &rlf, const DYN_16 dId, const DYN_16 newFlag)
{
   if (rlf.Find (D00Text, dId) != -1)
   {
      DYN_8 tIds [16];
      LoadData (rlf, tIds);
      LoadTeams (planfilename, rlf, tIds, newFlag);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadData

   FUNCTION:   read in 1 block of division data

   INPUT:      rlf-> file to get info from
               tIds -> array to put team id's into
               (blockified file pointing at block to read)

   RETURNS:    division data read
   ---------------------------------------------------------------------
*/

void
Division::LoadData (ReadBFile &rlf, DYN_8 *tIds)
{
   DivDiskData ddd;
   rlf.Read (&ddd.id, sizeof (DivDiskData));
   memcpy (&data->id, &ddd.id, sizeof (DivDiskData));
   rlf.Read (tIds, data->numTeams);
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadTeams

   FUNCTION:   read in each team's data of the division

   INPUT:      tIds-> array of team id's for this division
               newFlag = league dreation indicator
               this -> Division object
               fp -> vm_bfile

   RETURNS:    team blocks read in
   ---------------------------------------------------------------------
*/

void
Division::LoadTeams (char *planFile, ReadBFile &rlf, const DYN_8 *tIds, const DYN_16 newFlag)
{

   for (DYN_8 i = 0; i < data->numTeams; i++)
   {
      Team  *tPtr = new Team (planFile, rlf, *tIds++, newFlag);
      vmAssert (tPtr);
      teamList.linkin (tPtr);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SaveSubs

   FUNCTION:   write out the rest of the division block (team ids),
               plus any teams

   INPUT:      wfp->file to write to
               teamList-> list of teams to write

   RETURNS:    data file written
   ---------------------------------------------------------------------
*/

void
Division::SaveSubs (char *planFile, WriteBFile &wfp)
{
   // first write out ids
   DYN_8    tIds [16],
            *ptr = tIds;

   teamList.start ();
   for (DYN_8 i = data->numTeams; i > 0; i--)
   {
      Team  *tPtr = teamList ();
      *ptr++ = (char)tPtr->GetId ();
      ++teamList;
   }
   wfp.Write (tIds, data->numTeams);
   wfp.CloseBlock ();

   // then the teams themselves
   teamList.start ();
   for (i = data->numTeams; i > 0; i--)
   {
      Team  *tPtr = teamList ();
      tPtr->Save (planFile, wfp);
      ++teamList;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SetTeamPtr

   FUNCTION:   go through linked list and set pointer to team

   INPUT:      index = number of links from start

   RETURNS:    team -> team data
   ---------------------------------------------------------------------
*/

void
Division::SetTeamPtr (DYN_8 index)
{
   teamList.start ();
   teamList += index;
   data->team = teamList ();
}








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
