/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   city.h

   the City class
   GJW: 1-12-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _CITY_H
#define  _CITY_H

#include    <stdio.h>

#include    "genlist.h"
#include    "lgefile.h"

const NUM_TLEVELS = 7;
const NUM_TRANGE = 5;
const NUM_HUMID_PERCENTS = 11;
const NUM_HUMID_TEMPS = 6;
const NUM_WIND_MPHS = 7;
const NUM_WIND_TEMPS = 10;
const NUM_MONTHS = 5;

enum
{
   WC_CLEAR,
   WC_PCLDY,
   WC_CLDY,
   WC_RAIN,
   WC_SNOW,
};

enum TempLevel
{
      TC_A = 'A',
      TC_B,
      TC_C,
      TC_D,
      TC_E,
      TC_F,
      TC_G
};

//
// clear, pCloudy etc. are the low and high probabilities of certain
// conditions happening (i.e. from 1 to 100, Chicago has the following
// chanches in September:
// clear     1   20
// p cloudy 21   45
// cloudy   46   55
// rain     56  100
//

struct LowHigh
{
   DYN_8    low,
            high;
};

struct WeatherMonth
{
   LowHigh  clear, pCloudy, cloudy, rain;
   DYN_8    temp, humid;
};

struct TempLevels
{
   LowHigh  levels [NUM_TRANGE];
};

class City
{
public:
      enum { NAME_LENGTH = 15 };

private:
//    member functions

//    data members

protected:
//    member functions
      DYN_16         Load (const DYN_16 id);
      void           ReadTables (ReadBFile &rlf);
      void           ReadCity (ReadBFile &rlf);
      void           ReadWind (ReadBFile &rlf);
      void           ReadWeather (ReadBFile &rlf);

//    data members
      DYN_8          cityId,
                     name [NAME_LENGTH + 1],
                     month;
      TempLevels     tLevels [NUM_TLEVELS];
      LowHigh        tRange [NUM_TRANGE];
      WeatherMonth   weather [NUM_MONTHS];
      DYN_8          wind,
                     rtnTemp, rtnHum, rtnWS,
                     hiTable [NUM_HUMID_PERCENTS] [NUM_HUMID_TEMPS],
                     wcTable [NUM_WIND_MPHS] [NUM_WIND_TEMPS];
      DYN_16         rtnConditions;

public:
//    member functions
                     City (DYN_16);
                     ~City ();

      DYN_16         GetId () const;
      DYN_16         GetConditions ();
      DYN_16         GetTemp ();
      DYN_16         GetHum ();
      DYN_16         GetWind ();
      DYN_16         GetHeatIndex () const;
      DYN_16         GetWindChill () const;
      void           SetMonth (const DYN_16 value);

      DYN_8          *GetName () const;

//    data members
};



struct CityData
{
   DYN_8    *name;
   DYN_16   cityId;
};

extern GenList<CityData> *makeCityDataList ();

#endif      // _CITY_H
