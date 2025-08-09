/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   confer.cpp

   GJW: 1-16-1992, original creation
   GJW: 3-24-1993, new format on disk (C00:)
   GJW: 4-6-1993, Read/WriteBFile
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "confer.h"
#include       "lcdtutil.h"
#include       "poolaloc.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

struct ConfDiskData
{
   DYN_8    id,
            confIndex,
            numDivs,
            real,
            name [Conference::NAME_LENGTH + 1];
};

struct FAR ConfData
{
   void              FAR * operator new (size_t size);
   void              operator delete (void FAR *t, size_t size);

   static PoolAlloc  *pa;  // where to get memory from

   DYN_8             id,
                     confIndex,
                     numDivs;
   DYN_8             real,
                     name [Conference::NAME_LENGTH + 1];
   Division          *div;
};

const NUM_ALLOC = 4;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

// where to get memory from
PoolAlloc   *Conference::pa,
            *ConfData::pa;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Conference

   FUNCTION:      Conference object CONSTRUCTOR

   INPUT:         inFp = file to get info from
                  cId = which conference to load
                  newFlag = new league creation identifier

   RETURNS:       -> created Conference object
   ---------------------------------------------------------------------
*/

Conference::Conference (char *planfilename, ReadBFile &rlf, const DYN_16 cId, const DYN_16 newFlag)
   // GJW: 9-15-1994, I know the maximum numbers,
   // and this is the last product, so pre-allocate the nodes
   :  divList (3, (DYN_16) 0)
{
   data = new ConfData;

   data->id = 0;
   Load (planfilename, rlf, cId, newFlag);
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Conference

   FUNCTION:      Conference object DESTRUCTOR

   INPUT:         -> Conference object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Conference::~Conference ()
{
   divList.start ();
   while (TRUE)
   {
      delete divList ();   // delete data
      if (++divList)
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
Conference::operator new (size_t size)
{
   // if the size if wrong (indicating a derived class),
   // allocate out of conventional memory
   vmAssert (pa && (size == sizeof (Conference)));
   void  *t = pa->New ();
   vmAssert (t);

   memset (t, 0, sizeof (Conference));
   return t;
}

void
Conference::operator delete (void *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (Conference)));
   pa->Delete (t);
}



void FAR *
ConfData::operator new (size_t size)
{
   // if there is no allocation object, or the size if wrong (indicating
   // a derived class), allocate out of conventional memory
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (ConfData)));

   void  FAR *t = pa->New ();
   vmAssert (t);

   // return a 'clean slate' to the constructor
   memset (t, 0, sizeof (ConfData));
   return t;
}

void
ConfData::operator delete (void FAR *t, size_t size)
{
   // only process items that are of the expected size
   vmAssert (pa && (size == sizeof (ConfData)));
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
               use for allocating Conference object memory
               initialize pointer for ConfData memory management

   INPUT:      none

   RETURNS:    PoolAlloc object for Conference created
               PoolAlloc object for ConfData created
   ---------------------------------------------------------------------
*/

void
Conference::Init ()
{
   pa = new PoolAlloc (sizeof (Conference), NUM_ALLOC);
   ConfData::pa = new PoolAlloc (sizeof (ConfData), NUM_ALLOC);
   vmAssert (pa && ConfData::pa);
}
void
Conference::DeInit ()
{
   delete pa;
   delete ConfData::pa;
}

/*
   ---------------------------------------------------------------------
   NAME:       Save

   FUNCTION:   write a "C00:" block to the file
               "C00:"
               id                   sizeof (DYN_8)
               confIndex            sizeof (DYN_8)
               numDivs              sizeof (DYN_8)
               real                 sizeof (DYN_8)
               div block numbers    (numDivs * sizeof (DYN_8))
               name                 strlen (name)

   INPUT:      this -> Conference

   RETURNS:    file updated
   ---------------------------------------------------------------------
*/

void
Conference::Save (char *planBuff, WriteBFile &wlf)
{
   wlf.OpenBlock (C00Text);

   ConfDiskData   cdd;
   memcpy (&cdd.id, &data->id, sizeof (ConfDiskData));
   wlf.Write (&cdd.id, sizeof (ConfDiskData));

   SaveSubs (planBuff, wlf);
}

/*
   ---------------------------------------------------------------------
   NAME:       GetDiv, Team

   FUNCTION:   return pointer to particular subsection of the conference

   INPUT:      indexes to division, team

   RETURNS:    -> class desired
   ---------------------------------------------------------------------
*/

Division *
Conference::GetDiv (DYN_8 dIndex)
{
   if (dIndex < data->numDivs)
   {
      SetDivPtr (dIndex);
      return data->div;
   }

   else
      return 0;
}

Team *
Conference::GetTeam (DYN_8 dIndex, DYN_8 tIndex)
{
   Team  *tPtr = 0;
   if (dIndex < data->numDivs)
   {
      SetDivPtr (dIndex);
      tPtr = data->div->GetTeam (tIndex);
   }

   return tPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetNumSub

   FUNCTION:   get the number of Divisions in this Conference

   INPUT:      none

   RETURNS:    numDivs
   ---------------------------------------------------------------------
*/

DYN_8
Conference::GetNumSub ()
{
   return data->numDivs;
}

/*
   ---------------------------------------------------------------------
   NAME:       Get/SetData

   FUNCTION:   return data of Conference/Division/Team based on
               number of parameters

   INPUT:      confIndex (return Division Data)
               divIndex (return Division Data)
               teamIndex (return Team Data)

   RETURNS:    ->name desired
   ---------------------------------------------------------------------
*/

DYN_16
Conference::GetId () const
{
   return data->id;
}

DYN_16
Conference::GetReal () const
{
   return data->real;
}

void
Conference::SetReal (const DYN_16 value)
{
   data->real = (char)value;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetData

   FUNCTION:   return name of Conference/Division/Team based on number of
               parameters

               (first parameter is string info wanted)

   INPUT:      none (return Conference Name)
               divIndex (return Division Name)
               teamIndex (return Team Name)

   RETURNS:    ->name desired
   ---------------------------------------------------------------------
*/

DYN_8 *
Conference::GetName () const
{
   strcpy ((DYN_8 *) name, data->name);
   return (DYN_8 *) name;
}

void
Conference::SetName (const DYN_8 *string)
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

   FUNCTION:   read in a Conference file
               I assume a confernce DOES NOT HAVE MORE THAN 16 Divisions!

   INPUT:      rlf -> ReadBFile to get info from
               cId = which "C00:" block to read in
               newFlag = how much/what kind of data to read in

   RETURNS:    new one loaded
   ---------------------------------------------------------------------
*/

void
Conference::Load (char *planFile, ReadBFile &rlf, const DYN_16 cId, const DYN_16 newFlag)
{
   if (rlf.Find (C00Text, cId) != -1)
   {
      DYN_8 dIds [16];
      LoadData (rlf, dIds);
      LoadDivs (planFile, rlf, dIds, newFlag);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadData

   FUNCTION:   read in 1 block of conference data

   INPUT:      rlf-> file to get info from
               dIds -> array to put division id's into
               (blockified file pointing at block to read)

   RETURNS:    conference data read
   ---------------------------------------------------------------------
*/

void
Conference::LoadData (ReadBFile &rlf, DYN_8 *dIds)
{
   ConfDiskData   cdd;
   rlf.Read (&cdd.id, sizeof (ConfDiskData));
   memcpy (&data->id, &cdd.id, sizeof (ConfDiskData));
   rlf.Read (dIds, data->numDivs);
}

/*
   ---------------------------------------------------------------------
   NAME:       LoadDivs

   FUNCTION:   read in each division's data of the conference

   INPUT:      rlf -> file to get info from
               dIds -> array of division ids to load
               newFlag = type of data to load

   RETURNS:    division blocks read in
   ---------------------------------------------------------------------
*/

void
Conference::LoadDivs (char *planFile, ReadBFile &rlf, const DYN_8 *dIds, const DYN_16 newFlag)
{
   for (DYN_8 i = 0; i < data->numDivs; i++)
   {
      Division *dPtr = new Division (planFile, rlf, *dIds++, newFlag);
      vmAssert (dPtr);
      divList.linkin (dPtr);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SaveSubs

   FUNCTION:   write out the rest of the conference block (division ids),
               plus any divisions

   INPUT:      wfp->file to write to
               divList-> list of divisions to write

   RETURNS:    data file written
   ---------------------------------------------------------------------
*/

void
Conference::SaveSubs (char *planBuff, WriteBFile &wfp)
{
   // first write out ids
   DYN_8    dIds [16],
            *ptr = dIds;

   divList.start ();
   for (DYN_8 i = data->numDivs; i > 0; i--)
   {
      Division *dPtr = divList ();
      *ptr++ = (char)dPtr->GetId ();
      ++divList;
   }
   wfp.Write (dIds, data->numDivs);
   wfp.CloseBlock ();

   // then the divisions themselves
   divList.start ();
   for (i = data->numDivs; i > 0; i--)
   {
      Division *dPtr = divList ();
      dPtr->Save (planBuff, wfp);
      ++divList;
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       SetDivPtr

   FUNCTION:   go through linked list and set pointer to division

   INPUT:      index = number of links from start

   RETURNS:    div -> division
   ---------------------------------------------------------------------
*/

void
Conference::SetDivPtr (DYN_8 index)
{
   divList.start ();
   divList += index;
   data->div = divList ();
}







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
