/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   calender.h

   the Calender class

   GJW: 3-22-1992, original creation
   GJW: 6-18-1992, added start of season, college & supplemental draft,
                     training camp
   GJW: 7-2-1993, made data static since there's only 1 calender
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _CALENDER_H
#define  _CALENDER_H

class Calender
{
public:
      struct WeekDate
      {
         DYN_8    month,
                  day;
      };

      enum cwdData
      {
            WEEKDATE_MONTH,
            WEEKDATE_DAY,
      };

      enum PreSeasonWeeks
      {
            PRESEASONWEEK_START = -4,
            PRESEASONWEEK_CDRAFT,
            PRESEASONWEEK_SDRAFT,
            PRESEASONWEEK_TRAIN,
      };


private:
//    member functions

//    data members


protected:
//    member functions

//    data members
      static DYN_16     firstYear,
                        lastYear;
      static DYN_8      numPSWeeks,
                        numWeeks,
                        numCals,
                        totalWeeks,
                        numYears,
                        *calPtr;
      static WeekDate   *wdPtr;

public:
//    member functions
                  Calender ();
                  ~Calender ();

      static void Load ();

      DYN_8       GetData (cwdData, DYN_16, DYN_8);
      void        FillWeekDate (WeekDate &, DYN_16, DYN_8);

//    data members
};

#endif      // _CALENDER_H
