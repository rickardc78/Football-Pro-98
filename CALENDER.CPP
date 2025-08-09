/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   calender.cpp

   GJW: 3-13-1992, original creation
   GJW: 6-18-1992, added start of season, college & supplemental draft,
                     training camp
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <dos.h>
#include       <stddef.h>
#include       <stdlib.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "calender.h"
#include       "lgefile.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

struct CalDiskData
{
         DYN_16      firstYear,
                     lastYear;
         DYN_8       numPSWeeks,
                     numWeeks,
                     numCals;
};

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
INCLUDE_ASSERT;

static DYN_8         calFileText [] = "calender.dat",
                     SPCText [] = "SPC:",
                     SWCText [] = "SWC:";

DYN_16               Calender::firstYear,
                     Calender::lastYear;
DYN_8                Calender::numPSWeeks,
                     Calender::numWeeks,
                     Calender::numCals,
                     Calender::totalWeeks,
                     Calender::numYears,
                     *Calender::calPtr;
Calender::WeekDate   *Calender::wdPtr;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Calender

   FUNCTION:      Calender object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created Calender object
   ---------------------------------------------------------------------
*/

Calender::Calender ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Calender

   FUNCTION:      Calender object DESTRUCTOR

   INPUT:         -> Calender object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Calender::~Calender ()
{
   if (wdPtr)
      delete [] wdPtr;

   if (calPtr)
      delete [] calPtr;
}






/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a calender file
               SPC (Season Perpetual Calender)
               SWC (Season Weekly Calender)
   INPUT:      calender -> structure

   RETURNS:    new one loaded
   ---------------------------------------------------------------------
*/

void
Calender::Load ()
{
   ReadBFile   rlf;
	CalDiskData cdd;
   sysAssert (rlf.Open (calFileText), EXITMSG_CALENDER_FILE);

   vmAssert (rlf.Find (SPCText, 1) != -1L);

//   rlf.Read (&firstYear, sizeof (CalDiskData)); no-workee in MS world
   rlf.Read (&cdd, sizeof (CalDiskData));
	firstYear	= cdd.firstYear;
	lastYear		= cdd.lastYear;
	numPSWeeks	= cdd.numPSWeeks;
	numWeeks		= cdd.numWeeks;
	numCals		= cdd.numCals;

   totalWeeks = numPSWeeks + numWeeks;

   numYears = lastYear - firstYear + 1;
   calPtr = new DYN_8 [numYears];
   if (calPtr)
      rlf.Read (calPtr, numYears);

   WeekDate    *ptr = wdPtr = (WeekDate *)
                        new DYN_8 [totalWeeks * numCals * sizeof (WeekDate)];
   vmAssert (ptr);
   DYN_8       nC = numCals;

   while (nC--)
   {
      if (rlf.Find (SWCText, 0) != -1L)
      {
         rlf.Read (ptr, totalWeeks * sizeof (WeekDate));
         ptr += totalWeeks;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       GetData

   FUNCTION:   return a certain year/week's actual month or day

   INPUT:      data = MONTH/DAY indicator
               year = year in question
               week = week in question (neagative numbers indicate preseason)

   RETURNS:    month or day
   ---------------------------------------------------------------------
*/

DYN_8
Calender::GetData (cwdData data, DYN_16 year, DYN_8 week)
{
   // clip year to the valid range
   // and make it a 0-based index off the first year kept in table
   if (year < firstYear)
      year = firstYear;

   else if (year > lastYear)
      year = firstYear + ((year - lastYear - 1) % numYears);

   year -= firstYear;

   // now determince which calender to use for this year (which is now just
   // an index into all the know year mappings)
   year = calPtr [year] - 1;

   // clip week to the valid range, and make it a 0-based index
   // after adjusting for preseason weeks
   week += numPSWeeks;
   if (week < 0)
      week = 0;

   else if (week > totalWeeks)
      week = totalWeeks;

   // point to the specific month and day
   WeekDate *wdFindPtr = wdPtr + (totalWeeks * year) + week;
   DYN_8    value = 0;

   switch (data)
   {
      case WEEKDATE_MONTH:
         value = wdFindPtr->month;
         break;

      case WEEKDATE_DAY:
         value = wdFindPtr->day;
         break;
   }

   return value;
}

/*
   ---------------------------------------------------------------------
   NAME:       FillWeekDate

   FUNCTION:   return a certain year/week's actual month AND day
               (basically 2 calls to the above function)

   INPUT:      wdDest-> where to put info
               year = year in question
               week = week in question

   RETURNS:    wdDest filled with month & day
   ---------------------------------------------------------------------
*/

void
Calender::FillWeekDate (WeekDate &wdDest, DYN_16 year, DYN_8 week)
{
   wdDest.month = GetData (WEEKDATE_MONTH, year, week);
   wdDest.day = GetData (WEEKDATE_DAY, year, week);
}






/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/










/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
