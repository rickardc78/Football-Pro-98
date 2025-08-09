/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   city.cpp

   GJW: 1-16-1992, original creation
   GJW: 2-19-1993, eliminate effects of wind chill and heat index
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "city.h"
#include       "fbmu.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

struct CityDiskData
{
         DYN_8    cityId,
                  name [City::NAME_LENGTH + 1];
};

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_8   CTYText [] = "CTY:",
               CTLText [] = "CTL:",
               CTRText [] = "CTR:",
               CHIText [] = "CHI:",
               CWCText [] = "CWC:";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          City

   FUNCTION:      City object CONSTRUCTOR

   INPUT:         id = city id

   RETURNS:       -> created City object
   ---------------------------------------------------------------------
*/

City::City (DYN_16 id)
{
   if (!Load (id))
   {
      cityId = 0;
      month = 0;
      wind = 0;
      name [0] = 0;

      WeatherMonth   *wmPtr = weather;;
      for (DYN_8 i = 0; i < NUM_MONTHS; i++, wmPtr++)
      {
         wmPtr->clear.low = 0;
         wmPtr->clear.high = 25;
         wmPtr->pCloudy.low = 26;
         wmPtr->pCloudy.high = 50;
         wmPtr->cloudy.low = 51;
         wmPtr->cloudy.high = 75;
         wmPtr->rain.low = 76;
         wmPtr->rain.high = 100;
         wmPtr->temp = TC_A;
         wmPtr->humid = 50;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:          ~City

   FUNCTION:      City object DESTRUCTOR

   INPUT:         -> City object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

City::~City ()
{
}


/*
   ---------------------------------------------------------------------
   NAME:       makeCityDataList

   FUNCTION:   fill in the given list with a bunch of CityData structures
               for each city
               (it is faster if each city id linked in as it is found in
               the blockified file, than if each city id is search for
               independantly (i.e. create city (1), (2) etc. until a city
               isn't created))

   INPUT:      nameList-> list to fill

   RETURNS:    number of cities in nameList
   ---------------------------------------------------------------------
*/

GenList<CityData> *
makeCityDataList ()
{
   GenList<CityData> *nameList = new GenList<CityData> (0, 1);
   if (nameList)
   {
      ReadBFile   rlf;
      DYN_16      cityId = 2;

      if (rlf.Open ("cities.dat") > 0)
      {
         while (TRUE)
         {
            if (rlf.Find (CTYText, cityId) == -1L)
               break;

            else
            {
               // throw away id
               DYN_8    temp;
               rlf.Read (&temp, sizeof (DYN_8));

               DYN_8    buffer [City::NAME_LENGTH + 1];
               rlf.Read (buffer, City::NAME_LENGTH + 1);

               CityData *cdPtr = (CityData *)
                           new DYN_8 [sizeof (CityData) + strlen (buffer) + 1];
               if (cdPtr)
               {
                  cdPtr->name = (DYN_8 *) (cdPtr + 1);
                  cdPtr->cityId = cityId++;
                  strcpy (cdPtr->name, buffer);

                  nameList->linkin (cdPtr);
               }
            }
         }
      }
   }

   return nameList;
}





/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetId

   FUNCTION:   return this city's id

   INPUT:      none

   RETURNS:    this id
   ---------------------------------------------------------------------
*/

DYN_16
City::GetId () const
{
   return cityId;
}


/*
   ---------------------------------------------------------------------
   NAME:       GetConditions

   FUNCTION:   determine whether city is clear, pcloudy, cloudy or raining

   INPUT:      this-> city object

   RETURNS:    WC_????
   ---------------------------------------------------------------------
*/

DYN_16
City::GetConditions ()
{
   WeatherMonth   &wmPtr = weather [month];
   DYN_8          value = getRandPercent ();

   if ((value >= wmPtr.clear.low) && (value <= wmPtr.clear.high))
      return rtnConditions = WC_CLEAR;

   if ((value >= wmPtr.pCloudy.low) && (value <= wmPtr.pCloudy.high))
      return rtnConditions = WC_PCLDY;

   if ((value >= wmPtr.cloudy.low) && (value <= wmPtr.cloudy.high))
      return rtnConditions = WC_CLDY;

   else
      return rtnConditions = WC_RAIN;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetTemp

   FUNCTION:   generate a temperature for the month

   INPUT:      this-> city object

   RETURNS:    game-time temperature
   ---------------------------------------------------------------------
*/

DYN_16
City::GetTemp ()
{
   WeatherMonth   &wmPtr = weather [month];
   DYN_8          value = getRandPercent ();
   LowHigh        *lhPtr = &tLevels [wmPtr.temp - 'A'].levels [0];

   // i will equal 0 (vCold), 1 (cold), 2 (ideal), 3 (hot), 4 (vHot)
   for (DYN_8 i = 0; i < NUM_TRANGE; i++, lhPtr++)
   {
      if ((value >= lhPtr->low) && (value <= lhPtr->high))
         break;
   }

   // now generate a random temperature with the given range
   lhPtr = &tRange [i];
   i = lhPtr->high - lhPtr->low;  // i = delta degrees

   return rtnTemp = (getPosRand () % i) + lhPtr->low;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetHum

   FUNCTION:   generate a humidity for the month

   INPUT:      this-> city object

   RETURNS:    humidity
   ---------------------------------------------------------------------
*/

DYN_16
City::GetHum ()
{
   DYN_16   hum;

   // if raining, it's gotta be wet
   if (rtnConditions == WC_RAIN)
      hum = 100;

   else
   {
      WeatherMonth   *wmPtr = &weather [month];
      DYN_8          value = getRandPercent ();

      hum = wmPtr->humid;
      if ((value >= 1) && (value <= 8))
            hum -= 20;
      else if ((value >= 9) && (value <= 28))
            hum -= 10;
      else if ((value >= 74) && (value <= 92))
            hum += 10;
      else if ((value >= 93) && (value <= 100))
            hum += 20;
   }

   return rtnHum = (char)hum;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetWind

   FUNCTION:   generate a wind speed

   INPUT:      this-> city object

   RETURNS:    starting wind speed
   ---------------------------------------------------------------------
*/

DYN_16
City::GetWind ()
{
   DYN_8    modifier = (getPosRand () % 17) - 8;   // +/- 8

   rtnWS = wind + modifier;
   if (rtnWS < 0)
      rtnWS = 0;

   return rtnWS;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetHeatIndex

   FUNCTION:   generate a modified temperature based on the humidity
               (only matters for temps > 70)

               GJW: 2-19-1993,
               effect too drastic (user complaints), so don't bother

   INPUT:      this-> city object

   RETURNS:    modified temperature
   ---------------------------------------------------------------------
*/

DYN_16
City::GetHeatIndex () const
{
//   DYN_8    tempIndex = 0,
//            humIndex = rtnHum / 10,
   DYN_8         hiTemp = rtnTemp;

#if   0
   if (humIndex >= NUM_HUMID_PERCENTS)
      humIndex = NUM_HUMID_PERCENTS - 1;

   if (rtnTemp >= 70)
   {
      if ((rtnTemp >= 70) && (rtnTemp < 75))
         tempIndex = 0;
      else if ((rtnTemp >= 75) && (rtnTemp < 80))
         tempIndex = 1;
      else if ((rtnTemp >= 80) && (rtnTemp < 85))
         tempIndex = 2;
      else if ((rtnTemp >= 85) && (rtnTemp < 90))
         tempIndex = 3;
      else if ((rtnTemp >= 90) && (rtnTemp < 95))
         tempIndex = 4;
      else
         tempIndex = 5;

      hiTemp = rtnTemp + hiTable [humIndex] [tempIndex];
   }
#endif

   return hiTemp;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetWindChill

   FUNCTION:   generate a modified temperature based on the wind speed

               GJW: 2-19-1993,
               effect too drastic (user complaints), so don't bother

   INPUT:      this-> city object

   RETURNS:    modified temperature
   ---------------------------------------------------------------------
*/

DYN_16
City::GetWindChill () const
{
//   DYN_8    tempIndex = 0,
//            windIndex = 0,
     DYN_8    wcTemp = rtnTemp;

#if   0
   if (wcTemp <= 55)
   {
      if ((rtnTemp > 50) && (rtnTemp <= 55))
         tempIndex = 0;
      else if ((rtnTemp > 45) && (rtnTemp <= 50))
         tempIndex = 1;
      else if ((rtnTemp > 40) && (rtnTemp <= 45))
         tempIndex = 2;
      else if ((rtnTemp > 35) && (rtnTemp <= 40))
         tempIndex = 3;
      else if ((rtnTemp > 30) && (rtnTemp <= 35))
         tempIndex = 4;
      else if ((rtnTemp > 25) && (rtnTemp <= 30))
         tempIndex = 5;
      else if ((rtnTemp > 20) && (rtnTemp <= 25))
         tempIndex = 6;
      else if ((rtnTemp > 15) && (rtnTemp <= 20))
         tempIndex = 7;
      else if ((rtnTemp > 10) && (rtnTemp <= 15))
         tempIndex = 8;
      else
         tempIndex = 9;

      // how hard is the wind a-blowin'?
      if ((rtnWS >= 10) && (rtnWS < 15))
         windIndex = 1;
      else if ((rtnWS >= 15) && (rtnWS < 20))
         windIndex = 2;
      else if ((rtnWS >= 20) && (rtnWS < 25))
         windIndex = 3;
      else if ((rtnWS >= 25) && (rtnWS < 30))
         windIndex = 4;
      else if ((rtnWS >= 30) && (rtnWS < 35))
         windIndex = 5;
      else
         windIndex = 6;

      wcTemp = rtnTemp - wcTable [windIndex] [tempIndex];
   }
#endif

   return wcTemp;
}


/*
   ---------------------------------------------------------------------
   NAME:       SetMonth

   FUNCTION:   (value is 1 - 12, but there are only statistics for
               SEPTEMBER (= 9) thru JANUARY (= 0), so convert:

               month    enum     index into WeatherMonth
               SEP      9        0 (so add 3 and modulo number of months)
               OCT      10       1
               NOV      11       2
               DEC      12       3
               JAN      1        4


   INPUT:      value = 1's based month

   RETURNS:    month = indexed month
   ---------------------------------------------------------------------
*/

void
City::SetMonth (const DYN_16 value)
{
   month = (value + 3) % DECEMBER;
}

DYN_8 *
City::GetName () const
{
   return (DYN_8 *) name;
}







/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a City file

   INPUT:      this -> City object

   RETURNS:    new one loaded
   ---------------------------------------------------------------------
*/

DYN_16
City::Load (const DYN_16 id)
{
   DYN_16      rtnValue = FALSE;
   ReadBFile   rlh;

   if (rlh.Open ("cities.dat") > 0)
   {
      ReadTables (rlh);

      if (rlh.Find (CTYText, id) != -1)
      {
         ReadCity (rlh);
         rtnValue = TRUE;
      }
   }

   return rtnValue;
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadTables

   FUNCTION:   read in the temp & humidity tables from the file

   INPUT:      rlf -> file with city data

   RETURNS:    tLevels, tRange tables read
   ---------------------------------------------------------------------
*/

void
City::ReadTables (ReadBFile &rlf)
{
   if (rlf.Find (CTLText, 1) != -1L)
      rlf.Read (tLevels, sizeof (tLevels));

   if (rlf.Find (CTRText, 1) != -1L)
      rlf.Read (tRange, sizeof (tRange));

   if (rlf.Find (CHIText, 1) != -1L)
      rlf.Read (hiTable, sizeof (hiTable));

   if (rlf.Find (CWCText, 1) != -1L)
      rlf.Read (wcTable, sizeof (wcTable));
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadCity

   FUNCTION:   read in city specific info

   INPUT:      rlf -> file with city data

   RETURNS:    city's name & specific weather patterns
   ---------------------------------------------------------------------
*/

void
City::ReadCity (ReadBFile &rlf)
{
   rlf.Read (&cityId, sizeof (CityDiskData));
   ReadWind (rlf);
   ReadWeather (rlf);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadWind

   FUNCTION:   read 1 byte of wind velocity

   INPUT:      rlf -> input file

   RETURNS:    city->wind
   ---------------------------------------------------------------------
*/

void
City::ReadWind (ReadBFile &rlf)
{
   rlf.Read (&wind, sizeof (DYN_8));
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadWeather

   FUNCTION:   read 5 months worth of weather statistics

   INPUT:      rlf -> input file

   RETURNS:    city->clear, pCloudy, cloudy, rain;
               temp & humid
   ---------------------------------------------------------------------
*/

void
City::ReadWeather (ReadBFile &rlf)
{
   rlf.Read (weather, NUM_MONTHS * sizeof (WeatherMonth));
}








/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
