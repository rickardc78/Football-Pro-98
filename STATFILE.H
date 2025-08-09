/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statfile.h

   the StatFile class

   GJW: 7-13-1992, original creation
   GJW: 2-23-1995, a little modifying for 32-bit (user will no longer need
                     to know about the virtual function table stuff,
                     I'll make each class avoid copying that info into
                     the record buffer)
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _STATFILE_H
#define  _STATFILE_H

#include "genlist.h"
#include "statdrvd.h"
#include "team.h"

// size of statistic written to disk
// (they're variable length depending on the statistic, but store fixed-length
// by c-tree for convenience)
const REC_LENGTH = 20;
const REC_BUFF_SIZE = REC_LENGTH;

enum StatFlush
{
      STATFLUSH_WEEK,
      STATFLUSH_SEASON,
      STATFLUSH_CAREER,
};

class StatFile
{
private:
//    member functions

//    data members


protected:
//    member functions
      DYN_16            Create (DYN_8 *, DYN_8 *, DYN_8);
      DYN_16            Open (DYN_8 *, DYN_8 *, DYN_8);
      DYN_16            BuildKey (DYN_8 *, StatType, DYN_16);
      void              DeleteStat (StatType, DYN_16);

//    data members
      DYN_16            poFlag;

public:
//    member functions
      static DYN_U_16   GetAverage (DYN_16 sum, DYN_16 trys);
      static DYN_U_16   GetAverage (DYN_32 sum, DYN_16 trys);
      static DYN_U_16   GetPct (DYN_16 sum, DYN_16 trys);
      static DYN_U_16   QBRating (DYN_U_16 *datBuff);

                        StatFile (DYN_8 *, DYN_8 *, DYN_8);
                        ~StatFile ();

                        // league games completed
                        // halftime stats
      void              Load (FILE *, DYN_8, DYN_8, DYN_8, DYN_8);
      DYN_16            Load (FILE *, DYN_8, DYN_8 *, DYN_8, DYN_8 *, DYN_16); 

      // new funcs for HOF
      DYN_16            Load (FILE *,
                              Team *, 
                              DYN_8 *, 
                              Team *t, 
                              DYN_8 *vScorePtr, 
                              DYN_16 playOffFlag, 
                              DYN_8 gn
                             );
      void              AddHOF1 (DYN_8 teamIndex, Statistic *pStat, DYN_U_16 type, DYN_U_16 id) ;
      void              AddHOF2 (DYN_8 teamIndex) ;
      void              AddHOFGSC(StatisticHOF *s, DYN_U_16 id, DYN_U_16 confIndex) ;
      void              CalcPOG() ; // calcultate the player of the game
      void              CalcPOW() ; // calcultate the player of the week
      void              CalcPOY() ; // calcultate the player of the Year et al
      void              CalcHALLOFAME() ;
      int               AddToHALLOFAME(DYN_U_16 playerId) ;


      DYN_16            AddStat (Statistic *, DYN_U_16, DYN_U_16);
      Statistic         *GetStat (StatType, DYN_16);
      DYN_16            GetStat (StatType, DYN_16, DYN_U_16 *);
      DYN_16            BuildStatList (GenList<Statistic> *, StatType, DYN_16);
      void              SortStatList (GenList<Statistic> *, StatSort);

      StatType          GetStatType (StatType, StatTimeFrame);

      void              Flush (StatFlush);

//    data members
};

#endif   // _STATFILE_H
