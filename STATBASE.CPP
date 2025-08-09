/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statbase.cpp

   the Statistic Base class

   GJW: 7-13-1992, original creation
   GJW: 2-23-1995, modification to 32-bit
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "fileutil.h"
#include       "statbase.h"
#include       "fbmu.h"

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

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          Statistic

   FUNCTION:      Statistic object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created Statistic object
   ---------------------------------------------------------------------
*/

Statistic::Statistic ()
{
   data.type = 0;
}

/*
   ---------------------------------------------------------------------
   NAME:          ~Statistic

   FUNCTION:      Statistic object DESTRUCTOR

   INPUT:         -> Statistic object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

Statistic::~Statistic ()
{
}









/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the Statistic object

   INPUT:      fp-> FILE to get data from
               t = stat type
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
Statistic::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   data.type = t;
   data.id = i;
   data.number = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
Statistic::FillBuff (DYN_U_16 FAR *ptr)
{
   *((Data *) ptr) = data;
}

void
Statistic::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   data.type = t;
   data.id = i;
   Statistic::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)
               make adjustments for virtual function table

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
Statistic::ReadBuff (DYN_U_16 FAR *ptr)
{
   data = *((Data *) ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data
               (move ptr back 2 bytes to account for the virtual table
               pointer maintained in the original Statistic object)

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
Statistic::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Data  *dPtr = (Data *) ptr;
   dPtr->number += data.number;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetData

   FUNCTION:   return the data found at the given offset
               (each derived class has a set of enums for accessing
               data with in the object. this enum is the number of DYN_16s
               past the 'number' data element of the Statistic base class)

   INPUT:      offset = data item wanted (offset from 'number' to return)
               obviously it should always be 0 when dealing with the base
               class, but derived classes have data beyond this lone
               element

   RETURNS:    value found at offset
   ---------------------------------------------------------------------
*/

DYN_U_16
Statistic::GetData (DYN_16 offset)
{
   DYN_U_16 FAR *ptr = &data.number + offset;
   return *ptr;
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
