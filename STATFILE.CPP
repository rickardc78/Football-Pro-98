/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statfile.cpp

   the interface between statistics and c-tree's ISAM

   GJW: 7-13-1992, original creation
   GJW: 11-3-1992, if a team scores more than 127 points, clip the result
                     to a positive value
   GJW: 4-7-1993, fixed htstats interface:
                     team Id = 1 WINNER
                              99 LOSER
   GJW: 9-30-1994, 2 point conversions
   GJW: 10-20-1994, finally, the REAL qb passer rating formula


  CAM		10 NOV 96	Added checks in all of JWM's Hall Of Fame statistic
							 stuff for divide by 0
  CAM		18 NOV 96	Misc. fixes for Rookie of the Year, Hall of Fame
  CAM		20 DEC 96	Fixed check for when to compute the Player of the Week
  JWM    06 MAR 97   Added coach of year award

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       "fbwin.h"

#include       <io.h>
#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"
#include    <vm_mylib.h>

#include       "assertvm.h"
#include       "ct.h"
#include       "fileutil.h"
#include       "statfile.h"
#include       "league.h"


#include       "fbmu.h"
#include       "schedule.h"
#include       "awardval.hxx"
#include			"aptcount.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
#define MAXPATH 260
// c-tree file indices
const DAT_FILE = 0;
const IDX_FILE = 1;

const NUM_BUFS = 3;        // index file buffers (minimum required by c-tree)
const NUM_FILS = 2;        // maximum simultaneous open data and indice files
const NUM_SECT = 4;        // node sectors (* 128 = index node size)

const NUM_KEY_SEGS = 2;    // key will be built from type & id of stat
const KEY_LENGTH = 4;      // total size of key built from segments

// score-by-quarters: [numQtrs] [5 * scores * 2]
const QTR_SCORE_SIZE = 1 + (5 * 2);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static Statistic  * NEAR NewStatistic (const StatType type);
static DYN_16     compareFunc (void FAR *, void FAR *, DYN_16);
static DYN_16     compareFuncSigned (void FAR *, void FAR *, DYN_16);
static DYN_16     NEAR copyFile (DYN_8 *, DYN_8 *, DYN_8 *, DYN_8 *);
static DYN_8      NEAR sumScores (DYN_U_8 *);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
INCLUDE_ASSERT;

#if   (LCDT != 0)

static StatisticPAT              statPAT;
static StatisticPAT              statPAT2;
static StatisticFG               statFG;
static StatisticRush             statRush;
static StatisticPass             statPass;
static StatisticPassRec          statPassRec;
static StatisticInt              statInt;
static StatisticPunt             statPunt;
static StatisticPuntRet          statPuntRet;
static StatisticKickRet          statKickRet;
static StatisticFumble           statFumble;
static StatisticFumbleRec        statFumbleRec;
static StatisticSack             statSack;
static StatisticSafety           statSafety;
static StatisticTackle           statTackle;
static StatisticPenalty          statPenalty; // HOF JWM 9/12/96
static StatisticHOF              statHOF;
static StatisticPOG              statPOG;
static StatisticTeamFirstDowns   statTeamFirstDowns;
static StatisticTeamAttDowns     statTeamAttDowns;
static StatisticTeamMisc         statTeamMisc;

#endif

static DYN_8 fileNameBuffer [MAXPATH];

// key is made from part of Statistic object's data
// (c-tree uses the 1st byte as the delete flag, but I will never delete
// a record, it only gets modified)
static ISEG statSeg [NUM_KEY_SEGS] =
            {
               {  0, 2, INTSEG   },    // type is 1st part of key
               {  2, 2, INTSEG   }     // id is 2nd part of key
            };

static IIDX statIdx =
            {
                KEY_LENGTH,      // key length (from statSeg above, 2 + 2)
                0,               // key type
                0,               // dup off
                0,               // null off
                0,               // empty char
                NUM_KEY_SEGS,    // number of key segments
                statSeg,         // pointer to segment array
                0                // index name for r-tree ("idx" is always assumed)
            };

static IFIL statDat =
            {
               fileNameBuffer,   // data file name ("dat" is always assumed)
               0,                // data file number
               REC_LENGTH,       // data record length
               4096,             // data extension size
               1,                // data file mode
               1,                // number of indices
               4096,             // index extension size
               1,                // index file mode
               &statIdx,         // pointer to index array
               "",               // r-tree info (not used here)
               ""                // r-tree info (not used here)
            };

static DYN_8   *datText = ".dat",
               *idxText = ".idx";


// HOF JWM 9/12/96
// new globs for HOF
Team *pTeams[2] ;
DYN_8 teamIds[2] ;
DYN_8 scores[2] ;
DYN_8 *teamNames[2] ;
DYN_8 gameNumber ;
DYN_U_16 playersInGame[2][100] ;
DYN_U_16 ConferenceIndex ;
DYN_8 logFile = 0 ;
HANDLE hfLog ; // handle to awards.log
char str[256] ;
static DYN_8   *posText[] =
               {
                  "QB", "FB", "HB", "TE", "WR", "C", "G", "T",
                  "DE", "DT", "LB", "CB", "S", "K", "P",
                  "RB", "R",  "OL", "DL", "DB"
               } ;
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:       StatFile

   FUNCTION:   StatFile object CONSTRUCTOR

   INPUT:      pName, fName-> filenames for this league
               writeFlag = should data be backed up (i.e. this is
               going to modify the data)

   RETURNS:    -> created StatFile object
   ---------------------------------------------------------------------
*/

StatFile::StatFile (DYN_8 *pName, DYN_8 *fName, DYN_8 writeFlag)
{

   // initialize for this go-around
   // stop any earlier setup
   CloseISAM ();
   if (!InitISAM (NUM_BUFS, NUM_FILS, NUM_SECT))
   {
      if (!Create (pName, fName, writeFlag))
         return;
   }
   sysAssert (FALSE, EXITMSG_DATABASE_ERR);
}

/*
   ---------------------------------------------------------------------
   NAME:       ~StatFile

   FUNCTION:   StatFile object DESTRUCTOR

   INPUT:      -> StatFile object

   RETURNS:    none
   ---------------------------------------------------------------------
*/

StatFile::~StatFile ()
{
   CloseISAM ();
}








/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       GetAverage

   FUNCTION:   return the average of one number divided by another

   INPUT:      sum = total value
               trys = number of elements in sum

   RETURNS:    one decimal place to the right 4 bits:
               sum % trys     x
               ---------- = ----
                  trys       10

               solve for x

               if the number if negative, it is returned as a positive,
               with the high bit set (NOT A TRUE 2's COMPLETEMENT!)
               (i.e. -2 would be 0x8002, not 0xFFFE)
   ---------------------------------------------------------------------
*/

DYN_U_16
StatFile::GetAverage (DYN_16 sum, DYN_16 trys)
{
   DYN_16   signBit = 0;

   if (sum < 0)
   {
      sum = -sum;
      signBit = (short) (1 << 15);
   }

   if (trys == 0)
      trys = 1;

   return ((sum / trys) << 4) |
            ((10 * (sum % trys)) / trys) |
            signBit;
}

DYN_U_16
StatFile::GetAverage (DYN_32 sum, DYN_16 trys)
{
   DYN_16   signBit = 0;

   if (sum < 0)
   {
      sum = -sum;
      signBit = (short) (1 << 15);
   }

   if (trys == 0)
      trys = 1;

   return ((sum / trys) << 4) |
            ((10 * (sum % trys)) / trys) |
            signBit;
}

/*
   ---------------------------------------------------------------------
   NAME:       GetPct

   FUNCTION:   return the percentage (adjust for 3 figures to left of decimal
               point) one number divided by another

   INPUT:      sum = total value
               trys = number of elements in sum

   RETURNS:    one decimal place to the right 4 bits:
               GetAverage (trys * 100, sum)
   ---------------------------------------------------------------------
*/

DYN_U_16
StatFile::GetPct (DYN_16 sum, DYN_16 trys)
{
   return GetAverage ((DYN_32)((DYN_32) trys * 100), sum);
}

#if   (LCDT == 0)
/*
   ---------------------------------------------------------------------
   NAME:       QBRating

   FUNCTION:   generate quarterback's rating based on the following
               formula:

   OLD WAY:
   Rtg = 16 2/3 * (Comp% score + TD% score + INT% score + Avg Yds score)
   Comp% score = (Completions/Attempts - .30) * 5
   TD% score = (TDs/Attempts) * 20
   INT% score = 2.375 - (INTs/Attempts * 25)
   Avg Yds score = (Gross Yards/Attempts - 3.0) * .250

   Maximum for each score is 2.375.
   Minimum for each score is 0.000.

   16 2/3 = 50/3


   OFFICIAL WAY:
   (completions / att) - .3) / .2
   ((yds / att) - 3.0) / 4
   (tds / att) / .05
   (.095 - (ints / att)) / .04

   Maximum for each score is 2.375.
   Minimum for each score is 0.000.

   sum each step, * 100, / 6

   completions:
   c
   - -.3
   a        (10 * c) - (3 * a)
   -----  = ------------------
    .2            2 * a

   yards:
   y
   - - 3
   a       y - (3 * a)
   ----- = -----------
     4        4 * a

   tds:
    t
    -
    a    20 * t
   --- = ------
   .05     a

   ints:
          int
   .095 - ---
           a    2.375a - 25i
   ---------- = ------------
      .04            a

   INPUT:      datBuff-> PASSING STAT
               (i.e. att, comp, yds, ints, tds = qb statitistics)

   RETURNS:    qb rating
   ---------------------------------------------------------------------
*/

static DYN_U_16 NEAR
ratingBounds (DYN_16 value)
{
   if (value < 0)
      value = 0;

   else if (value > 2375)
      value = 2375;

   return value;
}

DYN_U_16
StatFile::QBRating (DYN_U_16 *datBuff)
{
   DYN_U_32    att = datBuff [STATOFFSET_NUMBER],
               comp = datBuff [STATOFFSET_COMPS],
               yds = datBuff [STATOFFSET_YARDS],
               ints = datBuff [STATOFFSET_INTS],
               tds = datBuff [STATOFFSET_TDS];

   DYN_16   value = 0;
   if (att)
   {
      DYN_16      compHolder = (DYN_16)((10L * comp) - (3L * att));
      DYN_U_16    compScore = compHolder <= 0 ?
                                 0 : ratingBounds ((DYN_16)((1000L * (compHolder)) / (2 * att))),
                  tdScore = ratingBounds ((DYN_16)((20000L * tds) / att)),
                  intScore = ratingBounds ((DYN_16)(((2375L * att) - (25000 * ints)) / att)),
                  ydsScore;

      // if yardage is negative, don't let it mess things up
      DYN_16   *ydsPtr = (DYN_16 *) &datBuff [STATOFFSET_YARDS];
      if (*ydsPtr < 0)
         ydsScore = 0;
      else
         ydsScore = ratingBounds ((DYN_16)((1000L * (yds - (3L * att))) / (4L * att)));

      // convert to 'decimal point' representation
      value = GetAverage ((DYN_32) (compScore + tdScore + intScore + ydsScore), 60);
   }

   return value;
}
#endif

#if   (LCDT != 0)
// HOF JWM 9//16/96
// new version of load to control HOF award points
DYN_16
StatFile::Load (FILE *fpIn,
                  Team *tPtrh, DYN_8 *hScorePtr,
                  Team *tPtrv, DYN_8 *vScorePtr,
                  DYN_16 playOffFlag, DYN_8 gn)
{
   // init the global vars used for HOF calculation
   memset(playersInGame,0,sizeof(playersInGame)) ;
   gameNumber = gn ;
   pTeams[0]  = tPtrh ;
   pTeams[1] = tPtrv ;
   teamIds[0] = (DYN_8)tPtrh->GetId () ;
   teamIds[1] = (DYN_8)tPtrv->GetId () ;
   teamNames[0] = tPtrh->GetName () ;
   teamNames[1] = tPtrv->GetName () ;
#if _DEBUG
   logFile = !!(strstr( GlueApp.GetCmdLine(), "-l" ) != NULL) ;
#endif

   return Load(fpIn,
               (DYN_8)tPtrh->GetId (),
               hScorePtr,
               (DYN_8)tPtrv->GetId (),
               vScorePtr,
               playOffFlag);

}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in a set of game stats from the given file

   INPUT:      fp-> file with game stats in it
               hTeamId, hScore, vTeamId, vScore = home and visting
                  team id and final score
               poFlag = playoff game (don't update season and career
                           stats if TRUE)

               at halftime, the teams are indicated as 1 & 99

   RETURNS:    number of quarters played
               database updated
   ---------------------------------------------------------------------
*/

DYN_16
StatFile::Load (FILE *fpIn,
                  DYN_8 hTeamId, DYN_8 *hScorePtr,
                  DYN_8 vTeamId, DYN_8 *vScorePtr,
                  DYN_16 playOffFlag)
{
   poFlag = playOffFlag;

   // read the score by quarters, and sum for final score
   // (the arcade game modulos to a byte)
   DYN_U_8  qtrScore [QTR_SCORE_SIZE];
   my_fread (qtrScore, QTR_SCORE_SIZE, 1, fpIn);

   DYN_8    hScore = *hScorePtr = sumScores (&qtrScore [1]),
            vScore = *vScorePtr = sumScores (&qtrScore [1 + 5]);

   scores[0] = hScore ;
   scores[1] = vScore ;

   Load (fpIn, hTeamId, hScore, vTeamId, vScore);

   return qtrScore [0];
}

void
StatFile::Load (FILE *fpIn,
                     DYN_8 hTeamId, DYN_8 hScore,
                     DYN_8 vTeamId, DYN_8 vScore)
{
   // if hTeamId or vTeamId = 99, then this is halftime
   // (don't need to store as many stats, and player numbers for
   // losing team have high bit set)
   DYN_16   htFlag = 0;
   if (hTeamId == 99)
      htFlag = 1;
   else if (vTeamId == 99)
      htFlag = 2;

   // read stats for HOME, then VISITOR
   for (DYN_8 i = 0; i < 2; i++)
   {
      if ( !htFlag && !poFlag &&
				(ActiveLeague != NULL) )
		{
	      ConferenceIndex = pTeams[i]->GetCI() ;
		}

      STATGROUP_TEAM sgTeam;
      DYN_16         pId = 0,
                     count,
                     tTeamId = i ? vTeamId : hTeamId,
                     oTeamId = i ? hTeamId : vTeamId,
                     tTeamScore = i ? vScore : hScore,
                     oTeamScore = i ? hScore : vScore;

      // during halftime, the winning team's players have high bit set
      if ((i + 1) == htFlag)
         pId = (short)0x8000;

      my_fread (&sgTeam, sizeof (STATGROUP_TEAM), 1, fpIn);

      // Team's first downs made
      DYN_U_16       datBuff [REC_BUFF_SIZE];
      Statistic      *sPtr = &statTeamFirstDowns;
      datBuff [STATOFFSET_NUMBER] = sgTeam.first_downs.rushing +
                                       sgTeam.first_downs.passing +
                                       sgTeam.first_downs.penalty;
      datBuff [STATOFFSET_TRUSHING] = sgTeam.first_downs.rushing;
      datBuff [STATOFFSET_TPASSING] = sgTeam.first_downs.passing;
      datBuff [STATOFFSET_TPENALTY] = sgTeam.first_downs.penalty;
      sPtr->ReadBuff (datBuff);
      AddStat (sPtr, TGSTAT_DOWNSFIRST, tTeamId);
      AddStat (sPtr, OTGSTAT_DOWNSFIRST, oTeamId);
      if (!htFlag && !poFlag)
      {
         AddStat (sPtr, TSSTAT_DOWNSFIRST, tTeamId);
         AddStat (sPtr, OTSSTAT_DOWNSFIRST, oTeamId);
      }

      // Team's first downs attempted
      sPtr = &statTeamAttDowns;
      datBuff [STATOFFSET_TATT3] = sgTeam.third_downs.attempted;
      datBuff [STATOFFSET_TMADE3] = sgTeam.third_downs.made;
      datBuff [STATOFFSET_TATT4] = sgTeam.fourth_downs.attempted;
      datBuff [STATOFFSET_TMADE4] = sgTeam.fourth_downs.made;
      sPtr->ReadBuff (datBuff);
      AddStat (sPtr, TGSTAT_DOWNSATT, tTeamId);
      AddStat (sPtr, OTGSTAT_DOWNSATT, oTeamId);
      if (!htFlag && !poFlag)
      {
         AddStat (sPtr, TSSTAT_DOWNSATT, tTeamId);
         AddStat (sPtr, OTSSTAT_DOWNSATT, oTeamId);
      }

      // Team's miscellaneous
      sPtr = &statTeamMisc;
      // 'number' = other team's id
      datBuff [STATOFFSET_NUMBER] = oTeamId;
      datBuff [STATOFFSET_TPENALTIES] = sgTeam.penalties;
      datBuff [STATOFFSET_TYARDSLOST] = sgTeam.yards_lost;
      datBuff [STATOFFSET_TTIME] = sgTeam.time_of_possession;
      datBuff [STATOFFSET_TFUMLOST] = sgTeam.fumblesLost;
      // final score us | them
      datBuff [STATOFFSET_TFINAL] = (tTeamScore << 8) | oTeamScore;
      sPtr->ReadBuff (datBuff);
      AddStat (sPtr, TGSTAT_MISC, tTeamId);
      if (!htFlag && !poFlag)
         AddStat (sPtr, TSSTAT_MISC, tTeamId);
      datBuff [STATOFFSET_NUMBER] = tTeamId;
      datBuff [STATOFFSET_TFINAL] = (oTeamScore << 8) | tTeamScore;
      sPtr->ReadBuff (datBuff);
      AddStat (sPtr, OTGSTAT_MISC, oTeamId);
      if (!htFlag && !poFlag)
         AddStat (sPtr, OTSSTAT_MISC, oTeamId);

      count = sgTeam.count;
      while (count--)
      {
         STATGROUP_HEADER  stat;
         my_fread (&stat, sizeof (STATGROUP_HEADER), 1, fpIn);

         sPtr = 0;
         DYN_16   type;
         stat.id |= pId;

         switch (stat.type)
         {
            case STATGROUP_RUSHING:
               type = PGSTAT_RUSH;
               sPtr = &statRush;
               break;

            case STATGROUP_PASSING:
               type = PGSTAT_PASS;
               sPtr = &statPass;
               break;

            case STATGROUP_RECEIVING:
               type = PGSTAT_PASSREC;
               sPtr = &statPassRec;
               break;

            case STATGROUP_INTERCEPTIONS:
               type = PGSTAT_INT;
               sPtr = &statInt;
               break;

            case STATGROUP_FUMBLES:
               type = PGSTAT_FUMBLE;
               sPtr = &statFumble;
               break;

            case STATGROUP_FUMBLE_RECOVERIES:
               type = PGSTAT_FUMBLEREC;
               sPtr = &statFumbleRec;
               break;

            case STATGROUP_SACKS:
               type = PGSTAT_SACK;
               sPtr = &statSack;
               break;

            case STATGROUP_SAFETIES:
               type = PGSTAT_SAFETY;
               sPtr = &statSafety;
               break;

            case STATGROUP_TACKLES:
               type = PGSTAT_TACKLE;
               sPtr = &statTackle;
               break;

            case STATGROUP_PUNTING:
               type = PGSTAT_PUNT;
               sPtr = &statPunt;
               break;

            case STATGROUP_PUNT_RETURNS:
               type = PGSTAT_PUNTRET;
               sPtr = &statPuntRet;
               break;

            case STATGROUP_KICKOFF_RETURNS:
               type = PGSTAT_KICKRET;
               sPtr = &statKickRet;
               break;

            case STATGROUP_PAT:
               type = PGSTAT_PAT;
               sPtr = &statPAT;
               break;

            case STATGROUP_FIELDGOAL:
               type = PGSTAT_FG;
               sPtr = &statFG;
               break;

            case STATGROUP_PAT2:
               type = PGSTAT_PAT2;
               sPtr = &statPAT2;
               break;

            case STATGROUP_PENALTY:
               type = PGSTAT_PENALTY;
               sPtr = &statPenalty;
               break;

         }

         vmAssert (sPtr);

         if (sPtr)
         {
            // read data in
            sPtr->Load (fpIn, type, stat.id);

            // generate game (player team, other team)
            AddStat (sPtr, type + PGSTAT_PAT, stat.id);
            if ( type != PGSTAT_PENALTY )
				{
	            AddStat (sPtr, type + TGSTAT_PAT, tTeamId);
		         AddStat (sPtr, type + OTGSTAT_PAT, oTeamId);
				}

            // generate season (player team, other team)
            if (!htFlag && !poFlag && (type != PGSTAT_PENALTY))
            {
               AddStat (sPtr, type + PSSTAT_PAT, stat.id);
               AddStat (sPtr, type + TSSTAT_PAT, tTeamId);
               AddStat (sPtr, type + OTSSTAT_PAT, oTeamId);

               // generate career (player)
               AddStat (sPtr, type + PCSTAT_PAT, stat.id);
               // add HOF award points for this single stat
#if (LCDT == 4) || (LCDT == 6)
               AddHOF1(i, sPtr, type, stat.id);  // HOF JWM 9//16/96
#endif
            }
         }

      } // end while (count--)
   } // end for
#if (LCDT == 4) || (LCDT == 6)
   if ( (ActiveLeague != NULL) &&
				!htFlag && !poFlag )
	{
      ConferenceIndex = pTeams[0]->GetCI() ;
      AddHOF2(0) ; // award points for team stats or playing in game
      ConferenceIndex = pTeams[1]->GetCI() ;
      AddHOF2(1) ; // award points for team stats or playing in game

	   CalcPOG() ; // got all the HOF points awarded so calculate Player of the Game
   }
#endif
}
#endif

/*
   ---------------------------------------------------------------------
   NAME:       AddStat

   FUNCTION:   add a statistical record to the database
               a temporary data area is filled because the Statisitics
               are all derived classes. Therefore there are extra bytes
               at the beginning of each object for a virtual function table.
               Passing the address of the entire object will have these
               virtual function info bytes stored as part of the record,
               which in not a desired effect

               Make a key to search for out of the statistic type and
               team/player id. If a record is found, update it, otherwise
               make a new one.

   INPUT:      sPtr-> Statistic to add
               type = type of statistic being added
               id = team/player id

   RETURNS:    successful addition flag
   ---------------------------------------------------------------------
*/

DYN_16
StatFile::AddStat (Statistic *sPtr, DYN_U_16 type, DYN_U_16 id)
{
   DYN_U_8  keyBuff [KEY_LENGTH] =
            {
               type >> 8, type & 0xff,
               id >> 8, id & 0xff
            };
   DYN_U_16 datBuff [REC_BUFF_SIZE];

   // if no record for this type & player combo, make one
   if (GetRecord (IDX_FILE, keyBuff, datBuff))
   {
      #if   1
      sPtr->FillBuff (datBuff, type, id);
      return AddRecord (DAT_FILE, datBuff);

      #else
      sPtr->FillBuff (datBuff, type, id);
      DYN_16   addedOk = AddRecord (DAT_FILE, datBuff);
      GetRecord (IDX_FILE, keyBuff, datBuff);
      return addedOk;

      #endif
   }

   // otherwise update it
   else
   {
      sPtr->UpdateBuff (datBuff);
      return ReWriteRecord (DAT_FILE, datBuff);
   }
}

/*
   ---------------------------------------------------------------------
   NAME:       GetStat

   FUNCTION:   retrieve the given statistical record

   INPUT:      type = type of stat to search for
               id = id of team/player (0 is invalid)
               (optional ptr -> where to put data if found (i.e. a buffer
               of REC_BUFF_SIZE DYN_16s!))

   RETURNS:    -> Generated record if one found
   ---------------------------------------------------------------------
*/

Statistic *
StatFile::GetStat (StatType type, DYN_16 id)
{
   Statistic   *sPtr = 0;

   if (id)
   {
      DYN_8    keyBuff [4];
      DYN_U_16 datBuff [REC_BUFF_SIZE];
      BuildKey (keyBuff, type, id);
      if (!GetRecord (IDX_FILE, keyBuff, datBuff))
      {
         sPtr = NewStatistic (type);
         sPtr->ReadBuff (datBuff);
      }
   }

   return sPtr;
}

DYN_16
StatFile::GetStat (StatType type, DYN_16 id, DYN_U_16 *datBuff)
{
   DYN_16   value = FALSE;

   memset (datBuff, 0, REC_BUFF_SIZE);

   if (id)
   {
      DYN_8    keyBuff [4];
      BuildKey (keyBuff, type, id);

      return !GetRecord (IDX_FILE, keyBuff, datBuff);
   }

   return value;
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildStatList

   FUNCTION:   fill in the list of statistics with all those that match
               the given type and id

   INPUT:      sList-> where to put found stats
               type = type of stat to search for
               id = id of team/player (0 is all that match)

               if no sList, then user just wants to know if ANY stats
               are in database (at least 1)

   RETURNS:    size of list generated
   ---------------------------------------------------------------------
*/

DYN_16
StatFile::BuildStatList (GenList<Statistic> *sList, StatType type, DYN_16 id)
{
   DYN_8    keyBuff [4];
   DYN_16   keyLength = BuildKey (keyBuff, type, id),
            numStats = 0;
   DYN_U_16 datBuff [REC_BUFF_SIZE];

   // search for all possible matches
   if (!FirstInSet (IDX_FILE, keyBuff, datBuff, keyLength))
   {
      do
      {
         numStats++;

         if (sList)
         {
            Statistic   *sPtr = NewStatistic (type);
            sPtr->ReadBuff (datBuff);
            sList->linkin (sPtr);
         }

         else
            break;
      } while (!NextInSet (IDX_FILE, datBuff));
   }

   return numStats;
}

/*
   ---------------------------------------------------------------------
   NAME:       SortStatList

   FUNCTION:   given a list created by the above function,
               sort the list via a shell-sort based on some Statistic info

   INPUT:      sList -> list to sort
               sortType = which data item of statistics to sort on

   RETURNS:    list sorted based on type of sort wanted
   ---------------------------------------------------------------------
*/

void
StatFile::SortStatList (GenList<Statistic> *sList, StatSort sortType)
{
   ListShellSort ((zDList *) sList, sortType, compareFunc);
}

/*
   ---------------------------------------------------------------------
   NAME:       GetStatType

   FUNCTION:   user wants a StatType for a given statistic/timeframe

   INPUT:      statType = base type (i.e. Player Rushing)
               timeFrame = time of record (i.e. Season)

   RETURNS:    the 2 combined (i.e. PSSTAT_RUSH, Player Season Rushing)
   ---------------------------------------------------------------------
*/

StatType
StatFile::GetStatType (StatType inType, StatTimeFrame timeFrame)
{
   StatType retType = STAT_ERR;
   DYN_16   offset;

   // caller wants a PLAYER or TEAM statistic
   if (inType < PCSTAT_MAX)
   {
      retType = PGSTAT_PAT;
      offset = PSTAT_SET_OFFSET;
   }

   // no CAREER stats are kept for teams
   else if (timeFrame < STATTIMEFRAME_CAREER)
   {
      if (inType < TSSTAT_MAX)
         retType = TGSTAT_PAT;

      else if (inType < OTSSTAT_MAX)
         retType = OTGSTAT_PAT;

      inType = StatType(inType - TGSTAT_PAT);
      offset = TSTAT_SET_OFFSET;
   }

   // adjust BASE type (?_PAT) to specific time & category
   if (retType != STAT_ERR)
      retType = StatType(retType + ((offset * timeFrame) + (inType % offset)));

   return retType;
}

/*
   ---------------------------------------------------------------------
   NAME:       Flush

   FUNCTION:   after each week or season is complete, all the records
               in that time frame are to be delete

   INPUT:      sf = GAME/SEASON identifier

   RETURNS:    all PLAYER & TEAM GAME/SEASON records deleted from database
   ---------------------------------------------------------------------
*/

void
StatFile::Flush (StatFlush sf)
{
   StatType type;

   switch (sf)
   {
      case STATFLUSH_WEEK:
         for (type = PGSTAT_PAT; type < PGSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);

         for (type = TGSTAT_PAT; type < TGSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);

         for (type = OTGSTAT_PAT; type < OTGSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);
         break;

      case STATFLUSH_SEASON:
         for (type = PSSTAT_PAT; type < PSSTAT_MAX; type = StatType(type + 1)) {
            if (type == PSSTAT_APTEAM) {
               continue ;
            }
            DeleteStat (type, 0);
         }

         for (type = TSSTAT_PAT; type < TSSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);

         for (type = OTSSTAT_PAT; type < OTSSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);
         break;

      case STATFLUSH_CAREER:
         for (type = PCSTAT_PAT; type < PCSTAT_MAX; type = StatType(type + 1))
            DeleteStat (type, 0);
         break;
   }
}








/*
   =====================================================================
                           PRIVATE METHODS
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       Create

   FUNCTION:   opens data files and indices of IFIL type structure

   INPUT:      pName, fName-> file name string parts
               writeFlag = should data be backed up (i.e. this is
               going to modify the data)

   RETURNS:    FALSE if database Create/Opened ok
               statDat & statIdx filename filled in
               ISAM stuff created
               if failed because of prior existence, just calls open
   ---------------------------------------------------------------------
*/

DYN_16
StatFile::Create (DYN_8 *pName, DYN_8 *fName, DYN_8 writeFlag)
{
   makeFileName (fileNameBuffer, pName, fName, "");

   DYN_16   errCode = CreateIFile (&statDat);
   if (errCode == DOPN_ERR)
      errCode = Open (pName, fName, writeFlag);

   return errCode;
}

/*
   ---------------------------------------------------------------------
   NAME:       Open

   FUNCTION:   opens data files and indices of IFIL structure
               (Create () above failed, so data base must already exist,
               open for update)
               if can't open, flush the files
               (copying from backup doesn't work)

   INPUT:      pName, fName-> file name string parts

   RETURNS:    ISAM ready to roll for this league
   ---------------------------------------------------------------------
*/

//#pragma  argsused

DYN_16
StatFile::Open (DYN_8 *pName, DYN_8 *fName, DYN_8 writeFlag)
{
   DYN_16   errCode = OpenIFile (&statDat);
   if (errCode)
   {
      // if error is not a lack of memory,
      if (errCode != UALC_ERR)
      {
         // flush the files
         unlink (makeFileName (0, pName, fName, datText));
         unlink (makeFileName (0, pName, fName, idxText));

         // and start anew
         errCode = CreateIFile (&statDat);
         if (errCode)
            sysAssert (FALSE, EXITMSG_DATABASE_ERR);
      }
   }

   return errCode;
}

/*
   ---------------------------------------------------------------------
   NAME:       BuildKey

   FUNCTION:   create a key to search with based on the stat type and
               id

   INPUT:      dest-> array to build key into
               type = type of stat to search for
               id = id of team/player (0 means all matching)

   RETURNS:    size of key generated (2 or 4 bytes depending if id
               is 0 or a value)
   ---------------------------------------------------------------------
*/

DYN_16
StatFile::BuildKey (DYN_8 *dest, StatType type, DYN_16 id)
{
   DYN_16   keySize;

   *dest++ = type >> 8;
   *dest++ = type & 0xff;

   if (id)
   {
      *dest++ = id >> 8;
      *dest = id & 0xff;
      keySize = 4;
   }

   else
      keySize = 2;

   return keySize;
}

/*
   ---------------------------------------------------------------------
   NAME:       DeleteStat

   FUNCTION:   delete the given stat from the database

   INPUT:      type = type of stat to search for
               id = id of team/player (0 means all matching)

   RETURNS:    status of deleted record
   ---------------------------------------------------------------------
*/

void
StatFile::DeleteStat (StatType type, DYN_16 id)
{
   DYN_8    keyBuff [4];
   DYN_16   keyLength = BuildKey (keyBuff, type, id),
            datBuff [REC_BUFF_SIZE];

   // search for all possible matches
   if (!FirstInSet (IDX_FILE, keyBuff, datBuff, keyLength))
   {
      do
      {
         DeleteRecord (DAT_FILE);
      } while (!NextInSet (IDX_FILE, datBuff));
   }
}







/*
   =====================================================================
                           LOCAL UTILITIES
   =====================================================================
*/

/*
   ---------------------------------------------------------------------
   NAME:       sumScores

   FUNCTION:   add up the score by quarters, (if the game is ridiculous,
               the arcade reports score as 1 byte, which may be a little
               short)
               clip the result to 7 bits

   INPUT:      ptr-> array of 5 quarter scores

   RETURNS:    sum of the scores
   ---------------------------------------------------------------------
*/

static DYN_8 NEAR
sumScores (DYN_U_8 *ptr)
{
   DYN_16   sum = 0;
   for (DYN_16 i = 0; i < 5; i++, ptr++)
      sum += *ptr;

   if (sum > 127)
      sum = 127;

   return (DYN_8)sum;
}

/*
   ---------------------------------------------------------------------
   NAME:       NewStatistic

   FUNCTION:   create a statistic object of the given type
               (i.e. passing, PAT, etc.)

   INPUT:      type = which kind of statistic to create

   RETURNS:    -> created object
   ---------------------------------------------------------------------
*/

static Statistic * NEAR
NewStatistic (const StatType type)
{
   Statistic   *sPtr = 0;
   switch (type)
   {
      case PGSTAT_PAT:
      case PSSTAT_PAT:
      case PCSTAT_PAT:
      case TGSTAT_PAT:
      case TSSTAT_PAT:
      case OTGSTAT_PAT:
      case OTSSTAT_PAT:
      case PGSTAT_PAT2:
      case PSSTAT_PAT2:
      case PCSTAT_PAT2:
      case TGSTAT_PAT2:
      case TSSTAT_PAT2:
      case OTGSTAT_PAT2:
      case OTSSTAT_PAT2:
         sPtr = (Statistic *) new StatisticPAT ();
         break;

      case PGSTAT_FG:
      case PSSTAT_FG:
      case PCSTAT_FG:
      case TGSTAT_FG:
      case TSSTAT_FG:
      case OTGSTAT_FG:
      case OTSSTAT_FG:
         sPtr = (Statistic *) new StatisticFG ();
         break;

      case PGSTAT_RUSH:
      case PSSTAT_RUSH:
      case PCSTAT_RUSH:
      case TGSTAT_RUSH:
      case TSSTAT_RUSH:
      case OTGSTAT_RUSH:
      case OTSSTAT_RUSH:
         sPtr = (Statistic *) new StatisticRush ();
         break;

      case PGSTAT_PASS:
      case PSSTAT_PASS:
      case PCSTAT_PASS:
      case TGSTAT_PASS:
      case TSSTAT_PASS:
      case OTGSTAT_PASS:
      case OTSSTAT_PASS:
         sPtr = (Statistic *) new StatisticPass ();
         break;

      case PGSTAT_PASSREC:
      case PSSTAT_PASSREC:
      case PCSTAT_PASSREC:
      case TGSTAT_PASSREC:
      case TSSTAT_PASSREC:
      case OTGSTAT_PASSREC:
      case OTSSTAT_PASSREC:
         sPtr = (Statistic *) new StatisticPassRec ();
         break;

      case PGSTAT_INT:
      case PSSTAT_INT:
      case PCSTAT_INT:
      case TGSTAT_INT:
      case TSSTAT_INT:
      case OTGSTAT_INT:
      case OTSSTAT_INT:
         sPtr = (Statistic *) new StatisticInt ();
         break;

      case PGSTAT_PUNT:
      case PSSTAT_PUNT:
      case PCSTAT_PUNT:
      case TGSTAT_PUNT:
      case TSSTAT_PUNT:
      case OTGSTAT_PUNT:
      case OTSSTAT_PUNT:
         sPtr = (Statistic *) new StatisticPunt ();
         break;

      case PGSTAT_PUNTRET:
      case PSSTAT_PUNTRET:
      case PCSTAT_PUNTRET:
      case TGSTAT_PUNTRET:
      case TSSTAT_PUNTRET:
      case OTGSTAT_PUNTRET:
      case OTSSTAT_PUNTRET:
         sPtr = (Statistic *) new StatisticPuntRet ();
         break;

      case PGSTAT_KICKRET:
      case PSSTAT_KICKRET:
      case PCSTAT_KICKRET:
      case TGSTAT_KICKRET:
      case TSSTAT_KICKRET:
      case OTGSTAT_KICKRET:
      case OTSSTAT_KICKRET:
         sPtr = (Statistic *) new StatisticKickRet ();
         break;

      case PGSTAT_FUMBLE:
      case PSSTAT_FUMBLE:
      case PCSTAT_FUMBLE:
      case TGSTAT_FUMBLE:
      case TSSTAT_FUMBLE:
      case OTGSTAT_FUMBLE:
      case OTSSTAT_FUMBLE:
         sPtr = (Statistic *) new StatisticFumble ();
         break;

      case PGSTAT_FUMBLEREC:
      case PSSTAT_FUMBLEREC:
      case PCSTAT_FUMBLEREC:
      case TGSTAT_FUMBLEREC:
      case TSSTAT_FUMBLEREC:
      case OTGSTAT_FUMBLEREC:
      case OTSSTAT_FUMBLEREC:
         sPtr = (Statistic *) new StatisticFumbleRec ();
         break;

      case PGSTAT_SACK:
      case PSSTAT_SACK:
      case PCSTAT_SACK:
      case TGSTAT_SACK:
      case TSSTAT_SACK:
      case OTGSTAT_SACK:
      case OTSSTAT_SACK:
         sPtr = (Statistic *) new StatisticSack ();
         break;

      case PGSTAT_SAFETY:
      case PSSTAT_SAFETY:
      case PCSTAT_SAFETY:
      case TGSTAT_SAFETY:
      case TSSTAT_SAFETY:
      case OTGSTAT_SAFETY:
      case OTSSTAT_SAFETY:
         sPtr = (Statistic *) new StatisticSafety ();
         break;

      case PGSTAT_TACKLE:
      case PSSTAT_TACKLE:
      case PCSTAT_TACKLE:
      case TGSTAT_TACKLE:
      case TSSTAT_TACKLE:
      case OTGSTAT_TACKLE:
      case OTSSTAT_TACKLE:
         sPtr = (Statistic *) new StatisticTackle ();
         break;

      case PGSTAT_PENALTY:   /* HOF JWM 9/10 */
      //case TGSTAT_PENALTY:  // do I need these? I don't think so.
      //case TSSTAT_PENALTY:
      //case OTGSTAT_PENALTY:
      //case OTSSTAT_PENALTY:
         sPtr = (Statistic *) new StatisticPenalty ();
         break;

      case PGSTAT_HOF:   /* HOF JWM 9/11/96 */
      case PSSTAT_HOF:
      case PCSTAT_HOF:
      //case TGSTAT_HOF:  // do I need these? I don't think so.
      //case TSSTAT_HOF:
      //case OTGSTAT_HOF:
      //case OTSSTAT_HOF:
         sPtr = (Statistic *) new StatisticHOF ();
         break;

      case PGSTAT_POG:   /* HOF JWM 9/12/96 */
      case PSSTAT_APTEAM:   /* HOF JWM 9/21/96 */
      case PCSTAT_MVPROY:   /* HOF JWM 9/21/96 */
         sPtr = (Statistic *) new StatisticPOG ();
         break;

      case PCSTAT_HALLOFAME:   /* HOF JWM 9/21/96 */
         sPtr = (Statistic *) new StatisticHALLOFAME ();
         break;
   }

   return sPtr;
}

/*
   ---------------------------------------------------------------------
   NAME:       compareFunc

   FUNCTION:   the function called by the generic shell sort to determine
               if the 2 list elements should be swapped

   INPUT:      iPtr, plusgPtr-> 2 Statistics
               sortType = which sort is happening

   RETURNS:    TRUE/FALSE if 2 teams should be swapped
   ---------------------------------------------------------------------
*/

static DYN_16
compareFunc (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType)
{
   DYN_16      doSwap;
   Statistic   *sIPtr = (Statistic *) iPtr,
               *sPPtr = (Statistic *) plusgPtr;

   // sort based on single category?
   if (sortType < STATSORT_RR)
      doSwap = sIPtr->GetData (sortType) < sPPtr->GetData (sortType);

   // or several for tie-breakers
   else
   {
      DYN_16   ydsI = sIPtr->GetData (STATSORT_YARDS),
               tdsI = sIPtr->GetData (STATSORT_TDS),
               ydsP = sPPtr->GetData (STATSORT_YARDS),
               tdsP = sPPtr->GetData (STATSORT_TDS);

      // first 2 decisions are yards and touchdowns
      if (ydsI != ydsP)
         doSwap = ydsI < ydsP;

      else if (tdsI != tdsP)
         doSwap = tdsI < tdsP;

      // the next 2 depend on category
      else
      {
         DYN_16   numI = sIPtr->GetData (STATSORT_NUMBER),
                  numP = sPPtr->GetData (STATSORT_NUMBER);
         if (sortType == STATSORT_RR)
            doSwap = numI < numP;

         else
         {
            DYN_16   compI = sIPtr->GetData (STATSORT_COMPS),
                     compP = sIPtr->GetData (STATSORT_COMPS);
            if (compI != compP)
               doSwap = compI < compP;
            else
               doSwap = numI < numP;
         }
      }
   }

   return doSwap;
}
/* HOF JWM 9/20
   ---------------------------------------------------------------------
   NAME:       compareFuncSigned

   FUNCTION:   the function called by the generic shell sort to determine
               if the 2 list elements should be swapped

   INPUT:      iPtr, plusgPtr-> 2 Statistics
               sortType = which sort is happening

   RETURNS:    TRUE/FALSE if 2 teams should be swapped

  quick hack to make the point count field the sort criteria
   ---------------------------------------------------------------------
*/

static DYN_16
compareFuncSigned (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType)
{
   DYN_16      doSwap;
   StatisticHOF   *sIPtr = (StatisticHOF *) iPtr,
               *sPPtr = (StatisticHOF *) plusgPtr;

   // sort based on single category?
   doSwap = sIPtr->data.pointCount < sPPtr->data.pointCount;


   return doSwap;
}

/*
   ---------------------------------------------------------------------
   NAME:       copyFile

   FUNCTION:   make a backup of one of the database files

   INPUT:      pName, fName-> filenames for this league
               in, out-> extension strings for source, destination

   RETURNS:    file duplicated if possible
   ---------------------------------------------------------------------
*/

static DYN_16 NEAR
copyFile (DYN_8 *pName, DYN_8 *fName, DYN_8 *inExt, DYN_8 *outExt)
{
   FILE     *sFP = my_fopen (makeFileName (0, pName, fName, inExt), "rb");
   DYN_16   copied = FALSE;

   // if an old file existed, copy it
   if (sFP)
   {
      FILE  *dFP = my_fopen (makeFileName (0, pName, fName, outExt), "wb");

      if (dFP)
      {
         copied = copyFile (dFP, sFP);
         my_fclose (dFP);
      }

      my_fclose (sFP);
   }

   return copied;
}

#if   (LCDT == 4) || (LCDT == 6)

#define ISDEFENSIVE(x) ((x == RosPlayer::POS_LB) || (x == RosPlayer::POS_DB) || (x == RosPlayer::POS_DL))



void StatFile::CalcPOG()  // HOF JWM 9/14/96 calcultate the player of the game
{
   StatisticHOF *stat ;
   DWORD numBytes ;
   Roster      *pRoster ;

   if (ActiveLeague == NULL) {
      return ;
   }

   RosPlayer   *playerPtr ;
   DYN_16 curweek = ActiveLeague->GetSchedule ()->GetWData(SCDWEEKDATA_CUR_WEEK,0) ;

   if (logFile) {
      hfLog = CreateFile("awards.log",GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL) ;
      if (hfLog == INVALID_HANDLE_VALUE) {
         logFile = 0 ;
      }
      else {
         SetFilePointer(hfLog,0,NULL,FILE_END) ;
         wsprintf(str,"award points for week = %d, game = %d\n",curweek,gameNumber) ;
         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
      }

   }
   int playerId=0,
       playerIdWithMost=0,
       playerIdWithMostTeamId=0,
       playerIdWithMostOppTeamId=0,
       mostPoints=0 ;

   for (int teamIndex = 0 ; teamIndex < 2 ; teamIndex++) {
      if (logFile) {
         pRoster = ActiveLeague->LoadPlayers (pTeams[teamIndex]) ;
         wsprintf(str,"\n   %s\n\n",teamNames[teamIndex]) ;
         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
      }
      for (int lcv = 0 ; lcv < sizeof(playersInGame[0])/sizeof(playersInGame[0][0]) ; lcv++) {
         playerId = playersInGame[teamIndex][lcv] ;
         if (playerId == 0) { // end of list
            break ;
         }
         stat = (StatisticHOF *) GetStat (PGSTAT_HOF, playerId) ;
         if (stat == NULL) {
            continue ;
         }

         if (logFile) {
            playerPtr = pRoster->GetPlayer (playerId) ;
/*            wsprintf(str,
                     "%s %s %s %d\n",
                     playerPtr->GetFirstName (),
                     playerPtr->GetLastName (),
                     posText[playerPtr->GetPosSpec()] ,
                     stat->data.pointCount
                    ) ;*/
            wsprintf(str,
                     "%s %s                                    \n",
                     playerPtr->GetFirstName (),
                     playerPtr->GetLastName ()
                    ) ;
            wsprintf(&str[40],
                     "%2s   %3d %1d\n",
                     posText[playerPtr->GetPosSpec()] ,
                     stat->data.pointCount,
                     stat->data.confIndex
                    ) ;
            WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
         }

         if ((DYN_16) stat->data.pointCount > mostPoints) {
            mostPoints = stat->data.pointCount ;
            playerIdWithMost = playerId ;
            playerIdWithMostTeamId = teamIds[teamIndex] ;
            playerIdWithMostOppTeamId = teamIds[!teamIndex] ;
         } // Does this deal with ties? Of course it does, by ignoring them!
      }
   }

   if (logFile) {
      wsprintf(str,"\n\n\n") ;
      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
   }

   if (playerIdWithMost == 0 ) {
      goto abort ;
   }

   // make a POG stat
   statPOG.Statistic::data.number = mostPoints + av.PlayerOfTheGameBonus ;
   statPOG.data.playerId = playerIdWithMost ;
   statPOG.data.teamId = playerIdWithMostTeamId ;
   statPOG.data.opponentTeamId = playerIdWithMostOppTeamId ;
   AddStat (&statPOG, PGSTAT_POG, gameNumber+1);

   // add in POG bonus
   stat = (StatisticHOF *) GetStat (PGSTAT_HOF, playerIdWithMost) ;
   stat->data.pointCount = av.PlayerOfTheGameBonus ;
   AddHOFGSC (stat, playerIdWithMost,0xffff);

abort:
   CalcPOW() ;
   if (logFile) {
      CloseHandle(hfLog) ;
   }
} // void StatFile::CalcPOG()


/* copy player of the game with the most points to player of the week
   and give him his POW bonus.
*/
void StatFile::CalcPOW()  // HOF JWM 9/14/96 calcultate the player of the week
{
   RosPlayer   *playerPtr ;
   DWORD numBytes ;


   if (ActiveLeague == NULL) {
      return ;
   }
	Schedule *psCurSched = ActiveLeague->GetSchedule();

   DYN_16 curweek = psCurSched->GetWData(SCDWEEKDATA_CUR_WEEK,0) ;
   DYN_16 lastGameInWeek = psCurSched->GetWData(SCDWEEKDATA_NUM_GAMES,(DYN_8) curweek) ;
//lastGameInWeek = 1 ;
//   if (gameNumber != lastGameInWeek-1) {	// cannot assume that last game listed
//      return ;										// is the last game played in the week!
//   }
	DYN_16 numGamesPlayed = psCurSched->GamesDoneThisWeek();
	if ( numGamesPlayed != (lastGameInWeek-1) )
	{
		return;
	}

   if (logFile) {
      wsprintf(str,"*** players of game for week = %d ***\n\n",curweek) ;
      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
      ActiveLeague->LoadPlayers() ;
   }

   StatisticPOG *stat ;
   int playerOfWeek = 0 ;
   int playerWithMost ;
   int mostPoints=0 ;
   for (int gamenum = 0 ; gamenum < lastGameInWeek ; gamenum++) {
      stat = (StatisticPOG *) GetStat (PGSTAT_POG, gamenum+1) ;
      if (stat == NULL) {
         continue ;
      }
      if (logFile) {
         playerPtr = ActiveLeague->FindPlayer(stat->data.playerId,1) ;
			if ( playerPtr != NULL )
			{
	         wsprintf(str,"    player of game #%2d %s %s                                    ",
	                     gamenum,
		                  playerPtr->GetFirstName (),
		                    playerPtr->GetLastName ()
			           ) ;
	         wsprintf(&str[40],
	                  "%2s   %3d\n",
	                  posText[playerPtr->GetPosSpec()] ,
	                  stat->Statistic::data.number
	                 ) ;
	         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
			}
      }
      if (stat->Statistic::data.number > mostPoints) {
         mostPoints = stat->Statistic::data.number ;
         playerWithMost = gamenum+1 ;
      }
   }
   // found highest POG make him POW by making the stat id == 99
   stat = (StatisticPOG *) GetStat (PGSTAT_POG, playerWithMost) ;
   if (stat == NULL) {
      return ;
   }
   stat->Statistic::data.id = 99 ;
   stat->Statistic::data.number += av.PlayerOfTheWeekBonus ;
   AddStat (stat, PGSTAT_POG, 99);
   if (logFile) {
      playerPtr = ActiveLeague->FindPlayer(stat->data.playerId,1) ;
		if ( playerPtr != NULL )
		{
	      wsprintf(str,"\n*** player of week #%2d %s %s                                   ",
	                  curweek,
	                  playerPtr->GetFirstName (),
	                  playerPtr->GetLastName ()
	              ) ;
	      wsprintf(&str[40],
	               "%2s   %3d\n\n\n",
	               posText[playerPtr->GetPosSpec()] ,
	               stat->Statistic::data.number
	              ) ;
	      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
	      memset(str,'-',80) ;
	      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
	      memset(str,'\0',sizeof(str)) ;
		}
   }

   int playerId = stat->data.playerId ;

   // give him his POW bonus
   StatisticHOF *statHOF = (StatisticHOF *) GetStat (PGSTAT_HOF, playerId) ;
   statHOF->data.pointCount = av.PlayerOfTheWeekBonus ;
   AddHOFGSC (statHOF, playerId,0xffff);
   CalcPOY() ;
} // void StatFile::CalcPOW()

void
StatFile::CalcPOY()
{
   DWORD numBytes ;
   int pos ;
   int lcv ;

   if (ActiveLeague == NULL) {
      return ;
   }
   DYN_16 curweek = ActiveLeague->GetSchedule ()->GetWData(SCDWEEKDATA_CUR_WEEK,0) ;
   DYN_16 lastWeekInYear= ActiveLeague->GetSchedule ()->GetWData(SCDWEEKDATA_NUM_REGWEEKS,0) ;
   if (curweek != lastWeekInYear-1) {
      return ;
   }

   DYN_8 confCount = ActiveLeague->GetNumSub () ;
//   DYN_16 confIndex ;
   StatisticPOG newAward ;
   DYN_16 year = ActiveLeague->GetInception () + ActiveLeague->GetNumSeasons () ;
   year = ((year%100) << 8) ; // year will be the high byte of stat.id

   RosPlayer * player ;
   StatisticHOF *MVP   = NULL ; // most valued player
   StatisticHOF *MVPo1 = NULL ; // most valued offensive player conference 1
   StatisticHOF *MVPo2 = NULL ; // most valued offensive player conference 2
   StatisticHOF *MVPd1 = NULL ; // most valued defensive player conference 3
   StatisticHOF *MVPd2 = NULL ; // most valued defensive player conference 4
   if (confCount == 1) {
      MVPo2 = MVPd2 = (StatisticHOF *) -1 ;
   }

   GenList<Statistic> statList ;
   DYN_16 statCount = BuildStatList(&statList, PSSTAT_HOF, 0) ;

   ListShellSort ((zDList *) &statList, STATSORT_NUMBER, compareFuncSigned);

   StatisticHOF *statHOFptr ;

/**************************************
 find the Most Valued Player and the
 Most Valued Offensive and Defensive players for each conference
 *************************************/
   ActiveLeague->LoadPlayers() ;
   statList.start() ;
   MVP  = (StatisticHOF *) statList.get() ; // top of list is MVP
   player = ActiveLeague->FindPlayer(MVP->Statistic::data.id, 1) ;
	if ( player != NULL )
	{
	   if (ISDEFENSIVE(player->GetPosGen ())) {
	      if (MVP->data.confIndex == 0) {
	         MVPd1 = MVP ;
	      }
	      else {
	         MVPd2 = MVP ;
	      }
	   }
	   else {
	      if (MVP->data.confIndex == 0) {
	         MVPo1 = MVP ;
	      }
	      else {
	         MVPo2 = MVP ;
	      }
	   }
	}

   while (!MVPo1 || !MVPo2 || !MVPd1 || !MVPd2) {
      if ( statList.fwd() == DLIST_ATEND )
		{
			break;
		}

      statHOFptr = (StatisticHOF *) statList.get() ;
      player = ActiveLeague->FindPlayer(statHOFptr->Statistic::data.id, 1) ;
		if ( player != NULL )
		{
	      if (ISDEFENSIVE(player->GetPosGen ())) {
	         if ((MVPd1 == NULL) && (statHOFptr->data.confIndex == 0)) {
	            MVPd1 = statHOFptr ;
	         }
	         else if ((MVPd2 == NULL) && (statHOFptr->data.confIndex == 1)) {
	            MVPd2 = statHOFptr ;
	         }
	      }
	      else {
	         if ((MVPo1 == NULL) && (statHOFptr->data.confIndex == 0)) {
	            MVPo1 = statHOFptr ;
	         }
	         else if ((MVPo2 == NULL) && (statHOFptr->data.confIndex == 1)) {
	            MVPo2 = statHOFptr ;
	         }
	      }
		}
   }


/*****************************************
   calculate Rookie of the Year
*****************************************/
   int years ;
   StatisticHOF *ROY = NULL;
   statList.start() ;
   for (lcv = 0 ; lcv < statCount ; lcv++, statList.fwd()) {
      statHOFptr = (StatisticHOF *) statList.get() ;
      player = ActiveLeague->FindPlayer(statHOFptr->Statistic::data.id, 1) ;
		if ( player != NULL )
		{
	      years = player->GetYears() ;
	      if (years < 1) {		// 0 is the first year for a player
	         ROY = statHOFptr ;
	         if (years == 1) {
	            // check to see if he was ROY last year
	            // continue ;
	         }
	         break ;
			}
      }
   }

/*****************************************
   calculate All Pro Team
*****************************************/

   // first delete last year
   DeleteStat (PSSTAT_APTEAM, 0);

   StatisticHOF *AllProTeam[2][RosPlayer::POS_MAX_SPEC][5] ;
   int guysPerPos[RosPlayer::POS_MAX_SPEC] = {QB_COUNT,FB_COUNT,HB_COUNT,TE_COUNT,WR_COUNT,
                                               C_COUNT, G_COUNT, T_COUNT,DE_COUNT,DT_COUNT,
                                              LB_COUNT,CB_COUNT, S_COUNT, K_COUNT, P_COUNT} ;
//   int guysPerPos[RosPlayer::POS_MAX_SPEC] = {2,1,2,2,3,2,2,2, 2,2,5,3,4, 1,1} ;
   memset(AllProTeam,0,sizeof(AllProTeam)) ;



//   RosPlayer::Pos posid ;
   for (pos = RosPlayer::POS_QB ; pos < RosPlayer::POS_MAX_SPEC ; pos += 1) {
      for (int conf = 0 ; conf < confCount ; conf++) {
         statList.start() ;
         for (int pick = 0 ; pick < guysPerPos[pos] ; pick++) {
            do {
               statHOFptr = (StatisticHOF *) statList.get() ;
               if (statHOFptr == NULL) {
                  continue ;
               }
               player = ActiveLeague->FindPlayer(statHOFptr->Statistic::data.id, 1) ;
               if ( statList.fwd() == DLIST_ATEND )
					{
						 // -- at the end of the list
						break;
					}
					if ( player == NULL )
					{
						continue;
					}
            } while ( (player->GetPosSpec () != pos) || (statHOFptr->data.confIndex != conf) );

            if (statHOFptr) {
char str[256] ;
sprintf(str,"id %d %s %s %d",
            statHOFptr->Statistic::data.id,
            player->GetFirstName(),
            player->GetLastName(),
            player->GetPosSpec ()
       ) ;
               AllProTeam[conf][pos][pick] = statHOFptr ;
            }
         }
      }
   }
   // record All Pro Teams and give career award points
   for (pos = RosPlayer::POS_QB ; pos < RosPlayer::POS_MAX_SPEC ; pos += 1) {
      for (int conf = 0 ; conf < confCount ; conf++) {
         // 0 pick
			if ( AllProTeam[conf][pos][0] != NULL )
			{
	         newAward.data.playerId = AllProTeam[conf][pos][0]->Statistic::data.id ;
	         AddStat ( &newAward,
	                   PSSTAT_APTEAM,
	                   pos+1 | conf<<4 | 0x000
	                 ) ;
	         AllProTeam[conf][pos][0]->data.pointCount = av.ConsensusAllPro ;
	         AddStat ((Statistic *) AllProTeam[conf][pos][0], PCSTAT_HOF, AllProTeam[conf][pos][0]->Statistic::data.id) ;
			}
         // 1 pick
         if (AllProTeam[conf][pos][1]) {
            newAward.data.playerId = AllProTeam[conf][pos][1]->Statistic::data.id ;
            AddStat ( &newAward,
                      PSSTAT_APTEAM,
                      pos+1 | conf<<4 | 0x100
                    ) ;
            AllProTeam[conf][pos][1]->data.pointCount = av.FirstTeamAllPro ;
            AddStat ((Statistic *) AllProTeam[conf][pos][1], PCSTAT_HOF, AllProTeam[conf][pos][1]->Statistic::data.id) ;
         }
         // 2 pick
         if (AllProTeam[conf][pos][2]) {
            newAward.data.playerId = AllProTeam[conf][pos][2]->Statistic::data.id ;
            AddStat ( &newAward,
                      PSSTAT_APTEAM,
                      pos+1 | conf<<4 | 0x200
                    ) ;
            AllProTeam[conf][pos][2]->data.pointCount = av.SecondTeamAllPro ;
            AddStat ((Statistic *) AllProTeam[conf][pos][2], PCSTAT_HOF, AllProTeam[conf][pos][2]->Statistic::data.id) ;
         }
         // 3 pick
         if (AllProTeam[conf][pos][3]) {
            newAward.data.playerId = AllProTeam[conf][pos][3]->Statistic::data.id ;
            AddStat ( &newAward,
                      PSSTAT_APTEAM,
                      pos+1 | conf<<4 | 0x300
                    ) ;
            AllProTeam[conf][pos][3]->data.pointCount = av.SecondTeamAllPro ;
            AddStat ((Statistic *) AllProTeam[conf][pos][3], PCSTAT_HOF, AllProTeam[conf][pos][3]->Statistic::data.id) ;
         }
         // 4 pick
         if (AllProTeam[conf][pos][4]) {
            newAward.data.playerId = AllProTeam[conf][pos][4]->Statistic::data.id ;
            AddStat ( &newAward,
                      PSSTAT_APTEAM,
                      pos+1 | conf<<4 | 0x400
                    ) ;
            AllProTeam[conf][pos][4]->data.pointCount = av.SecondTeamAllPro ;
            AddStat ((Statistic *) AllProTeam[conf][pos][4], PCSTAT_HOF, AllProTeam[conf][pos][4]->Statistic::data.id) ;
         }
      }
   }

   // record Rookie of the Year and give career award points
   newAward.data.playerId        = ROY->Statistic::data.id ;
   AddStat (&newAward, PCSTAT_MVPROY, 99 | year) ;
   // now add award points
   ROY->data.pointCount = av.RookieOfYear ;
   AddStat ((Statistic *) ROY, PCSTAT_HOF, ROY->Statistic::data.id) ;

   // record MVP and give career award points
   newAward.data.playerId        = MVP->Statistic::data.id ;
   AddStat (&newAward, PCSTAT_MVPROY, 90 | year) ;
   // now add award points
   MVP->data.pointCount = av.MostValuablePlayer ;
   AddStat ((Statistic *) MVP, PCSTAT_HOF, MVP->Statistic::data.id) ;

   // record MVPo1 and give career award points
   newAward.data.playerId        = MVPo1->Statistic::data.id ;
   AddStat (&newAward, PCSTAT_MVPROY, 91 | year) ;
   // now add award points
   MVPo1->data.pointCount = av.OffensivePlayerOfYear ;
   AddStat ((Statistic *) MVPo1, PCSTAT_HOF, MVPo1->Statistic::data.id) ;

   // record MVPd1 and give career award points
   newAward.data.playerId        = MVPd1->Statistic::data.id ;
   AddStat (&newAward, PCSTAT_MVPROY, 92 | year) ;
   // now add award points
   MVPd1->data.pointCount = av.DefensivePlayerOfYear ;
   AddStat ((Statistic *) MVPd1, PCSTAT_HOF, MVPd1->Statistic::data.id) ;

   if (confCount == 2) {
      // record MVPo2 and give career award points
      newAward.data.playerId        = MVPo2->Statistic::data.id ;
      AddStat (&newAward, PCSTAT_MVPROY, 93 | year) ;
      // now add award points
      MVPo2->data.pointCount = av.OffensivePlayerOfYear ;
      AddStat ((Statistic *) MVPo2, PCSTAT_HOF, MVPo2->Statistic::data.id) ;

      // record MVPd2 and give career award points
      newAward.data.playerId        = MVPd2->Statistic::data.id ;
      AddStat (&newAward, PCSTAT_MVPROY, 94 | year) ;
      // now add award points
      MVPd2->data.pointCount = av.DefensivePlayerOfYear ;
      AddStat ((Statistic *) MVPd2, PCSTAT_HOF, MVPd2->Statistic::data.id) ;
   }
   if (logFile) {
      StatisticPOG *awardStat ;
      wsprintf(str,"\n\n********* ANNUAL AWARDS FOR = %d ********\n\n",ActiveLeague->GetInception () + ActiveLeague->GetNumSeasons ()) ;
      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;

      awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 90 | year) ;
      MVP = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
      player = ActiveLeague->FindPlayer(MVP->Statistic::data.id, 1) ;
		if ( player != NULL )
		{
	      wsprintf(str,"    MVP %s %s                                       ",
	                  player->GetFirstName (),
	                  player->GetLastName ()
	              ) ;
	      wsprintf(&str[40],
	               "%2s   %3d\n",
	               posText[player->GetPosSpec()] ,
	               MVP->data.pointCount
	              ) ;
	      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
		}

      awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 91 | year) ;
      MVPo1 = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
      player = ActiveLeague->FindPlayer(MVPo1->Statistic::data.id, 1) ;
		if ( player != NULL )
		{
	      wsprintf(str,"    MVPo1 %s %s                                     ",
	                  player->GetFirstName (),
	                  player->GetLastName ()
	              ) ;
	      wsprintf(&str[40],
	               "%2s   %3d\n",
	               posText[player->GetPosSpec()] ,
	               MVPo1->data.pointCount
	              ) ;
	      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
		}

      awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 92 | year) ;
      MVPd1 = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
      player = ActiveLeague->FindPlayer(MVPd1->Statistic::data.id, 1) ;
		if ( player != NULL )
		{
	      wsprintf(str,"    MVPD1 %s %s                                     ",
	                  player->GetFirstName (),
	                  player->GetLastName ()
	              ) ;
	      wsprintf(&str[40],
	               "%2s   %3d\n",
	               posText[player->GetPosSpec()] ,
	               MVPd1->data.pointCount
	              ) ;
	      WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
		}

      if (confCount == 2) {
         awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 93 | year) ;
         MVPo2 = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
         player = ActiveLeague->FindPlayer(MVPo2->Statistic::data.id, 1) ;
			if ( player != NULL )
			{
	         wsprintf(str,"    MVPo2 %s %s                                    ",
	                     player->GetFirstName (),
	                     player->GetLastName ()
	                 ) ;
	         wsprintf(&str[40],
	                  "%2s   %3d\n",
	                  posText[player->GetPosSpec()] ,
	                  MVPo2->data.pointCount
	                 ) ;
	         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
			}

         awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 94 | year) ;
         MVPd2 = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
         player = ActiveLeague->FindPlayer(MVPd2->Statistic::data.id, 1) ;
			if ( player != NULL )
			{
	         wsprintf(str,"    MVPD2 %s %s                                     ",
	                     player->GetFirstName (),
	                     player->GetLastName ()
	                 ) ;
	         wsprintf(&str[40],
	                  "%2s   %3d\n",
	                  posText[player->GetPosSpec()] ,
	                  MVPd2->data.pointCount
	                 ) ;
	         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
			}

         awardStat = (StatisticPOG*) GetStat(PCSTAT_MVPROY, 99 | year) ;
         ROY = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
         player = ActiveLeague->FindPlayer(ROY->Statistic::data.id, 1) ;
			if ( player != NULL )
			{
	         wsprintf(str,"    ROY %s %s                                     ",
	                     player->GetFirstName (),
	                     player->GetLastName ()
	                 ) ;
	         wsprintf(&str[40],
	                  "%2s   %3d\n",
	                  posText[player->GetPosSpec()] ,
	                  ROY->data.pointCount
	                 ) ;
	         WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
			}

         for (int conf = 0 ; conf < confCount ; conf++) {
            wsprintf(str,"\n    APT for conf = %d\n",conf) ;
            WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
            for (pos = RosPlayer::POS_QB ; pos < RosPlayer::POS_MAX_SPEC ; pos += 1) {
               for (int pick = 0 ; pick < guysPerPos[pos] ; pick++) {
                  awardStat = (StatisticPOG*) GetStat(PSSTAT_APTEAM,pos+1 | conf<<4 | pick<<8) ;

                  statHOFptr = (StatisticHOF*) GetStat(PSSTAT_HOF, awardStat->data.playerId) ;
	               player = ActiveLeague->FindPlayer(statHOFptr->Statistic::data.id, 1) ;
						if ( player != NULL )
						{
	                  wsprintf(str,"    APT %s %s                                     ",
	                              player->GetFirstName (),
	                              player->GetLastName ()
	                          ) ;
	                  wsprintf(&str[40],
	                           "%2s   %3d\n",
	                           posText[player->GetPosSpec()] ,
	                           statHOFptr->data.pointCount
	                          ) ;
	                  WriteFile(hfLog,str,strlen(str),&numBytes,NULL) ;
						}
               }
            }
         }

      }
   } // end if (logFile)

   // coach of the year
	GenList<Team>			teamList ;
   int numteams = ActiveLeague->BuildTeamList ( &teamList) ;
   teamList.start() ;
   Team *tPtr, *pTeamWithMost ;
   char *pTeamName, *pCoachName ;
   StatisticRush *statRush ;
   StatisticPass *statPass ;
   int tdsMade, tdsAllowed ;
   int pointsMade, pointsAllowed ;
   int wins, losses ;
   int mostpoints = 0 ;

   ActiveLeague->GenerateRecords(TRUE) ;
   for (int count = 0 ; count < numteams ; count++ ) {
      int gotRookie = FALSE, gotMVP  = FALSE, gotMVPo = FALSE, gotMVPd = FALSE ;
      int points ;

      tPtr = (Team *) teamList.get() ;
      wins = tPtr->GetWins ();
      losses = tPtr->GetLosses ();
      pointsMade = tPtr->GetPF ();
      pointsAllowed = tPtr->GetPA ();

      statRush = (StatisticRush *) GetStat (TSSTAT_RUSH, tPtr->GetId() ) ;
      tdsMade = statRush->StatisticSum::data.tds ;
      delete statRush ;

      statPass = (StatisticPass *) GetStat (TSSTAT_PASS, tPtr->GetId() ) ;
      tdsMade += statPass->StatisticSum::data.tds ;
      delete statRush ;

      statRush = (StatisticRush *) GetStat (OTSSTAT_RUSH, tPtr->GetId() ) ;
      tdsAllowed = statRush->StatisticSum::data.tds ;
      delete statRush ;

      statPass = (StatisticPass *) GetStat (OTSSTAT_PASS, tPtr->GetId() ) ;
      tdsAllowed += statPass->StatisticSum::data.tds ;
      delete statRush ;

      if ( tPtr->GetPlayer(MVP->Statistic::data.id) ) {
         gotMVP = TRUE ;
      }
      if ( tPtr->GetPlayer(MVPo1->Statistic::data.id) ||
           ( (MVPo2 != (StatisticHOF *) -1) && tPtr->GetPlayer(MVPo2->Statistic::data.id) )
         )
      {
         gotMVPo = TRUE ;
      }
      if ( tPtr->GetPlayer(MVPd1->Statistic::data.id) ||
           ( (MVPd2 != (StatisticHOF *) -1) && tPtr->GetPlayer(MVPd2->Statistic::data.id) )
         )
      {
         gotMVPd = TRUE ;
      }
      if ( tPtr->GetPlayer(ROY->Statistic::data.id) ) {
         gotRookie = TRUE ;
      }
      points = 0 ;
      points += gotMVP    ? av.coMostValuablePlayer    : 0 ;
      points += gotMVPo   ? av.coOffensivePlayerOfYear : 0 ;
      points += gotMVPd   ? av.coDefensivePlayerOfYear : 0 ;
      points += gotRookie ? av.coRookieOfYear          : 0 ;

      points += wins * av.coTeamWin ;
      points += losses * av.coTeamLoss ;
      points += pointsMade * av.coPointsScored ;
      points += pointsAllowed * av.coPointsAllowed ;
      points += tdsMade * av.coTouchdownsScored ;
      points += tdsAllowed * av.coTouchdownsAllowed ;
      if (points > mostpoints) {
         mostpoints = points ;
         pTeamWithMost = tPtr ;
      }

      // now write out a stat to the data base
      // The coach award is really totally based on the team.  The team is problematic because
      // the name can change and the name of the coach can change.  To use the existing stat data
      // base we can not add large strings so the coach of the year can only be displayed for the
      // year just completed.  So use a PSSTAT_APTEAM type stat it is a season stat and will get
      // deleted next year just like the all pro team.  Handle the coach of the year just like another
      // APT member, give him an ID of 1000h.
      newAward.data.playerId = pTeamWithMost->GetId() ;
      AddStat ( &newAward,
                PSSTAT_APTEAM,
                0x1000
              ) ;
      pTeamName  = pTeamWithMost->GetName() ;
      pCoachName = pTeamWithMost->GetCoach() ;


      teamList.fwd() ;
   } // end for (int count = 0 ; count < numteams ; count++ )

   CalcHALLOFAME() ;
} // end StatFile::CalcPOY()
#endif


// HOF JWM 9/12/96
// AddHOF1 add hall of fame points based on each personal stat that occurs
void
StatFile::AddHOF1 (DYN_8 teamIndex, Statistic *pStat, DYN_U_16 type, DYN_U_16 id)
{
   RosPlayer::Pos genPos  ;
   RosPlayer::Pos SpecPos ;
   StatType statType = (StatType) type ;
   StatisticHOF statHOF ; // local statHOF global also exists

   statHOF.Statistic::data.number = 0 ; // init to collect sums later
   statHOF.data.pointCount = 0 ;
   statHOF.data.confIndex = ConferenceIndex ;

   if (type <= PGSTAT_MAX) {
      Team *pTeam =  pTeams[teamIndex] ;
      char infostr [500] ;

      Roster      *pRoster = ActiveLeague->LoadPlayers (pTeam) ;
      RosPlayer   *playerPtr = pRoster->GetPlayer (id) ;

      strcpy(infostr,playerPtr->GetFirstName()) ;
      strcat(infostr," ") ;
      strcat(infostr,playerPtr->GetLastName()) ;

      genPos  = playerPtr->GetPosGen () ;
      SpecPos = playerPtr->GetPosSpec () ;
      // record the fact that this player has played in the game
      // played in the game is defined as one who has generated at least one stat
      for (int lcv = 0 ; lcv < sizeof(playersInGame[0])/sizeof(playersInGame[0][0]) ; lcv++) {
         if (playersInGame[teamIndex][lcv] == 0) { // end of list so add him
            playersInGame[teamIndex][lcv] = id ;
            break ;
         }
         if (playersInGame[teamIndex][lcv] == id) { // already in list
            break ;
         }
      }
   }
   // if QB track how many stats to see which QB played the most
   if (genPos == RosPlayer::POS_QB) {
      statHOF.Statistic::data.number = pStat->data.number ;
   }

   switch (type) {
      case PGSTAT_PASS: {
         StatisticPass *pStatPass = (StatisticPass *) pStat ;

         // points for touchdown pass
         int count = pStatPass->StatisticSum::data.tds ; // how many touchdowns
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 6 * av.kPointsScored ;
         }

         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount += av.qbPassingTouchdown * count ;
         }

         // 2 pts for each 5 percentage points of completion percentage above 50
         if (genPos == RosPlayer::POS_QB) {
            int percentCompleted = 0;
				if ( pStatPass->Statistic::data.number > 0 )
				{
					percentCompleted =
	               (pStatPass->data.comps * 100) / (pStatPass->Statistic::data.number) ;
				}

            if (percentCompleted > av.qbCompletionPercentageThreshhold) {
					if ( av.qbCompletionPercentageStepValue > 0 )
					{
						int awardPoints = ( percentCompleted -
                                   av.qbCompletionPercentageThreshhold
                                 ) / av.qbCompletionPercentageStepValue ;
		            awardPoints *= av.qbCompletionPercentage ;
	               statHOF.data.pointCount += awardPoints ;
					}
            }
         }

         //  passing yardage
         if (genPos == RosPlayer::POS_QB) {
            int yardsCompleted =(pStatPass->StatisticSum::data.total) ;
            if (yardsCompleted > av.qbPassingYardsThreshhold) {
					if ( av.qbPassingYardsStepValue > 0 )
					{
	               int awardPoints = ( yardsCompleted -
                                   av.qbCompletionPercentageThreshhold
                                 ) / av.qbPassingYardsStepValue ;
		            awardPoints *= av.qbPassingYards ;
			         statHOF.data.pointCount += awardPoints ;
					}
            }
         }

         // negative for being sacked
         count = pStatPass->data.sacks ;
         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount += av.qbSacked * count ;
         }

         // negative for being intercepted
         count = pStatPass->data.ints ;
         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount += av.qbInterception * count ;
         }
      } break ;

      case PGSTAT_RUSH: {
         StatisticRush *pStatRush = (StatisticRush *) pStat ;

         // points for rushing touchdown
         int count = pStatRush->StatisticSum::data.tds ; // how many touchdowns
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.recPointsScored ;

            int totalYards = pStatRush->data.total ;
            int averageYards ;
            if (pStatRush->Statistic::data.number) {
               averageYards = totalYards / pStatRush->Statistic::data.number ;
            }
            else {
               averageYards = 0 ;
            }

            // average rush yardage
            if ( (averageYards > av.recYardsPerCarryThreshhold) &&
							(av.recYardsPerCarryStepValue > 0) ) {
               int awardPoints = (averageYards - av.recYardsPerCarryThreshhold) /
                                   av.recYardsPerCarryStepValue ;
               awardPoints *= av.recYardsPerCarry ;
               statHOF.data.pointCount += awardPoints ;
            }
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 6 * av.kPointsScored ;
         }

         if ( genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount += av.qbRushingTouchdown * count ;
         }
         else if (genPos == RosPlayer::POS_RB) {
            statHOF.data.pointCount += av.rbRushingTouchdown * count ;

            int totalYards = pStatRush->data.total ;
            int averageYards ;
            if (pStatRush->Statistic::data.number) {
               averageYards = totalYards / pStatRush->Statistic::data.number ;
            }
            else {
               averageYards = 0 ;
            }

            // 1 pts for each 10 rushing yards above 50
            if ( (totalYards > av.rbRushingYardsThreshhold) &&
							(av.rbRushingYardsStepValue > 0) ) {
               int awardPoints = (totalYards - av.rbRushingYardsThreshhold) /
                                   av.rbRushingYardsStepValue ;
               awardPoints *= av.rbRushingYards ;
               statHOF.data.pointCount += awardPoints ;
            }

            // average rush yardage
            if ( (averageYards > av.rbYardsPerCarryThreshhold) &&
							(av.rbYardsPerCarryStepValue > 0) ) {
               int awardPoints = (averageYards - av.rbYardsPerCarryThreshhold) /
                                   av.rbYardsPerCarryStepValue ;
               awardPoints *= av.rbYardsPerCarry ;
               statHOF.data.pointCount += awardPoints ;
            }
         }
         else if (genPos == RosPlayer::POS_REC) {
            statHOF.data.pointCount += av.recRushingTouchdown * count ;

            // 1 pts for each 10 rushing yards above 50
            int totalYards = pStatRush->data.total ;

            if ( (totalYards > av.recRushingYardsThreshhold) &&
							(av.recRushingYardsStepValue > 0) ) {
               int awardPoints = (totalYards - av.recRushingYardsThreshhold) /
                                   av.recRushingYardsStepValue ;
               awardPoints *= av.recRushingYards ;
               statHOF.data.pointCount += awardPoints ;
            }
         }
         else if (genPos == RosPlayer::POS_K) {
            statHOF.data.pointCount += av.kRushingTouchdown * count ;
         }
         else if (genPos == RosPlayer::POS_P) {
            statHOF.data.pointCount += av.pRushingTouchdown * count ;
         }
      } break ;

      case PGSTAT_PAT2: {
         StatisticPAT *pStatPAT = (StatisticPAT *) pStat ;

         int count = pStatPAT->data.made ;
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 2 ;
             statHOF.data.pointCount += count * 2 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 2 ;
             statHOF.data.pointCount += count * 2 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 2 * av.kPointsScored ;

				  // -- 1 points for each 2 percent above 50
				 // -- 1 point for each 5 percentage points above 50
				if (pStatPAT->Statistic::data.number > 0 )
				{
					int fgPercentage = static_cast<int>(100 * (static_cast<double>(pStatPAT->data.made) /
									static_cast<double>(pStatPAT->Statistic::data.number)));
					if ( (fgPercentage > av.kPointAfterPercentageThresshold) &&
									(av.kPointAfterPercentageStepValue > 0) )
					{
						int fgPerAbove = (fgPercentage - av.kPointAfterPercentageThresshold);
						statHOF.data.pointCount += (fgPerAbove / av.kPointAfterPercentageStepValue)
																	* av.kPointAfterPercentage;
					}
				}
         }

         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount = av.qbTwoPointConversion * count ;
         }
      } break ;

      case PGSTAT_FUMBLE: {
         StatisticFumble *pStatFumble = (StatisticFumble *) pStat ;

         int count = pStatFumble->data.number ;
         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount = av.qbFumble * count ;
         }
         if (count && genPos == RosPlayer::POS_RB) {
            statHOF.data.pointCount = av.rbFumbles * count ;
         }
         if (count && genPos == RosPlayer::POS_REC) {
            statHOF.data.pointCount = av.recFumbles * count ;
         }
         if (count && genPos == RosPlayer::POS_K) {
            statHOF.data.pointCount = av.kFumble * count ;
         }
         if (count && genPos == RosPlayer::POS_P) {
            statHOF.data.pointCount = av.pFumble * count ;
         }
      } break ;

      case PGSTAT_PENALTY: {
         StatisticPenalty *pStatPenalty = (StatisticPenalty *) pStat ;

         int count = pStatPenalty->data.number ;
         if (count && genPos == RosPlayer::POS_QB) {
            statHOF.data.pointCount = av.qbPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_RB) {
            statHOF.data.pointCount = av.rbPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_REC) {
            statHOF.data.pointCount = av.recPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_K) {
            statHOF.data.pointCount = av.kPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_P) {
            statHOF.data.pointCount = av.pPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_OL) {
            statHOF.data.pointCount = av.olPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_DL) {
            statHOF.data.pointCount = av.dlPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_LB) {
            statHOF.data.pointCount = av.lbPenalty * count ;
         }
         if (count && genPos == RosPlayer::POS_DB) {
            statHOF.data.pointCount = av.dbPenalty * count ;
         }
      } break ;

      case PGSTAT_PASSREC: {
         StatisticPassRec *pStatPassRec = (StatisticPassRec *) pStat ;
         int count = pStatPassRec->StatisticSum::data.tds ; // how many touchdowns
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 6 * av.kPointsScored ;
         }

         if ( genPos == RosPlayer::POS_RB) {
            statHOF.data.pointCount = av.rbReceivingTouchdown * count ;
         }
         if ( genPos == RosPlayer::POS_REC) {
            statHOF.data.pointCount += av.recReceivingTouchdown * count ;
            int totalYardsReceived = pStatPassRec->StatisticSum::data.total ;
            int averageYards ;
            if (pStatPassRec->Statistic::data.number) {
               averageYards = totalYardsReceived / pStatPassRec->Statistic::data.number ;
            }
            else {
               averageYards = 0 ;
            }

            // REC total receivingYards
            if ( (totalYardsReceived > av.recReceivingYardsThreshhold) &&
							(av.recReceivingYardsStepValue > 0) ) {
               int awardPoints = (totalYardsReceived - av.recReceivingYardsThreshhold)
                                 / av.recReceivingYardsStepValue ;
               awardPoints *= av.recReceivingYards ;
               statHOF.data.pointCount += awardPoints ;
            }

            // REC average receivingYards
            if ( (averageYards > av.recYardsPerReceptionThreshhold) &&
							(av.recYardsPerReceptionStepValue > 0) ) {
               int awardPoints = (averageYards - av.recYardsPerReceptionThreshhold)
                                 / av.recYardsPerReceptionStepValue ;
               awardPoints *= av.recYardsPerReception ;
               statHOF.data.pointCount += awardPoints ;
            }
         }
         if (genPos == RosPlayer::POS_RB) {
            int totalYardsReceived = pStatPassRec->StatisticSum::data.total ;
            int averageYards ;
            if (pStatPassRec->Statistic::data.number) {
               averageYards = totalYardsReceived / pStatPassRec->Statistic::data.number ;
            }
            else {
               averageYards = 0 ;
            }

            // RB total receivingYards
            if ( (totalYardsReceived > av.rbReceivingYardsThreshhold) &&
							(av.rbReceivingYardsStepValue > 0) ) {
               int awardPoints = (totalYardsReceived - av.rbReceivingYardsThreshhold)
                                 / av.rbReceivingYardsStepValue ;
               awardPoints *= av.rbReceivingYards ;
               statHOF.data.pointCount += awardPoints ;
            }

            // RB average receivingYards
            if ( (averageYards > av.rbYardsPerReceptionThreshhold) &&
							(av.rbYardsPerReceptionStepValue > 0) ) {
               int awardPoints = (averageYards - av.rbYardsPerReceptionThreshhold)
                                 / av.rbYardsPerReceptionStepValue ;
               awardPoints *= av.rbYardsPerReception ;
               statHOF.data.pointCount += awardPoints ;
            }
         }
      } break ;

      case PGSTAT_KICKRET: {
         StatisticKickRet *pStatKickRet = (StatisticKickRet *) pStat ;
         int count = pStatKickRet->StatisticSum::data.tds ; // how many touchdowns
         int krYards = pStatKickRet->StatisticSum::data.total ; // how many kr yards
         int awardPoints ;

         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 6 ;
             statHOF.data.pointCount += count * 6 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 6 * av.kPointsScored ;
         }

         switch (genPos) {
            case RosPlayer::POS_RB: {
               // points for kr touchdown
               statHOF.data.pointCount += av.rbKickReturnTouchdown * count ;

               // points for return yardage
               awardPoints = krYards - av.rbKickReturnYardsThreshhold ;
               if ( (awardPoints) && (av.rbKickReturnYardsStepValue > 0) ) {
                  awardPoints /= av.rbKickReturnYardsStepValue ;
                  awardPoints *= av.rbKickReturnYards ;
                  statHOF.data.pointCount += awardPoints ;
               }
            } break ;

            case RosPlayer::POS_REC: {
               // points for kr touchdown
               statHOF.data.pointCount += av.recKickReturnTouchdown * count ;

               // points for return yardage
               awardPoints = krYards - av.recKickReturnYardsThreshhold ;
               if ( (awardPoints) && (av.recKickReturnYardsStepValue > 0) ) {
                  awardPoints /= av.recKickReturnYardsStepValue ;
                  awardPoints *= av.recKickReturnYards ;
                  statHOF.data.pointCount += awardPoints ;
               }
            } break ;

            case RosPlayer::POS_LB: {
               // points for kr touchdown
               statHOF.data.pointCount += av.lbKickReturnTouchdown * count ;

               // points for return yardage
               awardPoints = krYards - av.lbKickReturnYardsThreshhold ;
               if ( (awardPoints) && (av.lbKickReturnYardsStepValue > 0) ) {
                  awardPoints /= av.lbKickReturnYardsStepValue ;
                  awardPoints *= av.lbKickReturnYards ;
                  statHOF.data.pointCount += awardPoints ;
               }
            } break ;

            case RosPlayer::POS_DB: {
               // points for kr touchdown
               statHOF.data.pointCount += av.dbKickReturnTouchdown * count ;

               // points for return yardage
               awardPoints = krYards - av.dbKickReturnYardsThreshhold ;
               if ((awardPoints) && (av.dbKickReturnYardsStepValue > 0) ) {
                  awardPoints /= av.dbKickReturnYardsStepValue ;
                  awardPoints *= av.dbKickReturnYards ;
                  statHOF.data.pointCount += awardPoints ;
               }
            } break ;
         } // end switch (genPos)
      } break ; // end case PGSTAT_KICKRET:

      case PGSTAT_TACKLE: {
         int count = pStat->data.number ; // how many of these actions were done

         switch (genPos) {
            case RosPlayer::POS_DB: {
               statHOF.data.pointCount += av.dbTackle * count ;
            } break ;
            case RosPlayer::POS_LB: {
               statHOF.data.pointCount += av.lbTackle * count ;
            } break ;
            case RosPlayer::POS_DL: {
               statHOF.data.pointCount += av.dlTackle * count ;
            } break ;
         } // end switch (genPos)
      } break ;

      case PGSTAT_SACK: {
         int count = pStat->data.number ; // how many of these actions were done

         switch (genPos) {
            case RosPlayer::POS_LB: {
               statHOF.data.pointCount += av.lbIndividualSack * count ;
            } break ;
            case RosPlayer::POS_DL: {
               statHOF.data.pointCount += av.dlIndividualSack * count ;
            } break ;
         } // end switch (genPos)
      } break ;

      case PGSTAT_FUMBLEREC: {
         int count = pStat->data.number ; // how many of these actions were done

         switch (genPos) {
            case RosPlayer::POS_DB: {
               statHOF.data.pointCount += av.dbFumbleRecovery * count ;
            } break ;
            case RosPlayer::POS_LB: {
               statHOF.data.pointCount += av.lbFumbleRecovery * count ;
            } break ;
            case RosPlayer::POS_DL: {
               statHOF.data.pointCount += av.dlFumbleRecovery * count ;
            } break ;
         } // end switch (genPos)
      } break ;

      case PGSTAT_INT: {
         StatisticInt *pStatInt = (StatisticInt *) pStat ;
         int awardPoints = pStatInt->data.total ;
         int count = pStat->data.number ; // how many of these actions were done

         switch (genPos) {
            case RosPlayer::POS_DB: {
               // points for interception
               statHOF.data.pointCount += av.dbInterception * count ;
               // points for interception yardage
               awardPoints -= av.dbYardsPerInterceptionReturnThreshhold ;
					if ( av.dbYardsPerInterceptionReturnStepValue > 0 )
					{
	               awardPoints /= av.dbYardsPerInterceptionReturnStepValue ;
	               awardPoints *= av.dbYardsPerInterceptionReturn ;
	               if (awardPoints) {
	                  statHOF.data.pointCount += awardPoints ;
						}
               }
            } break ;
            case RosPlayer::POS_LB: {
               statHOF.data.pointCount += av.lbInterception * count ;
               // points for interception yardage
               awardPoints -= av.lbYardsPerInterceptionReturnThreshhold ;
					if (av.lbYardsPerInterceptionReturnStepValue > 0 )
					{
	               awardPoints /= av.lbYardsPerInterceptionReturnStepValue ;
	               awardPoints *= av.lbYardsPerInterceptionReturn ;
	               if (awardPoints) {
	                  statHOF.data.pointCount += awardPoints ;
						}
               }
            } break ;
         } // end switch (genPos)
      } break ;

      case PGSTAT_PAT: {
         StatisticPAT *pStatPAT = (StatisticPAT *) pStat ;

         int count = pStatPAT->data.made ;
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 1 ;
             statHOF.data.pointCount += count * 1 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 1 ;
             statHOF.data.pointCount += count * 1 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 1 * av.kPointsScored ;
         }
      } break ;

      case PGSTAT_SAFETY: {
         StatisticSafety *pStatSafety = (StatisticSafety *) pStat ;

         int count = pStatSafety->Statistic::data.number ;
         // Track total points for certain positions for highest scorer calculations
         // give award for each point scored
         if (genPos == RosPlayer::POS_RB) {
             statHOF.Statistic::data.number = count * 2 ;
             statHOF.data.pointCount += count * 2 * av.rbPointsScored ;
         }
         else if (genPos == RosPlayer::POS_REC) {
             statHOF.Statistic::data.number = count * 2 ;
             statHOF.data.pointCount += count * 2 * av.recPointsScored ;
         }
         else if (genPos == RosPlayer::POS_K) {
             statHOF.data.pointCount += count * 2 * av.kPointsScored ;
         }
      } break ;

      case PGSTAT_FG: {
         StatisticFG *pStatFG = (StatisticFG *) pStat ;

         if (genPos == RosPlayer::POS_K) {
				 // -- 1 point for each point scored for any distance

				int totalMade = (pStatFG->data.range10 & 0x00FF);
				totalMade += (pStatFG->data.range20 & 0x00FF);
				totalMade += (pStatFG->data.range30 & 0x00FF);
				totalMade += (pStatFG->data.range40 & 0x00FF);
				totalMade += (pStatFG->data.range50 & 0x00FF);
				statHOF.data.pointCount += totalMade * 3 * av.kPointsScored;

				if ( (totalMade > 0) && (av.kYardsPerFieldGoalStepValue > 0) )
				{
				   // -- 1 point for each 2 average yards per fg over 10
					int totalYards = (pStatFG->data.range10 & 0x00FF) * 15;
					totalYards += (pStatFG->data.range20 & 0x00FF) * 25;
					totalYards += (pStatFG->data.range30 & 0x00FF) * 35;
					totalYards += (pStatFG->data.range40 & 0x00FF) * 45;
					totalYards += (pStatFG->data.range50 & 0x00FF) * 55;
					statHOF.data.pointCount +=
						((totalYards / totalMade) / av.kYardsPerFieldGoalStepValue) * av.kYardsPerFieldGoal;
				}

				int totalAtt = (pStatFG->data.range10 & 0xFF00);
				totalAtt += (pStatFG->data.range20 & 0xFF00);
				totalAtt += (pStatFG->data.range30 & 0xFF00);
				totalAtt += (pStatFG->data.range40 & 0xFF00);
				totalAtt += (pStatFG->data.range50 & 0xFF00);
				totalAtt = totalAtt >> 8;  // get the lower byte
	
				 // -- 1 point for each 5 percentage points above 50
				if ( totalAtt > 0 )
				{
					int fgPercentage = static_cast<int>(100 * (static_cast<double>(totalMade) /
															static_cast<double>(totalAtt)));
					if ( (fgPercentage > av.kFieldGoalPercentageThreshhold) &&
								(av.kFieldGoalPercentageStepValue > 0) )
					{
						int fgPerAbove = (fgPercentage - av.kFieldGoalPercentageThreshhold);
						statHOF.data.pointCount += (fgPerAbove / av.kFieldGoalPercentageStepValue)
																			* av.kFieldGoalPercentage;
					}
				}

				if ( (totalMade < totalAtt) &&		// missed >1 FG
							(scores[teamIndex] < scores[!teamIndex]) &&	// losing team
								((scores[!teamIndex] - scores[teamIndex]) <= 3) )	// close game
				{
					 // -- -25 for having one or more missed FG in a close losing game
					statHOF.data.pointCount += av.kMissedFieldGoalInCloseLoss;
				}

         }
      } break ;

		case PGSTAT_PUNT:
		{
         StatisticPunt *pStatPunt = (StatisticPunt *) pStat;

         if ( genPos == RosPlayer::POS_P )
			{
				 //  average yards per punt
				if ( pStatPunt->Statistic::data.number > 0 )
				{
					int puntYardsGT = pStatPunt->StatisticSum::data.total /
                                 pStatPunt->Statistic::data.number ;

					if ( ((puntYardsGT - av.pYardsPerPuntThreshhold) > 0) &&
								(av.pYardsPerPuntStepValue > 0) )
					{
						 statHOF.data.pointCount += (puntYardsGT - av.pYardsPerPuntThreshhold)
		                                           / av.pYardsPerPuntStepValue
			                                           * av.pYardsPerPunt;
					}
				}
			} // end if a punter

		} // end case PGSTAT_PUNT
			break;

   } // end switch (type)
   AddHOFGSC (&statHOF, id, ConferenceIndex); // add points to Game Season and Career totals
} //end StatFile::AddHOF1 ()

/* HOF JWM 9/21
*  award points for team plays after all stats are processed
*  also award points for playing in game (having one stat)
*/
void
StatFile::AddHOF2 (DYN_8 teamIndex)
{
   RosPlayer::Pos genPos  ;
//   RosPlayer::Pos SpecPos ;
   Team           *pTeam =  pTeams[teamIndex] ;
   RosPlayer      *playerPtr ;
   DYN_U_16       playerId ;
   StatisticHOF statHOF ; // local statHOF global also exists
   StatisticHOF *statScoreHOF ;
   StatisticRush *statLongRush ;
   statHOF.Statistic::data.number = 0 ; // init to collect sums later
   statHOF.data.pointCount = 0 ;
   int winningTeam = (scores[1] > scores[0]) ;
   int mostQBPlays = 0 ;
   int winningQB   = 0 ;
   int highScore   = 0 ;
   int highScorer  = 0 ;
   int longestRush = 0 ;
   int longestRusher = 0 ;

   Roster      *pRoster = ActiveLeague->LoadPlayers (pTeam) ;

// points for Team Sack
   int teamSackCount = 0 ;
   StatisticSack *statSack = (StatisticSack *) GetStat (TGSTAT_SACK, teamIds[teamIndex]) ;
   if (statSack != NULL) {
      teamSackCount = statSack->data.number ;
   }
// points for Opposing Team Sack
   int oTeamSackCount = 0 ;
   StatisticSack *statOSack = (StatisticSack *) GetStat (TGSTAT_SACK, teamIds[!teamIndex]) ;
   if (statOSack != NULL) {
      oTeamSackCount = statOSack->data.number ;
   }
// points for Team Rush
   int teamRushingTDCount = 0 ;
   int teamRushingLongest = 0 ;
   int TeamRushingYards   = 0 ;
   StatisticRush *statRush = (StatisticRush *) GetStat (TGSTAT_RUSH, teamIds[teamIndex]) ;
   if (statRush != NULL) {
      teamRushingTDCount = statRush->data.tds ;
      teamRushingLongest = statRush->data.best ; // set to 0 if OT is longer
      TeamRushingYards   = statRush->data.total ;

   }
// points for opposing Team Rush
   int oTeamRushingTDCount = 0 ;
   int oTeamRushingYards = 0 ;
   int oTeamAverageRushingYards = 0 ;
   int oTeamRushingTotal = 0 ;
   StatisticRush *statRushOT = (StatisticRush *) GetStat (TGSTAT_RUSH, teamIds[!teamIndex]) ;
   if (statRushOT != NULL) {
      oTeamRushingTDCount = statRushOT->data.tds ;
      oTeamRushingYards   = statRushOT->data.total ;
      oTeamRushingTotal   = statRushOT->Statistic::data.number ;
      if (statRushOT->data.best > teamRushingLongest) {
         teamRushingLongest = 0 ;
      }
		if ( oTeamRushingTotal > 0)
		{
	      oTeamAverageRushingYards = oTeamRushingYards / oTeamRushingTotal ;
		}
   }
// points for 2pt conv
   int teamPAT2count = 0 ;
   StatisticPAT *pStatPAT2 = (StatisticPAT *) GetStat (TGSTAT_PAT2, teamIds[teamIndex]) ;
   if (pStatPAT2 != NULL) {
      teamPAT2count = pStatPAT2->data.made ;
   }
// points for opposing Team 2pt conv
   int oTeamPAT2count = 0 ;
   StatisticPAT *pStatPAT2OT = (StatisticPAT *) GetStat (TGSTAT_PAT2, teamIds[!teamIndex]) ;
   if (pStatPAT2OT != NULL) {
      oTeamPAT2count = pStatPAT2OT->data.made ;
   }
// points for opposing team FG
   int oTeamFGattempts = 0 ;
   StatisticFG *pStatOTFG = (StatisticFG *) GetStat (TGSTAT_FG, teamIds[!teamIndex]) ;
   if (pStatOTFG != NULL) {
      oTeamFGattempts =  pStatOTFG->data.range10 >> 8 ;
      oTeamFGattempts += pStatOTFG->data.range20 >> 8 ;
      oTeamFGattempts += pStatOTFG->data.range30 >> 8 ;
      oTeamFGattempts += pStatOTFG->data.range40 >> 8 ;
      oTeamFGattempts += pStatOTFG->data.range50 >> 8 ;
   }
// points for FG
   int teamFGattempts = 0 ;
   StatisticFG *pStatFG = (StatisticFG *) GetStat (TGSTAT_FG, teamIds[teamIndex]) ;
   if (pStatFG != NULL) {
      teamFGattempts =  pStatFG->data.range10 >> 8 ;
      teamFGattempts += pStatFG->data.range20 >> 8 ;
      teamFGattempts += pStatFG->data.range30 >> 8 ;
      teamFGattempts += pStatFG->data.range40 >> 8 ;
      teamFGattempts += pStatFG->data.range50 >> 8 ;
   }
// points for opposing Team Pass
   int oTeamPassingTDCount = 0 ;
   int oTeamPassingYards   = 0 ;
   int oTeamAveragePassingYards   = 0 ;
   int oTeamPassingTotal   = 0 ;
   StatisticPass *statPassOT = (StatisticPass *) GetStat (TGSTAT_PASS, teamIds[!teamIndex]) ;
   if (statPassOT != NULL) {
      oTeamPassingTDCount = statPassOT->StatisticSum::data.tds ;
      oTeamPassingYards   = statPassOT->StatisticSum::data.total ;
      oTeamPassingTotal   = statPassOT->Statistic::data.number ;
		if ( oTeamPassingTotal > 0)
		{
	      oTeamAveragePassingYards  = oTeamPassingYards / oTeamPassingTotal ;
		}
   }

// points for Team Pass
   int TeamPassingTDCount     = 0 ;
   int TeamPassingYards       = 0 ;
   int TeamPassingTotal       = 0 ;
   int TeamPassingYardAverage = 0 ;
   int TeamPassingNumber      = 0 ;
   StatisticPass *statPass = (StatisticPass *) GetStat (TGSTAT_PASS, teamIds[teamIndex]) ;
   if (statPass != NULL) {
      TeamPassingYards   = statPass->StatisticSum::data.total ;
      TeamPassingNumber  = statPass->Statistic::data.number ;
		if ( TeamPassingNumber > 0 )
		{
	      TeamPassingYardAverage = TeamPassingYards / TeamPassingNumber ;
		}
   }

	 // -- points for blocked punts
	int teamBlockedPunts = 0;
	StatisticPunt *statPunt = (StatisticPunt *) GetStat (TGSTAT_PUNT, teamIds[teamIndex]) ;
	if ( statPunt != NULL )
	{
		teamBlockedPunts = statPunt->data.blocks;
	}
	 // -- points for blocked punts
	int oTeamBlockedPunts = 0;
	StatisticPunt *statPuntO = (StatisticPunt *) GetStat (TGSTAT_PUNT, teamIds[!teamIndex]) ;
	if ( statPuntO != NULL )
	{
		oTeamBlockedPunts = statPuntO->data.blocks;
	}

   for (int lcv = 0 ; lcv < sizeof (playersInGame[teamIndex]) / sizeof (playersInGame[0][0]) ; lcv++) {
      playerId  = playersInGame[teamIndex][lcv] ;
      if (playerId == 0) {
         break ;
      }
      playerPtr = pRoster->GetPlayer (playerId) ;
      genPos  = playerPtr->GetPosGen () ;
      //SpecPos = playerPtr->GetPosSpec () ;
      switch (genPos) {

         case RosPlayer::POS_QB:
            statHOF.data.pointCount = av.qbPlaysGame ;
            // calculate the winning QB
            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
               StatisticHOF *statQBHOF = (StatisticHOF *) GetStat (PGSTAT_HOF, playerId);
               if (statQBHOF == NULL) {
                  break ;
               }
               if (statQBHOF->Statistic::data.number > mostQBPlays) {
                   mostQBPlays = statQBHOF->Statistic::data.number ;
                   winningQB = playerId ;
               }
            }
         break ;

         case RosPlayer::POS_RB:
            statHOF.data.pointCount = av.rbPlaysGame ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            statScoreHOF = (StatisticHOF *) GetStat (PGSTAT_HOF, playerId);
            if (statScoreHOF == NULL) {
               break ;
            }
            if (statScoreHOF->Statistic::data.number > highScore) {
                highScore = statScoreHOF->Statistic::data.number ;
                highScorer = playerId ;
            }

            statLongRush = (StatisticRush *) GetStat (PGSTAT_RUSH, playerId);
            if (statLongRush == NULL) {
               break ;
            }
            if (statLongRush->data.best > longestRush) {
                longestRush = statLongRush->data.best ;
                longestRusher = playerId ;
            }
         break ;

         case RosPlayer::POS_REC:
            statHOF.data.pointCount = av.recPlaysGame ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            statLongRush = (StatisticRush *) GetStat (PGSTAT_RUSH, playerId);
            if (statLongRush == NULL) {
               break ;
            }
            if (statLongRush->data.best > longestRush) {
                longestRush = statLongRush->data.best ;
                longestRusher = playerId ;
            }
         break ;

         case RosPlayer::POS_OL:
            statHOF.data.pointCount = av.olPlaysGame +
                                      (teamRushingTDCount * av.olTeamRushingTouchdown) +
                                      (scores[teamIndex] * av.olTeamPointsScored) +
                                      (oTeamSackCount * av.olTeamSacks) +
                                      (teamPAT2count *  av.olTeamTwoPointConversions) +
                                      (teamFGattempts * av.olTeamTriesFieldGoal) +
												  (teamBlockedPunts * av.olTeamBlockedKickPunt) ;


            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            if (teamRushingLongest > av.olLongestTeamCarryAdjustment) {
               statHOF.data.pointCount += av.olLongestTeamCarryAdjustment ;
            }

            if ( (TeamPassingYardAverage > av.olTeamYardsPerCarryThreshhold) &&
							(av.olTeamYardsPerCarryStepValue > 0) ){
               statHOF.data.pointCount += ((TeamPassingYardAverage - av.olTeamYardsPerCarryThreshhold)
                                          / av.olTeamYardsPerCarryStepValue
                                          )
                                          * av.olTeamYardsPerCarry ;

            }

            if ( (TeamRushingYards > av.olTeamRushingYardsThreshhold) &&
							(av.olTeamRushingYardsStepValue > 0) ) {
               statHOF.data.pointCount += ((TeamRushingYards - av.olTeamYardsPerCarryThreshhold)
                                          / av.olTeamRushingYardsStepValue
                                          )
                                          * av.olTeamRushingYards ;

            }
         break ;

         case RosPlayer::POS_K:
            statHOF.data.pointCount = av.kPlaysGame ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }
         break ;

         case RosPlayer::POS_P:
            statHOF.data.pointCount = av.pPlaysGame ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }
         break ;

         case RosPlayer::POS_DL:
            statHOF.data.pointCount = av.dlPlaysGame +
                                      (teamSackCount * av.dlTeamSack) +
                                      (oTeamFGattempts * av.dlFieldGoalAttemptByOpponents) +
                                      (oTeamPAT2count *  av.dlTwoPointConversionsAllowed) +
                                      (oTeamRushingTDCount * av.dlRushingTouchdownAllowed) +
												  (oTeamBlockedPunts * av.dlTeamBlockedKick)
            ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            if ( (oTeamAverageRushingYards > av.dlYardsPerCarryThreshhold) &&
							(av.dlYardsPerCarryStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamAverageRushingYards - av.dlYardsPerCarryThreshhold)
                                          / av.dlYardsPerCarryStepValue
                                          )
                                          * av.dlYardsPerCarry ;

            }
            if ( (oTeamRushingYards > av.dlRushingYardsAllowedThreshhold) &&
							(av.dlRushingYardsAllowedStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamRushingYards - av.dlRushingYardsAllowedThreshhold)
                                          / av.dlRushingYardsAllowedStepValue
                                          )
                                          * av.dlRushingYardsAllowed ;

            }
         break ;

         case RosPlayer::POS_LB:
            statHOF.data.pointCount = av.lbPlaysGame +
                                      (oTeamFGattempts * av.lbFieldGoalAttemptByOpponent) +
                                      (oTeamPAT2count *  av.lbTwoPointConversionAllowed) +
                                      (oTeamPassingTDCount *  av.lbPassingTouchdownAllowed) +
                                      (oTeamRushingTDCount * av.lbRushingTouchdownAllowed) +
												  (oTeamBlockedPunts * av.lbTeamBlockedKickPunt)
            ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            if ( (oTeamAverageRushingYards > av.lbYardsPerCarryThreshhold) &&
							(av.lbYardsPerCarryStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamAverageRushingYards - av.lbYardsPerCarryThreshhold)
                                          / av.lbYardsPerCarryStepValue
                                          )
                                          * av.lbYardsPerCarry ;

            }
            if ( (oTeamRushingYards > av.lbRushingYardsAllowedThreshhold) &&
							(av.lbRushingYardsAllowedStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamRushingYards - av.lbRushingYardsAllowedThreshhold)
                                          / av.lbRushingYardsAllowedStepValue
                                          )
                                          * av.lbRushingYardsAllowed ;

            }
            if ( (oTeamAveragePassingYards > av.lbYardsPerReceptionThreshhold) &&
							(av.lbYardsPerReceptionStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamAveragePassingYards - av.lbYardsPerReceptionThreshhold)
                                          / av.lbYardsPerReceptionStepValue
                                          )
                                          * av.lbYardsPerReception ;

            }
            if ( (oTeamPassingYards > av.lbPassingYardsAllowedThreshhold) &&
							(av.lbPassingYardsAllowedStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamPassingYards - av.lbPassingYardsAllowedThreshhold)
                                          / av.lbPassingYardsAllowedStepValue
                                          )
                                          * av.lbPassingYardsAllowed ;

            }
         break ;

         case RosPlayer::POS_DB:
            statHOF.data.pointCount = av.dbPlaysGame +
                                      (oTeamPassingTDCount *  av.dbPassingTouchdownAllowed) +
                                      (oTeamPAT2count *  av.dbTwoPointConversionAllowed)
            ;

            if (teamIndex == winningTeam) {
               statHOF.data.pointCount += av.winningTeam ;
            }

            if ( (oTeamAveragePassingYards > av.dbYardsPerReceptionThreshhold) &&
							(av.dbYardsPerReceptionStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamAveragePassingYards - av.dbYardsPerReceptionThreshhold)
                                          / av.dbYardsPerReceptionStepValue
                                          )
                                          * av.dbYardsPerReception ;
            }

            if ( (oTeamPassingYards > av.dbPassingYardsAllowedThreshhold) &&
							(av.dbPassingYardsAllowedStepValue > 0) ) {
               statHOF.data.pointCount += ((oTeamPassingYards - av.dbPassingYardsAllowedThreshhold)
                                          / av.dbPassingYardsAllowedStepValue
                                          )
                                          * av.dbPassingYardsAllowed ;
            }
         break ;
      } // end switch (genPos)
      AddHOFGSC (&statHOF, playerId,ConferenceIndex); // add points to Game Season and Career totals
   } // end for
   // add points for winning QB
   statHOF.data.pointCount = av.qbWinningQB ;
   AddHOFGSC (&statHOF, winningQB,0xffff);

   // points for high Scorer
   playerPtr = pRoster->GetPlayer (highScorer) ;
   genPos  = playerPtr->GetPosGen () ;
   if (genPos == RosPlayer::POS_RB) {
       statHOF.data.pointCount = av.rbLeadingScorer ;
   }
   else if (genPos == RosPlayer::POS_REC) {
       statHOF.data.pointCount = av.recLeadingScorer ;
   }
   AddHOFGSC (&statHOF, highScorer,0xffff);

   // points for longest rush
   playerPtr = pRoster->GetPlayer (longestRusher) ;
   genPos  = playerPtr->GetPosGen () ;
   if (genPos == RosPlayer::POS_RB) {
       statHOF.data.pointCount = av.rbLongestCarryAdjustment ;
   }
   else if (genPos == RosPlayer::POS_REC) {
       statHOF.data.pointCount = av.recLongestCarryAdjustment  ;
   }
   AddHOFGSC (&statHOF, highScorer,0xffff);

} //end StatFile::AddHOF2 ()


void
StatFile::AddHOFGSC(StatisticHOF *s, DYN_U_16 id, DYN_U_16 confIndex)
{
   if ( (s == NULL) || (id == 0) || (s->data.pointCount <= 0) ) {
      return ;
   }

   if (confIndex != 0xffff) {
      s->data.confIndex = confIndex ;
   }
   AddStat (s, PGSTAT_HOF, id) ;
   AddStat (s, PSSTAT_HOF, id) ;
   AddStat (s, PCSTAT_HOF, id) ;
}

void StatFile::CalcHALLOFAME()
{

   DYN_16 year = ActiveLeague->GetInception () + ActiveLeague->GetNumSeasons () ;
   StatisticHALLOFAME *pStat, *pStatPlayerWithMost ;

   GenList<Statistic> statList ;
   DYN_16 statCount = BuildStatList(&statList, PCSTAT_HALLOFAME, 0) ;
   if (!statCount) {
      return ;
   }

   int maxElectedPlayers ;
   int mostPoints = 0 ;
   int minimumPoints = 0 ;

   // calculate the maximum number of players to be elected
   switch (ActiveLeague->GetConfig ()) {
      case League::CONFIG_8:
         maxElectedPlayers = av.MaxPlayers8Team ;
      break ;

      case League::CONFIG_10:
         maxElectedPlayers = av.MaxPlayers10Team ;
      break ;

      case League::CONFIG_12:
         maxElectedPlayers = av.MaxPlayers12Team ;
      break ;

      case League::CONFIG_18:
         maxElectedPlayers = av.MaxPlayers18Team ;
      break ;

      case League::CONFIG_28:
         maxElectedPlayers = av.MaxPlayers28Team ;
      break ;

      case League::CONFIG_30:
         maxElectedPlayers = av.MaxPlayers30Team ;
      break ;
   }

   // deleted players nominatied over 20 years ago from stat database
   // skip anybody nominated less than 5 years ago
   // skip anybody already elected
   statList.start() ;
   for (int count = 0 ; count < statCount ; count++ ) {
      pStat = (StatisticHALLOFAME *) statList.get() ;
      if ((year - pStat->data.YearRetired) > av.YearsToIneligible) {
       DeleteStat(PCSTAT_HALLOFAME, pStat->Statistic::data.id) ;
       statList.linkout() ;
      }
      else if ((year - pStat->data.YearRetired) < 5) {
       statList.linkout() ;
      }
      else if (pStat->data.YearElected != 0) {
       statList.linkout() ;
      }
      else {
       // find the eligable retiree with the most points
       if (pStat->data.pointCount > mostPoints) {
          mostPoints = pStat->data.pointCount ;
       }
       statList.fwd() ;
      }
statList.size() ; // just for debug
   }

   // found the maximum amount of points so compute the minimum points and pick out the winners
   minimumPoints = mostPoints * av.PercentOfTopPlayerValue / 100 ;
   statCount = statList.size() ;
   while (maxElectedPlayers != 0) {
	   mostPoints = 0 ;					// moved to inside while loop		18 NOV 96 CAM
		pStatPlayerWithMost = NULL;	// reset this to make sure  18 NOV 96 CAM

      statList.start() ;
      statCount = statList.size() ;
      if (!statCount) {
         break ;
      }
      while (statCount != 0) {
         pStat = (StatisticHALLOFAME *) statList.get() ;
         if (pStat->data.pointCount > mostPoints) {
            mostPoints = pStat->data.pointCount ;
            pStatPlayerWithMost = pStat ;
         }
         statCount-- ;
         statList.fwd() ;
      }
      if (mostPoints < minimumPoints) {
         break ;
      }
		if ( pStatPlayerWithMost != NULL )
		{
	      pStatPlayerWithMost->data.YearElected = year ;
	      AddStat((Statistic *) pStatPlayerWithMost, PCSTAT_HALLOFAME,
														pStatPlayerWithMost->Statistic::data.id) ;

	      maxElectedPlayers-- ;
	      // now remove this guy from the list
	      statList.start() ;
	      statCount = statList.size() ;
	      while (statCount != 0) {
	         pStat = (StatisticHALLOFAME *) statList.get() ;
//	         if (pStat->data.YearElected != 0) {		// statList.get returns a copy of the data
				if ( pStat->Statistic::data.id ==
												pStatPlayerWithMost->Statistic::data.id )
				{
	            statList.linkout() ;			
	            break ;
	         }
	         statList.fwd() ;	// be sure to increment the list, John!  CAM
	         statCount-- ;
	      }
		} // if found someone
		else
		{
			 // -- must not be any more guys
			break;
		}

   } // while still need more players elected

} // StatFile::CalcHALLOFAME

int StatFile::AddToHALLOFAME(DYN_U_16 playerId)
{
   if (ActiveLeague == NULL) {
      return FALSE ;
   }
   RosPlayer * player ;
   player = ActiveLeague->FindPlayer(playerId, 1) ;
	if ( player == NULL )
		return( FALSE );

   DYN_16 year = ActiveLeague->GetInception () + ActiveLeague->GetNumSeasons () ;
   DYN_16 yearsExp = player->GetYears () ;

   StatisticHOF *statHOF = (StatisticHOF *) GetStat(PCSTAT_HOF, playerId) ;
   if (statHOF == NULL) {
      return FALSE ;
   }

   StatisticHALLOFAME statHALLOFAME ;
   statHALLOFAME.data.YearRetired = year ;
   statHALLOFAME.data.YearElected = 0 ;
   if ( (ActiveLeague->GetNumSeasons () < yearsExp) &&
				(ActiveLeague->GetNumSeasons () > 0) ) {
      statHALLOFAME.data.pointCount = statHOF->data.pointCount
                                      * yearsExp
                                      / ActiveLeague->GetNumSeasons () ;
   }
   else {
      statHALLOFAME.data.pointCount = statHOF->data.pointCount ;
   }
   if (statHALLOFAME.data.pointCount > av.MinPointsForEligibility) {
      AddStat (&statHALLOFAME, PCSTAT_HALLOFAME, playerId) ;
   }
   DeleteStat(PCSTAT_HOF, playerId) ;
   return TRUE ;
}

// StatFile.cpp
