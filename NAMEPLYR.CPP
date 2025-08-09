/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   nameplyr.cpp

   the player name generation class

   GJW: 3-26-1992, original creation
   GJW: 10-19-1994, store strings in EMM
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <io.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

//extern "C" {
   #include    <vm_mylib.h>
   #include    <vm.h>
//}

#include    "nameplyr.h"
#include    "fbmu.h"

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

static DYN_8   firstNameFile [] = "namef.dat",
               lastNameFile [] = "namel.dat";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:       NamePlayer

   FUNCTION:   NamePlayer object CONSTRUCTOR
               allocate memory in EMM
               open the 2 files of names data & read it in

   INPUT:      none

   RETURNS:    -> created NamePlayer object
   ---------------------------------------------------------------------
*/

NamePlayer::NamePlayer ()
   :  bae (32 * 1024)
{
   ReadFile (firstNameFile, firstNames);
   MovePtr (firstNames, getPosRand () % 2500);

   ReadFile (lastNameFile, lastNames);
   MovePtr (lastNames, getPosRand () % 2500);
}

NamePlayer::~NamePlayer ()
{
}






/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetName

   FUNCTION:   returns a randomly generated name in the given buffer
               in the form:
               Lastname, First

   INPUT:      fName, lName-> where strings should go
               (they must be big enough to hold the longest name)

   RETURNS:    TRUE/FALSE if name was generated
   ---------------------------------------------------------------------
*/

DYN_16
NamePlayer::GetName (DYN_8 *fName, DYN_8 *lName)
{
   DYN_16   ok = FALSE;

   if (firstNames.start && lastNames.start)
   {
      MovePtr (firstNames, getPosRand () % 300 + 100);
      strcpy (fName, firstNames.cur);
      MovePtr (lastNames, getPosRand () % 300 + 100);
      strcpy (lName, lastNames.cur);

      ok = TRUE;
   }

   return ok;
}





/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       MovePtr

   FUNCTION:   run through the list of names looking for cur += seeks,
               where each cur is a null terminated string

   INPUT:      nbr-> NameBuffer structure (contains start, current &
               end pointers)
               seeks = how many names to skip

   RETURNS:    nb->cur += seeks
   ---------------------------------------------------------------------
*/

void NEAR
NamePlayer::MovePtr (NameBuffer &nb, DYN_16 seeks)
{
   DYN_8 FAR *ptr = nb.cur,
         FAR *ePtr = nb.end;

   while (seeks--)
   {
      while (*ptr++)
         ;

      if (ptr == ePtr)
         ptr = nb.start;
   }

   nb.cur = ptr;
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadFile

   FUNCTION:   open the file, allocate a buffer, and read the
               file in

   INPUT:      fName-> which file to open
               nb = NameBuffer to fill

   RETURNS:    -> buffer where file was read into
   ---------------------------------------------------------------------
*/

void NEAR
NamePlayer::ReadFile (DYN_8 *fName, NameBuffer &nb)
{
   FILE     *fp = my_fopen (fName, "rb");
   DYN_8    FAR *ptr;
   nb.start = nb.end = nb.cur = 0;

   if (fp)
   {
      DYN_16   fHandle = fileno (fp);
      DYN_32   fSize = filelength (fHandle);

      // get a buffer in EMM
      if (ptr = (DYN_8 FAR *) bae.New ((DYN_16)fSize))
         my_fread (ptr, (DYN_16)fSize, sizeof (DYN_8), fp);

      my_fclose (fp);

      nb.start = nb.cur = ptr;
      nb.end = ptr + (DYN_U_16) fSize;
   }
}







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/
