/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   lchamps.cpp

   LeagueChamps object (list of who's won the big game)

   GJW: 2-5-1993, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       <dyndefs.h>

#include       "bfile.h"
#include       "fileutil.h"
#include       "lchamps.h"
#include       "team.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

struct ChampDiskData
{
   DYN_8    wBuff [Team::NAME_LENGTH + 1],
            wScore,
            lBuff [Team::NAME_LENGTH + 1],
            lScore;
};

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// 8-25-1995 GJW
static DYN_8 lgcFileText [] = ".lgc",
               LGCText [] = "LGC:";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          LeagueChamps

   FUNCTION:      LeagueChamps object CONSTRUCTOR

   INPUT:         path, fName-> file to read for champions
                  inception = year of first championship

   RETURNS:       -> created LeagueChamps object
   ---------------------------------------------------------------------
*/

LeagueChamps::LeagueChamps (DYN_8 *path, DYN_8 *fName, DYN_16 inception)
{
   // GJW: 8-30-1995
   DYN_16   numChamps = 0;
   BFILE    cFP;
   if (cFP.xopen (makeFileName (0, path, fName, lgcFileText)))
   {
      ChampDiskData  cdd;
      DYN_8             // Year - Name pts, Name pts
                        buffer [4 + 3 +
                                 2 * ((Team::NAME_LENGTH + 1) + 1 + 3 + 1)];

      while (cFP.bfind (LGCText, 0) != -1)
      {
         // new way stores entire game summary
         if (cFP.bsize () == sizeof (ChampDiskData))
         {
            cFP.read (&cdd, sizeof (ChampDiskData));
            sprintf (buffer, "%d - %s %d, %s %d",
                     inception + numChamps,
                     cdd.wBuff,
                     cdd.wScore,
                     cdd.lBuff,
                     cdd.lScore);
         }

         // old way just stored winner's name
         else
         {
            cFP.read (&cdd, Team::NAME_LENGTH + 1);
            sprintf (buffer, "%d - %s", inception + numChamps, cdd.wBuff);
         }

         if (Add (buffer))
            numChamps++;

         else
            break;
      }

      cFP.close ();
   }
}

/*
   ---------------------------------------------------------------------
   NAME:          ~LeagueChamps

   FUNCTION:      LeagueChamps object DESTRUCTOR
                  don't destroy the roster if this team is a copy of another

   INPUT:         -> LeagueChamps object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

LeagueChamps::~LeagueChamps ()
{
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       WriteChamp

   FUNCTION:   write out this year's big game

   INPUT:      wTeam, wScore, lTeam, lScore = data to write

   RETURNS:    data appened to file
   ---------------------------------------------------------------------
*/

void
LeagueChamps::WriteChamp (DYN_8 *path, DYN_8 *fName,
                                 DYN_8 FAR *wTeam, DYN_8 wScore,
                                 DYN_8 FAR *lTeam, DYN_8 lScore)
{
   // GJW: 8-30-1995
   DYN_8    *fNamePtr = makeFileName (0, path, fName, lgcFileText);
   DOSFILE  fpC;

   // GJW: 9-20-1995, changed xopen (read only) to open
   if (fpC.open (fNamePtr))
      fpC.seek (0, SEEK_END);

   else
      fpC.create (fNamePtr);

   if (fpC.isopen ())
   {
      ChampDiskData  cdd;
      strcpy (cdd.wBuff, wTeam);
      cdd.wScore = wScore;
      strcpy (cdd.lBuff, lTeam);
      cdd.lScore = lScore;

      open_block (LGCText, &fpC);
      fpC.write (&cdd, sizeof (ChampDiskData));
      close_block (&fpC);

      fpC.close ();
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       Copy

   FUNCTION:   copy league data from 1 file to another

   INPUT:      buffer-> workspace for 1st filename
               path/filename combinations for source & dest

   RETURNS:    .lgc file copied
   ---------------------------------------------------------------------
*/

DYN_16
LeagueChamps::Copy (DYN_8 *buffer,
                     DYN_8 *dPath, DYN_8 *dfName, DYN_8 *sPath, DYN_8 *sfName)
{
   // GJW: 8-30-1995
   makeFileName (buffer, dPath, dfName, lgcFileText);
   return copyFile (buffer, makeFileName (0, sPath, sfName, lgcFileText));
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
