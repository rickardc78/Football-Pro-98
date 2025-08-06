/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   encrypt.cpp

   GJW: 2-28-1994, original creation
   GJW: 9-24-1994, ported to football
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include <fbwindef.h>
#include <string.h>

#include "ports.h"
#include "dyndefs.h"

#include "encrypt.h"
#include "fbmufunc.h"

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

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:       Encryption

   FUNCTION:   object constructor

   INPUT:      none

   RETURNS:    Encryption object created
   ---------------------------------------------------------------------
*/

Encryption::Encryption ()
{
   // set up reverse identity array as the default
   DYN_U_8  *ptr = array;
   for (DYN_16 i = 255; i >= 0;)
      *ptr++ = (DYN_U_8)(i--);
}

/*
   ---------------------------------------------------------------------
   NAME:       ~Encryption

   FUNCTION:   object destructor

   INPUT:      none

   RETURNS:    Encryption object destroyed
   ---------------------------------------------------------------------
*/

Encryption::~Encryption ()
{
}






/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetInit

   FUNCTION:   generate a couple values for the first stored
               value, and the increment

   INPUT:      s = reference to first value to store
               i = reference to increment value

   RETURNS:    s, i, initialized
   ---------------------------------------------------------------------
*/

void
Encryption::GetInit (DYN_U_8 &s, DYN_U_8 &i)
{
   s = (DYN_U_8)getPosRange (0, 255);
   i = (DYN_U_8)getPosRange (1, 255);
}

/*
   ---------------------------------------------------------------------
   NAME:       Init

   FUNCTION:   set up the array for future encryption/decription
               1) the array if filled with the values s-1
               2) the first item is written into array [0],
                  and all subsequent values (s+1, s+2...s+255)
                  are writtin into the array at array [i], array [2*i],
                  wrapping the pointer around the array whenever it goes
                  past the end, also, if the value at the pointer is
                  not s-1 (indicating a previosly stored value), then
                  ptr is incremented until an openning is found

   INPUT:      s = value that goes into array [0]
               i = increment value for each 'fill'

   RETURNS:    array initialized
   ---------------------------------------------------------------------
*/

void
Encryption::Init (const DYN_U_8 s, const DYN_U_8 i)
{
   DYN_U_8  sValue = s,
            fillValue = s - 1,
            *ptr = array,
            *ePtr = &array [256];
   memset (array, fillValue, 256);

   DYN_16   numLeft = 256;
   while (numLeft--)
   {
      // store the value
      *ptr = sValue++;

      // go on to next slot
      ptr += i;

      // wrap if necessary
      if ((ptr < array) || (ptr >= ePtr))
         ptr = &array [ptr - ePtr];

      // check if slot is empty (i.e. value stored is fillValue)
      while (*ptr != fillValue)
      {
         // if spot already taken, increment until an openning found
         ptr++;
         if (ptr == ePtr)
            ptr = array;
      }
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Encrypt/Decrypt

   FUNCTION:   convert a given set of data to/from scrambled
               if encrypting:
               c = *ptr++
               store array[array[array [c]]]

               otherwise:
               find (array [x] == (array [x] == (array [x] == c)))
               // GJW: 3-11-1994,
               instead of SEARCHING the array over and over,
               run through it once, making yet another conversion array
               so that extraction is also just a translation

   INPUT:      p-> bytes to modify
               size = number of bytes to run through

   RETURNS:    data at ptr modified
   ---------------------------------------------------------------------
*/

void
Encryption::Encrypt (void FAR *p, DYN_16 size)
{
   DYN_U_8  FAR *ptr = (DYN_U_8 FAR *) p;
   while (size--)
   {
      DYN_U_8  c = *ptr;
      *ptr++ = array [array [array [c]]];
   }
}

void
Encryption::Decrypt (void FAR *p, DYN_16 size)
{
   DYN_U_8  FAR *ptr = array,
            translate [SIZE];

   // go through the original set of data (array), and store where
   // every value is located, into translate (which 'finds' each value,
   // just once, and stores that info)
   DYN_16   count = 0;
   while (count < SIZE)
      translate [*ptr++] = (DYN_U_8)(count++);

   // now go through the user's data, and just translate all the values
   ptr = (DYN_U_8 FAR *) p;
   while (size--)
   {
      DYN_U_8  c = *ptr;
      for (DYN_16 tries = 3; tries > 0; tries--)
         c = translate [c];

      *ptr++ = c;
   }
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
