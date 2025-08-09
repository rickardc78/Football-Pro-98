#include "FBWin.h"
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


	league.cpp

	this file contains ENGLISH text for error messages and the
	default league name

	GJW: 1-17-1992, original creation
	GJW: 5-7-1992, added teamList to internal data
	GJW: 7-2-1992, different types fo 'fullLoads' so that quickstart needs
						only the teams loaded (i.e. smaller data kept)
	GJW: 8-13-1992, draft strategies
	GJW: 8-20-1992, trade, free agent, injury decisions every week
	GJW: 8-28-1992, table that maintains team compare relationships
							(i.e. save results of team 3 vs 5, so that when
							5 vs 3 is needed, they won't be re-computed)
							+1 means lower number team is better
							 0 means no comparision yet
							-1 means higher number team is better
	GJW: 3-24-1993, new format on disk (L00:)
	GJW: 5-3-1993, calender changed from pointer, to static 'has a',
						since all leagues use the same one (duh)
	GJW: 5-6-1993, pro bowl
	GJW: 7-15-1993, upped the talenCounts for ks and ps to 4 more than # of
							teams
	GJW: 7-27-1993, no CEI injuries for SINGLE-SEASON Leagues, just
							hurt the player a lot
	GJW: 8-10-1993, free agents created for a NEW LEAGUE (not rookies for
							a continuing), come in at 75 the normal for rookies
	GJW: 9-21-1993, moved the change to SEASON_RUNNING from GameComplete ()
							to HealPlayers (). Possible for a user to start a simmed
							game, pause it, save for later, then go to league data
							screen and change ground rules to fast sim (which doesn't
							allow paused games)
	GJW: 9-14-1994, Init () for emm
	GJW: 9-28-1993, new format on disk (L01:)
							nflFlag indicates logos are valid for screens
	GJW: 10-12-1994, HealPlayers heals ALL teams, even during the playoffs
	GJW: 10-18-1994, leagues are nfl valid if ALL the teams going in,
							are also NFL valid
	GJW: 11-22-1994, FDList has its nodes in far memory, which is used by
							PlayerList, so pre-allocation of nodes in
							LoadDraftList () can be a little more liberal once
							again
	GJW: 2-1-1995, 30 team leagues
	GJW: 7-18-1995, new format on disk (L02:) (no change in data,
							but used as a version number)

	CAM	11 JUL 96	Changed auxiliary league files from .PLN to .LG2
							Added .lg2 to delete list
	CAM	22 JUL 96	Added .set to delete list
	CAM	26 AUG 96	Changed default league to NflPi96
	CAM	10 SEP 96	Fixed LDDcpy2 to correctly save the trophy and commPN
							 items
	GJW	SEP 96		MultiPlayer trade
	CAM	23 SEP 96	Added ClosePlayerFile for MultiPlayer copying of this
							 file
							 Move FindPlayer out of #if LCDT section
	CAM	28 SEP 96	Move LoadPlayer(s) of of #if LCDT == 3 section
	MMQ 11 DEC 96   Added IsCurrentSeasonCompatible(), called from
	                ConfDivPlayersSetup
   JWM   24 DEC 96   Make the Probowl play the new All Pro Team


컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	#includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include <io.h>
#include <direct.h>
#include			<dos.h>
#include			<string.h>

#include			<ports.h>
#include			"dyndefs.h"
#include		"vm_mylib.h"

#include			"assertvm.h"
#include			"city.h"
#include			"draftpro.h"
#include			"fileutil.h"
#include			"league.h"
#include			"lcdtutil.h"
#include			"mono.h"
#include			"poolaloc.h"
#include			"equation.h"
#include			"ras_file.h"
#include			"fbmu.h"
#include			"aptcount.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
#define MAXEXT 5
#define	DEBUG_PLIST		FALSE
#define	DEBUG_PROBOWL	FALSE
#define	WEEKLY_MOVES	TRUE
#define	WEEKLY_STATS	FALSE

#if	DEBUG_PLIST | DEBUG_PROBOWL
#include "fdebug.h"
#endif

const MAX_TRADES_PER_WEEK = 1;
const MONO_START_LINE = 13;

// for draft strategies:
typedef struct
{
	DYN_8			tId;
	DYN_U_16		ranks [Roster::STATLIST_MAX];
} STRAT_STAT;

enum TeamId
{
		TEAM_HOME,
		TEAM_VISIT,
		TEAM_MAX,
};

const MAX_TEAMS = 30;
const COMP_TABLE_SIZE = MAX_TEAMS * ((MAX_TEAMS / 2) - 1) + (MAX_TEAMS / 2);

struct LeagueDiskData
{
		League::Config			config;
		DYN_8						numConfs;
		League::Type			tol;
		DYN_U_8					stage;
		League::FileType		fileType;
		DYN_8						nflFlag;
		DYN_16					uniqueId,
									nextPlayerId,
									inception;
		DYN_8						numSeasons;
		League::GROUND_RULES rules;
		DYN_8						name [League::NAME_LENGTH + 1],
									trophy [League::TROPHY_NAME_LENGTH + 1],
									commPN [Team::PN_NAME_LENGTH + 1];
};
 // -- mirror structure for reading in Win32
struct LeagueDiskDataF
{
		DYN_8/*League::Config*/ config;
		DYN_8						numConfs;
		DYN_8/*League::Type*/	tol;
		DYN_U_8					stage;
		DYN_8/*League::FileType*/ fileType;
		DYN_8						nflFlag;
		DYN_16					uniqueId,
									nextPlayerId,
									inception;
		DYN_8						numSeasons;
		League::GROUND_RULESF rules;
		DYN_8						name [League::NAME_LENGTH + 1],
									trophy [League::TROPHY_NAME_LENGTH + 1],
									commPN [Team::PN_NAME_LENGTH + 1];
};

struct FAR LeagData
{
	void						FAR * operator new (size_t size);
	void						operator delete (void FAR *t, size_t size);

	static PoolAlloc		*pa;	// where to get memory from

	League::Config			config;
	DYN_8						numConfs;
	League::Type			tol;
	DYN_U_8					stage;
	League::FileType		fileType;
	DYN_8						nflFlag;
	DYN_16					uniqueId,
								nextPlayerId,
								inception;
	DYN_8						numSeasons;
	League::GROUND_RULES rules;
	DYN_8						name [League::NAME_LENGTH + 1],
								trophy [League::TROPHY_NAME_LENGTH + 1],
								commPN [Team::PN_NAME_LENGTH + 1];
	PlayerRdWr				*fpPlayers;
	DYN_8						newFlag,
								saveEnable;								
	Conference				*conf;
	Schedule					*schedule;
	CollegeDraftPool		*cdpPtr;
	FreeAgentPool			*fapPtr;
	TradeNews				*tradeNews;
	StatFile					*statFile;
	LeagueChamps			*champs;

	League::Byes			byes;
	DYN_8						*numTrades,
								byesHealed,
								path [MAXPATH],
								fileName [MAXFILE];
};

const NUM_ALLOC = 4;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static DYN_16	compareFunc (void *iPtr, void *plusgPtr, DYN_16 sortType);
static DYN_16	NEAR alphaCompare (Team *tm1, Team *tm2);
static DYN_16	NEAR recordCompare (Team *tm1, Team *tm2);
static DYN_16	NEAR playoffCompare (Team *tm1, Team *tm2);
static DYN_16	NEAR idCompare (Team *tm1, Team *tm2);
static DYN_16	proBowlCompare (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType);
static void		NEAR setStatRankings (GenList<Team> *tList, StatFile *statFile,
											GenList<STRAT_STAT> *sList, STRAT_STAT *ssPtr);
static void		NEAR setStatRanks (GenList<STRAT_STAT> *sList, DYN_16 index);
static DYN_16	statCompareFunc (void *iPtr, void *plusgPtr, DYN_16 index);
static DYN_8	* NEAR getCompTablePtr (DYN_8 *base, DYN_8 index1, DYN_8 index2);
static void		NEAR defaultRules (League::GROUND_RULES FAR *grPtr);

#if	DEBUG
static void		NEAR saveMonoUpdate (DYN_U_16, DYN_U_16);
#endif

static void LDDcpy( LeagData *pdata, LeagueDiskDataF *pldd );
static void LDDcpy2( LeagueDiskDataF *pldd,	LeagData *pdata );

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

DYN_8 defLeagueFile [] = "_DEFAULT.LGE",
		dynText [] = "DYNAMIX",
		nflLeagueFile [] = "NFLPI96.LGE";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

enum {	NUM_KILL_FILES = 13	};

static DYN_8	thisDirText [] = ".",
					LGTText [] = "LGT:",
					lgeText [] = ".lge",
					pyfText [] = ".pyf",
					pyrText [] = ".pyr",
					rstText [] = ".rst",
					xxxText [] = ".xxx",
					lg2Text [] = ".lg2",		// added 1 JUL 96 JM/CAM
					setText [] = ".set",		// added 22 JUL 96 CAM
					*compTable,
					*deleteExts [NUM_KILL_FILES] =
					{
						".dat",	".idx",
						".dft",	".trn", ".tmn",
						".lgc",	lgeText, rstText,
						".pyc",	pyfText, pyrText,
						lg2Text, setText
					};

#if	(LCDT == 0) || (LCDT == 4)
// for playoffCompare
static Schedule		*poSchedPtr;
static DYN_U_8 poStage;

static RosPlayer::Pos	genPos [Roster::MAX_DRAFT_LIST] =
								{
									RosPlayer::POS_QB, RosPlayer::POS_RB,
									RosPlayer::POS_REC, RosPlayer::POS_OL,
									RosPlayer::POS_DL, RosPlayer::POS_LB,
									RosPlayer::POS_DB, RosPlayer::POS_K,
									RosPlayer::POS_P,
								};
#endif

#if	(LCDT == 0)
static DYN_16	talentCounts [League::CONFIG_30 + 1] [Roster::MAX_DRAFT_LIST] =
					{
						//	 qb	rb	 rec	 ol	dl	  lb	 db	 k		p
						#if	MAN_45
						{	 22,	41,  65,	 73,	54,  60,	 67,	12,  12	},	 //  8 teams
						{	 28,	51,  81,	 91,	68,  75,	 84,	14,  14	},	 // 10 teams
						{	 34,	61,  97, 110,	81,  90, 101,	16,  16	},	 // 12 teams
						{	 51,	91, 146, 164, 122, 136, 151,	22,  22	},	 // 18 teams
						{	 80, 142, 227, 255, 189, 211, 235,	32,  32	},	 // 28 teams
						{	 86, 152, 243, 273, 203, 226, 252,	34,  34	},	 // 30 teams
						#else
						{	 32,	56,  90, 101,	76,  85,	 94,	13,  13	},	 //  8 teams
						{	 40,	70, 113, 126,	95, 106, 118,	16,  16	},	 // 10 teams
						{	 48,	84, 136, 151, 114, 127, 142,	19,  19	},	 // 12 teams
						{	 72, 126, 203, 227, 171, 191, 212,	29,  29	},	 // 18 teams
						{	112, 196, 315, 355, 266, 297, 329,	45,  45	},	 // 28 teams
						{	120, 210, 338, 380, 285, 318, 353,	48,  48	},	 // 30 teams
						#endif
					};

static DYN_8	newLeagueName [] = "New League",
					proBowlName [] = "Pro Bowl",
					ltapeText [] = "TAPES\\GAME",
					*templateFile [] =
					{
						"ltpl08.lgt",
						"ltpl10.lgt",
						"ltpl12.lgt",
						"ltpl18.lgt",
						"ltpl28.lgt",
						"ltpl30.lgt",
						"ltpl02.lgt",
					};
#endif


PoolAlloc	*LeagData::pa = NULL,		 // where to get memory from
				*League::pa = NULL;			 // where to get memory from
FILE			*League::logFP = NULL;
Calender		League::calender;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
	---------------------------------------------------------------------
	NAME:			League

	FUNCTION:	League object CONSTRUCTOR
					there are several different type depending on what you
					want to do:

					no parameters: just create space for a league (will be filled
					in later by user)

	INPUT:		pName = path name (optional)
					fName = which league file to read
					fullLoad = just league data, or all the conferences,
					divisions & teams too

	RETURNS:		-> created League object
	---------------------------------------------------------------------
*/

League::League ()
	// GJW: 9-15-1994, I know the maximum numbers,
	// and this is the last product, so pre-allocate the nodes
	:	confList (2, (DYN_16) 0),
		teamList (MAX_TEAMS, (DYN_16) 0)
{
	memset (&pList, 0, sizeof (pList) +
								sizeof (name) +
								sizeof (trophy) +
								sizeof (path) +
								sizeof (fileName));

	data = new LeagData;

	data->stage = STAGE_SEASON_START;
	data->newFlag = TRUE;
	data->saveEnable = TRUE;

	defaultRules (&data->rules);
}

League::League (DYN_8 *fName, FullLoad fullLoad)
	// GJW: 9-15-1994, I know the maximum numbers,
	// and this is the last product, so pre-allocate the nodes
	:	confList (2, (DYN_16) 0),
		teamList (MAX_TEAMS, (DYN_16) 0)
{
	data = new LeagData;
	Load (fName, fullLoad);
}

League::League (DYN_8 *pName, DYN_8 *fName, FullLoad fullLoad)
	// GJW: 9-15-1994, I know the maximum numbers,
	// and this is the last product, so pre-allocate the nodes
	:	confList (2, (DYN_16) 0),
		teamList (MAX_TEAMS, (DYN_16) 0)
{
	data = new LeagData;
	Load (pName, fName, fullLoad);
}

/*
	---------------------------------------------------------------------
	NAME:			~League

	FUNCTION:	League object DESTRUCTOR

	INPUT:		-> League object

	RETURNS:		all allocations freed
	---------------------------------------------------------------------
*/

League::~League ()
{
	if (data->fpPlayers)
	{
		data->fpPlayers->Close ();
		delete data->fpPlayers;
	}

	// fullLoad == _ALL will have this full list mechanism intact
	if (!confList.start ())
	{
		while (TRUE)
		{
			delete confList ();
			if (++confList)
				break;
		}
	}

	// fullLoad = _TEAMS
	else if (!teamList.start ())
	{
		while (TRUE)
		{
			Team	*tPtr = (Team *) teamList.linkout ();

			if (tPtr)
				delete tPtr;

			else
				break;
		}
	}

	#if	(LCDT == 0)
	DeleteChamps ();
	#endif

	#if	(LCDT == 0) || (LCDT == 4)
	if (data->statFile)
		delete data->statFile;
	#endif

	delete data->schedule;

	#if	(LCDT == 0)
	delete data->cdpPtr;
	delete data->fapPtr;
	#endif

	delete data;
}

/*
	---------------------------------------------------------------------
	NAME:			new/delete

	FUNCTION:	the memory allocation/deallocation overloading functions
					(I have overloaded so that memory can come from a block rather
					than piece by piece)

	INPUT:		size & ptr = mandatory parameters for overloading the
					new/delete operators

	RETURNS:		-> memory allocations
	---------------------------------------------------------------------
*/

void *
League::operator new (size_t size)
{
	// if the size if wrong (indicating a derived class),
	// allocate out of conventional memory
	vmAssert (pa && (size == sizeof (League)));
	void	*t = pa->New ();
	vmAssert (t);

	memset (t, 0, sizeof (League));
	return t;
}

void
League::operator delete (void *t, size_t size)
{
	// only process items that are of the expected size
	vmAssert (pa && (size == sizeof (League)));
	pa->Delete (t);
}



void FAR *
LeagData::operator new (size_t size)
{
	// if there is no allocation object, or the size if wrong (indicating
	// a derived class), allocate out of conventional memory
	// only process items that are of the expected size
	vmAssert (pa && (size == sizeof (LeagData)));

	void	FAR *t = pa->New ();
	vmAssert (t);

	// return a 'clean slate' to the constructor
	memset (t, 0, sizeof (LeagData));
	return t;
}

void
LeagData::operator delete (void FAR *t, size_t size)
{
	// only process items that are of the expected size
	vmAssert (pa && (size == sizeof (LeagData)));

	pa->Delete (t);
}






/*
	=====================================================================
									PUBLIC METHODS
	=====================================================================
*/

/*
	---------------------------------------------------------------------
	NAME:			Init

	FUNCTION:	before any league methods may be called,
					the system needs to create its memory handlers
					ALL sub-objects (confs, divisions, team, rosters, etc.)
					are really only pointers to data (just like the League).
					They all have their operator new () re-assigned to allocate
					out of this 'global' pool. (Their own internal data is
					allocated separately, since the individual data size is not
					know. I KNOW that all sub-objects are of size struct pointer.)

	INPUT:		none

	RETURNS:		PoolAlloc object for League created
						(pointer is passed to League, and below)
					PoolAlloc object for LeagData created
	---------------------------------------------------------------------
*/

void
League::Init ()
{
	pa = new PoolAlloc (sizeof (League), NUM_ALLOC);

	LeagData::pa = new PoolAlloc (sizeof (LeagData), NUM_ALLOC);
	vmAssert (pa && LeagData::pa);

	// continue on down the chain of command
	Conference::Init ();
	Division::Init ();
	Team::Init ();

	RosPlayer::Init ();
}
void
League::DeInit ()
{
	delete pa;

	delete LeagData::pa;

	// continue on down the chain of command
	Conference::DeInit ();
	Division::DeInit ();
	Team::DeInit ();

	RosPlayer::DeInit ();
}

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			LoadTeam

	FUNCTION:	load a team from the given league, without creating
					a whole league

	INPUT:		pName = path name (optional)
					fName = which league file to read
					tId = which team to load

	RETURNS:		-> created team
	---------------------------------------------------------------------
*/

Team *League::LoadTeam (DYN_8 *pName, DYN_8 *fName, DYN_16 tId)
{
	DYN_8		fullName [MAXPATH];
	DYN_8		planfilename [MAXPATH];
	Team		*tPtr = 0;

	if (tId)
	{
		makeFileName (fullName, pName, fName, lgeText);
		makeFileName (planfilename, pName, fName, lg2Text );

		ReadBFile	rlf;
		sysAssert (rlf.Open (fullName), EXITMSG_LEAGUE_LOAD);
		sysAssert (rlf.Find (L02Text, 1) != -1L, EXITMSG_LEAGUE_BFIND);

		tPtr = new Team (planfilename, rlf, tId);
		vmAssert (tPtr);
	}

	return tPtr;
}

/*
	---------------------------------------------------------------------
	NAME:			SetTradeLog

	FUNCTION:	allow user control of whether trades are recorded to disk

	INPUT:		value = TRUE/FALSE to set flag to

	RETURNS:		logTrades flag set
	---------------------------------------------------------------------
*/

void League::SetTradeLog (const DYN_16 value)
{
	if (value)
		OpenLog ();

	else
		CloseLog ();
}

/*
	---------------------------------------------------------------------
	NAME:			PFNameValid

	FUNCTION:	determine if the given file is in fact a league file

	INPUT:		pName-> path
					fName-> filename

	RETURNS:		TRUE/FALSE if file is a league
					(i.e. does it pass the vm_isbfile test)
	---------------------------------------------------------------------
*/

DYN_16
League::PFNameValid (DYN_8 *pName, DYN_8 *fName)
{
	return DOSFILE::exists (makeFileName (0, pName, fName, lgeText));
}

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Loadable

	FUNCTION:	open the file & check for the openning "L02:" block
					(user may have some old "LGE:" block files lying about)
					also, no all star league may ever be loaded via loading box

	INPUT:		fName -> string of file to open

	RETURNS:		TRUE/FALSE if file is ok
	---------------------------------------------------------------------
*/

DYN_16
League::Loadable (const DYN_8 *fName)
{
	DYN_16	ok = FALSE;
	FILE		*fp = my_fopen ((DYN_8 *) fName, "rb");
	if (fp)
	{
		DYN_8		buffer [5];
		buffer [4] = 0;
		my_fread (buffer, sizeof (DYN_8), 4, fp);
		ok = !strcmp (buffer, L02Text);

		// league is new, now make sure it's not an all star league
		if (ok)
		{
			// read in size of file (4)
			// plus the configuration type (1)
			my_fread (buffer, sizeof (DYN_8), 5, fp);
			ok = (buffer [4] != CONFIG_2);
		}

		my_fclose (fp);
	}

	return ok;
}

/*
	---------------------------------------------------------------------
	NAME:			BuildTeamList

	FUNCTION:	return list of team pointers in League/Conference/Division
					based on number of parameters

	INPUT:		tList -> where list goes (if 0, just return count)
					none (return list of all teams in League)
					confIndex (return list of all teams in Conference)
					divIndex (return list of all teams in Division)

	RETURNS:		number of teams in list
					list set with team pointers
	---------------------------------------------------------------------
*/

DYN_16
League::BuildTeamList (GenList<Team> *tList)
{
	// if the league's list already exists, then just scan through it,
	// rather than by each conference/division/team

	DYN_16	numTeams = teamList.size ();
	if (numTeams)
	{
		// only fill in list if 1 is provided
		if (tList)
		{
			teamList.start ();

			while (numTeams--)
			{
				if (tList->linkin (teamList ()))
					break;	// if destination list is full, stop
				++teamList;
			}
		}
	}

	else
	{
		DYN_16	numConfs = data->numConfs;
		for (DYN_8 conf = 0; conf < numConfs; conf++)
		{
			Conference	*cPtr = GetConf (conf);
			DYN_8			numDiv = cPtr->GetNumSub ();

			for (DYN_8 div = 0; div < numDiv; div++)
			{
				Division *dPtr = cPtr->GetDiv (div);
				DYN_8		numTeam = dPtr->GetNumSub ();

				for (DYN_8 team = 0; team < numTeam; team++)
					if (tList->linkin (dPtr->GetTeam (team)))
						break;	// if destination list is full, stop
			}
		}
	}

	return teamList.size ();
}

#if	(LCDT != 3)
DYN_16
League::BuildTeamList (GenList<Team> *tList, DYN_8 cIndex)
{
	Conference		*cPtr;
	if (cPtr = GetConf (cIndex))
	{
		DYN_8 numDiv = cPtr->GetNumSub ();

		for (DYN_8 div = 0; div < numDiv; div++)
		{
			Division *dPtr = cPtr->GetDiv (div);
			DYN_8		numTeam = dPtr->GetNumSub ();

			for (DYN_8 team = 0; team < numTeam; team++)
				if (tList->linkin (dPtr->GetTeam (team)))
					break;	// if destination list is full, stop
		}
	}

	return tList->size ();
}
#endif

#if	(LCDT == 0)
DYN_16
League::BuildTeamList (GenList<Team> *tList, DYN_8 cIndex, DYN_8 dIndex)
{
	Conference		*cPtr;

	if (cPtr = GetConf (cIndex))
	{
		Division		*dPtr;
		if (dPtr = cPtr->GetDiv (dIndex))
		{
			DYN_8		numTeam = dPtr->GetNumSub ();

			for (DYN_8 team = 0; team < numTeam; team++)
				if (tList->linkin (dPtr->GetTeam (team)))
					break;	// if destination list is full, stop
		}
	}

	return tList->size ();
}

/*
	---------------------------------------------------------------------
	NAME:			BuildIRTeamList

	FUNCTION:	return list of team pointers in League that still have
					players on their injured reserve slots

	INPUT:		tList -> where list goes

	RETURNS:		number of teams in list
					list set with team pointers
	---------------------------------------------------------------------
*/

DYN_16
League::BuildIRTeamList (GenList<Team> *tList)
{
	teamList.start ();

	while (TRUE)
	{
		Team		*tPtr = teamList ();
		Roster	*rPtr = LoadPlayers (tPtr);

		if (rPtr->GetNumIRPlayers ())
			if (tList->linkin (tPtr))
				break;	// if destination list is full, stop

		if (++teamList)
			break;
	}

	return tList->size ();
}

/*
	---------------------------------------------------------------------
	NAME:			BuildTCTeamList

	FUNCTION:	return list of team pointers in League that still have
					not copmleted training camp allocations

	INPUT:		tList -> where list goes

	RETURNS:		number of teams in list
					list set with team pointers
	---------------------------------------------------------------------
*/

DYN_16
League::BuildTCTeamList (GenList<Team> *tList)
{
	teamList.start ();

	while (TRUE)
	{
		Team		*tPtr = teamList ();

		if (!tPtr->TrainAlloc ())
			if (tList->linkin (tPtr))
				break;	// if destination list is full, stop

		if (++teamList)
			break;
	}

	return tList->size ();
}

/*
	---------------------------------------------------------------------
	NAME:			BuildTradeTeamList

	FUNCTION:	return list of team pointers in League that are interested
					in obtaining the given player, for a another player of the
					given position

	INPUT:		tList -> where list goes
					tId = id of team looking for a trade
					pPtr-> player(s) willing to part with
					pos = position(s) wanted in return

	RETURNS:		number of teams in list
					list set with team pointers
	---------------------------------------------------------------------
*/

DYN_16
League::BuildTradeTeamList (GenList<Team> *tList,
										DYN_16 tId,
										RosPlayer **pPtrs, Roster::SlotId *sIds)
{
	teamList.start ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();

		// don't try to trade with yourself
		if (tPtr->GetId () != tId)
		{
			tPtr->LoadPlayers (data->fpPlayers);

			// human teams are always acceptable,
			// computer teams must want to
			if ((tPtr->GetManage () == OWNER_HUMAN) ||
					tPtr->WannaTrade (0, sIds, pPtrs))
			{
				if (tList->linkin (tPtr))
					break;	// if destination list is full, stop
			}
			else
				tPtr->DeletePlayers ();
		}

		if (++teamList)
			break;
	}

	return tList->size ();
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			SortTeamList

	FUNCTION:	given a list created by one of the above functions, sort
					the list via a shell-sort based on some Team info

	INPUT:		tList -> list to sort
					LeagueSort = type of sort desired

	RETURNS:		list sorted based on type of sort wanted
	---------------------------------------------------------------------
*/

void
League::SortTeamList (GenList<Team> *tList, Sort sortType)
{
	if (sortType == SORT_RECORD)
		GenerateRecords (FALSE);

	// allocate space for comparision results store
	compTable = new char [COMP_TABLE_SIZE];
	memset (compTable, 0, COMP_TABLE_SIZE);

	poSchedPtr = data->schedule;
	poStage = data->stage;
	ListShellSort ((zDList *) tList, sortType, compareFunc);

	if (compTable)
		delete [] compTable;
}
#endif	// LCDT

/*
	---------------------------------------------------------------------
	NAME:			GetConf, Div, Team

	FUNCTION:	return pointer to particular subsection of the league

	INPUT:		indexes to conference, division, team

	RETURNS:		-> class desired
	---------------------------------------------------------------------
*/

Conference *
League::GetConf (DYN_8 cIndex)
{
	Conference	*cPtr = 0;

	if (cIndex < data->numConfs)
	{
		SetConfPtr (cIndex);
		cPtr = data->conf;
	}

	return cPtr;
}

Division *
League::GetDiv (DYN_8 cIndex, DYN_8 dIndex)
{
	Division *dPtr = 0;

	if (cIndex < data->numConfs)
	{
		SetConfPtr (cIndex);
		dPtr = data->conf->GetDiv (dIndex);
	}

	return dPtr;
}

Team *
League::GetTeam (DYN_8 cIndex, DYN_8 dIndex, DYN_8 tIndex)
{
	Team	*tPtr = 0;

	if (cIndex < data->numConfs)
	{
		SetConfPtr (cIndex);
		tPtr = data->conf->GetTeam (dIndex, tIndex);
	}

	return tPtr;
}

Team *
League::GetTeam (DYN_8 id)
{
	Team					*tPtr = 0;
	GenCursor<Team>	tCursor (&teamList);

	while (TRUE)
	{
		Team		*getPtr = tCursor ();
		if (getPtr->GetId () == id)
		{
			tPtr = getPtr;
			break;
		}

		if (++tCursor)
			break;
	}

	return tPtr;
}

/*
	---------------------------------------------------------------------
	NAME:			GetSchedule

	FUNCTION:	return a pointer to the league's schedule

	INPUT:		none

	RETURNS:		-> schedule object
	---------------------------------------------------------------------
*/

Schedule *
League::GetSchedule ()
{
	return data->schedule;
}

/*
	---------------------------------------------------------------------
	NAME:			GetCalender

	FUNCTION:	return a pointer to the league's calender

	INPUT:		none

	RETURNS:		-> calender object
	---------------------------------------------------------------------
*/

Calender *
League::GetCalender ()
{
	return &calender;
}

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			Get/CloseStats

	FUNCTION:	open/close the league's ISAM handler

	INPUT:		writeFlag = TRUE/FALSE if data is going to be updated
									(or just read only)
					statFile-> maybe already opened ISAM stuff

	RETURNS:		-> StatFile ISAM interface object
	---------------------------------------------------------------------
*/

StatFile *
League::GetStats (DYN_8 writeFlag)
{
	if (!data->statFile)
	{
		data->statFile = new StatFile (path, fileName, writeFlag);
		vmAssert (data->statFile);
	}

	return data->statFile;
}

void
League::CloseStats ()
{
	if (data->statFile)
	{
		delete data->statFile;
		data->statFile = 0;
	}
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			GetNumSub

	FUNCTION:	get the number of Conferences in this league

	INPUT:		none

	RETURNS:		data->numConfs
	---------------------------------------------------------------------
*/

DYN_8
League::GetNumSub ()
{
	return data->numConfs;
}

/*
	---------------------------------------------------------------------
	NAME:			Get/SetData

	FUNCTION:	return data of League/Conference/Division/Team based on
					number of parameters

					had to cast things as unsigned chars since it assumed
					all the enums were DYN_16s:

					i.e. it thought id was a DYN_16 since it is of type
					LeagueConfig, so it would just return [si], not
					mov	al, [si]
					xor	ah, ah

	INPUT:		none (return League Data)
					confIndex (return Division Data)
					divIndex (return Division Data)
					teamIndex (return Team Data)

	RETURNS:		->name desired
	---------------------------------------------------------------------
*/

League::Config
League::GetConfig () const
{
	return data->config;
}

void
League::SetConfig (const Config value)
{
	data->config = value;
}

League::Type
League::GetTOL () const
{
	return data->tol;
}

void
League::SetTOL (const Type value)
{
	data->tol = value;
}

DYN_U_8
League::GetStage () const
{
	return data->stage;
}

void
League::SetStage (DYN_U_8 value)
{
	data->stage = value;
}

League::FileType
League::GetFileType () const
{
	return data->fileType;
}

void
League::SetFileType (const FileType value)
{
	data->fileType = value;
}

DYN_16
League::GetInception () const
{
	return data->inception;
}

DYN_16
League::GetNumSeasons () const
{
	return data->numSeasons;
}

League::Byes
League::GetByes () const
{
	return data->byes;
}

void
League::SetByes (const Byes value)
{
	data->byes = value;
}

QtrLen
League::GetQtr () const
{
	return data->rules.qtrLength;
}

void
League::SetQtr (const QtrLen value)
{
	data->rules.qtrLength = value;
}

DYN_16
League::GetFatigue () const
{
	return data->rules.fatigue;
}

void
League::SetFatigue (const DYN_16 value)
{
	data->rules.fatigue = (DYN_8) value;
}

DYN_16
League::GetFumbles () const
{
	return data->rules.fumbles;
}

void
League::SetFumbles (const DYN_16 value)
{
	data->rules.fumbles = (DYN_8) value;
}

DYN_16
League::GetInjuries () const
{
	return data->rules.injuries;
}

void
League::SetInjuries (const DYN_16 value)
{
	data->rules.injuries = (DYN_8) value;
}

DYN_16
League::GetInts () const
{
	return data->rules.ints;
}

void
League::SetInts (const DYN_16 value)
{
	data->rules.ints = (DYN_8) value;
}

DYN_16
League::GetPenalties () const
{
	return data->rules.penalties;
}

void
League::SetPenalties (const DYN_16 value)
{
	data->rules.penalties = (DYN_8) value;
}

DYN_16
League::Get45Sec () const
{
	return data->rules.clock45;
}

void
League::Set45Sec (const DYN_16 value)
{
	data->rules.clock45 = (DYN_8) value;
}

DYN_16
League::GetFastSim () const
{
	return data->rules.fastSim;
}

void
League::SetFastSim (const DYN_16 value)
{
	data->rules.fastSim = (DYN_8) value;
}

DYN_16
League::GetNFLValid () const
{
	return data->nflFlag;
}

void
League::SetNFLValid (const DYN_16 value)
{
	#if	0
	// this flag can only be changed from TRUE to FALSE,
	// then this league is no longer NFL valid
	if (data->nflFlag)
		data->nflFlag = value;
	#endif
}

/*
	---------------------------------------------------------------------
	NAME:			Get/SetData

	FUNCTION:	return string name of League/Conference/Division/Team
					based on number of parameters

					(first parameter is string info wanted)

	INPUT:		none (return League Name)
					confIndex (return Division Name)
					divIndex (return Division Name)
					teamIndex (return Team Name)

	RETURNS:		->name desired
	---------------------------------------------------------------------
*/

DYN_8 *
League::GetName () const
{
	strcpy ((DYN_8 *) name, data->name);
	return (DYN_8 *) name;
}

void
League::SetName (const DYN_8 *string)
{
	if (strlen (string) >= NAME_LENGTH)
	{
		strncpy (data->name, string, NAME_LENGTH);
		data->name [NAME_LENGTH - 1] = 0;
	}

	else
		strcpy (data->name, string);
}

DYN_8 *
League::GetTrophy () const
{
	strcpy ((DYN_8 *) trophy, data->trophy);
	return (DYN_8 *) trophy;
}

void
League::SetTrophy (const DYN_8 *string)
{
	strncpy (data->trophy, string, TROPHY_NAME_LENGTH);
	data->trophy [TROPHY_NAME_LENGTH - 1] = 0;
}

#if	0
DYN_8 *
League::GetCommPn () const
{
	return (DYN_8 *) commPN;
}

void
League::SetCommPn (const DYN_8 *string)
{
	strncpy (commPN, string, Team::PN_NAME_LENGTH);
	commPN [Team::PN_NAME_LENGTH - 1] = 0;
}
#endif

DYN_8 *
League::GetPath () const
{
	return (DYN_8 *) path;
}

void
League::SetPath (const DYN_8 *string)
{
	strcpy (path, string);
	strcpy (data->path, string);
}

DYN_8 *
League::GetFName () const
{
	return (DYN_8 *) fileName;
}

void
League::SetFName (const DYN_8 *string)
{
	strncpy (fileName, string, MAXFILE);
	fileName [MAXFILE - 1] = 0;

	strncpy (data->fileName, string, MAXFILE);
	data->fileName [MAXFILE - 1] = 0;
}


#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			Save

	FUNCTION:	write the blockified file to disk

	INPUT:		this -> league to update
					optional file name

	RETURNS:		fiel written to disk
	---------------------------------------------------------------------
*/

void
League::Save ()
{
	Save (fileName);
}

void
League::Save (DYN_8 *fName)
{
	if (data->saveEnable)
	{
		#if	(LCDT == 0)
		if (data->newFlag)
		{
			data->newFlag = FALSE;

			AssignDraftProfileNames ();
			NewLeagueFiles ();

			LoadPlayers ((Team *) 0);
			SavePlayers ();
			DYN_16	posCounts [Roster::MAX_DRAFT_LIST];
			DeterminePlayerCounts (posCounts);
			DeletePlayers ();

			// now make & save any free agents to round out the league
			GenerateFAPlayers (posCounts);
			SavePlayers ();
			DeleteDraftList (DRAFT_FREE_AGENT);
		}
		#endif

		DYN_8		newBuff [MAXPATH],
					planBuff [MAXPATH],
					oldBuff [MAXPATH];

		if (!fileName [0])
		{
			strcpy (fileName, fName);
			strcat (fileName, lgeText);

			strcpy (data->fileName, fName);
			strcat (data->fileName, lgeText);
		}

		makeFileName (newBuff, path, fName, xxxText);
		makeFileName (oldBuff, path, fName, lgeText);
		makeFileName (planBuff, path, fName, lg2Text );

		WriteBFile	wlf;
		if (wlf.Open (newBuff))
		{
			Save (planBuff,wlf);

			// no schedules for PROBOWL leagues
			if (data->tol != TYPE_PROBOWL)
				data->schedule->Save (wlf);

			wlf.Close ();
			updateFile (newBuff, oldBuff);
		}
	}
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			ComputerManage

	FUNCTION:	'globally' let all teams that have computer manager turned
					on re-shuffle their rosters

	INPUT:		flag = TRUE/FALSE if computer managed teams should shuffle

	RETURNS:		all computer managed teams organize their rosters
	---------------------------------------------------------------------
*/

void
League::ComputerManage (const DYN_16 flag)
{
	if (flag == OWNER_COMPUTER)
	{
		if (!teamList.start ())
		{
			while (TRUE)
			{
				Team		*tPtr = teamList ();
				LoadPlayers (tPtr);

				tPtr->Manage (MANAGE_ROSTER, MANAGE_OWNERSHIP);

				DeletePlayers (tPtr);
				if (++teamList)
					break;
			}
		}
	}
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			GenerateRecords

	FUNCTION:	generate the won/loss/ties for all the teams

	INPUT:		forceFlag = should teams re-calculate their records, or
									use any that may be there
					teamList-> list of team pointers in this league

	RETURNS:		all team's records generated
	---------------------------------------------------------------------
*/

void
League::GenerateRecords (DYN_16 forceFlag)
{
	// determine all teams' records
	DYN_16	numTeams = teamList.size ();

	if (numTeams)
	{
		teamList.start ();
		while (numTeams--)
		{
			Team	*tPtr = teamList ();

			// team will re-calculate its record if wins == -1,
			// so to force, set wins appropriately
			if (forceFlag)
				tPtr->SetWins (-1);

			tPtr->GenerateRecord (data->schedule);
			++teamList;
		}
	}
}

/*
	---------------------------------------------------------------------
	NAME:			Sequence

	FUNCTION:	user input may have changed the stage the league is in,
					the hoped-for stage is passed in, if the league can get
					to there it will
					(for example, clicking on the StartNewSeason button, will
					result in Sequence (STAGE_START) being called. If
					the current stage of the league is STAGE_PLAYOFFS_END,
					then all of the league startup will occur, and the next
					stage will be progessed to)

	INPUT:		ls = hoped-for next stage

	RETURNS:		TRUE/FALSE whether stage occured
	---------------------------------------------------------------------
*/

DYN_16
League::Sequence (DYN_U_8 ls)
{
	DYN_U_8 oldStage = data->stage;

	#if	(LCDT == 0)
	switch (ls)
	{

		case STAGE_START:
			Start ();
			break;

		case STAGE_CDRAFT_START:
			CDraftStart ();
			break;

		case STAGE_CDRAFT_PAUSE:
			CDraftPause ();
			break;

		case STAGE_CDRAFT_END:
			CDraftEnd ();
			break;

		case STAGE_SDRAFT_START:
			SDraftStart ();
			break;

		case STAGE_SDRAFT_PAUSE:
			SDraftPause ();
			break;

		case STAGE_SDRAFT_END:
			SDraftEnd ();
			break;

		case STAGE_TRAINING_START:
			TrainingStart ();
			break;

		case STAGE_TRAINING_END:
			TrainingEnd ();
			break;

		case STAGE_SEASON_START:
			SeasonStart ();
			break;

		case STAGE_SEASON_RUNNING:
			SeasonRunning ();
			break;

		case STAGE_SEASON_END:
			SeasonEnd ();
			break;

		case STAGE_PLAYOFFS_START:
			PlayoffStart ();
			break;

		case STAGE_PLAYOFFS_END:
			PlayoffEnd ();
			break;
	}
	#endif

	if (data->stage != oldStage)
		Save ();

	return data->stage == ls;
}

/*
	---------------------------------------------------------------------
	NAME:			DetermineStage

	FUNCTION:	determine what phase of the season the league is in,
					and update the schedule week if appropriate

	INPUT:		League object

	RETURNS:		Schedule updated if week's worth of games done
	---------------------------------------------------------------------
*/

void
League::DetermineStage ()
{
}

/*
	---------------------------------------------------------------------
	NAME:			GameComplete

	FUNCTION:	after a game has been completed, save the league data,
					and determine if a new week has been progressed to

	INPUT:		wIndex = week number of game (0 based)
					gIndex = game number of week (0 based)
					pfH, pfV = points for home, visitor
					otFlag = overTime indicator

	RETURNS:		league file updated
	---------------------------------------------------------------------
*/

void
League::GameComplete (DYN_8 wIndex, DYN_8 gIndex,
								DYN_8 pfH, DYN_8 pfV,
								DYN_8 otFlag)
{
	// get the week that THIS game is taking place in
	DYN_16	lastWeek = data->schedule->DetermineWeek ();
	data->schedule->GameComplete (wIndex, gIndex, pfH, pfV, otFlag);

	// get the week that the NEXT game will take place in
	DYN_16	curWeek = data->schedule->DetermineWeek ();

	// schedule returns -1 when last playoff game has been played
	if (curWeek < 0)
		PlayoffEnd ();

	else
	{
		// determine if another week has been advanced to
		if (curWeek > lastWeek)
		{
			// then update all team win/loss/tie data
			GenerateRecords (FALSE);

			// check if in playoff games
		  if (curWeek >= data->schedule->GetWData (SCDWEEKDATA_NUM_REGWEEKS, 0))
				PlayoffStart ();
		}
	}
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			GenerateNewSchedule

	FUNCTION:	call the schedule object to generate next year's version

	INPUT:		byes = should byes be included in 28 team schedule

	RETURNS:		new schedule generated
	---------------------------------------------------------------------
*/

void
League::GenerateNewSchedule (Byes byes)
{
	data->schedule->GenerateNew (byes, data->config);
}

/*
	---------------------------------------------------------------------
	NAME:			MakePool

	FUNCTION:	take every player off every roster, and put into a big
					draft pool

	INPUT:		generate = new order to make?
					callBack-> func to call with currently worked on team
					every team, plus the free agent list

	RETURNS:		free agent list is HUGE
	---------------------------------------------------------------------
*/

void
League::MakePool (DYN_16 generate, V_CCFP_FUNC callBack)
{
	LoadDraftList (DRAFT_FREE_AGENT, FALSE);

	if (data->fapPtr)
	{
		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();
			Roster	*rPtr = LoadPlayers (tPtr);

			if (callBack)
				callBack (tPtr->GetName ());

			rPtr->PoolPlayers (*data->fapPtr);

			if (++teamList)
				break;
		}

		data->fapPtr->Save (path, fileName);
		DeletePlayerList ();
		DeleteDraftList (DRAFT_FREE_AGENT);

		data->schedule->ClearCurWeek ();
		data->stage = STAGE_SDRAFT_START;
		Save ();
		DeletePlayers ();

		if (generate)
		{
			Draft draft;
			draft.Generate ((GenList<DYN_16> *) &teamList, 0);
			draft.Reset ();
			draft.Save (path, fileName);
		}
	}
}

#endif	// LCDT
/*
	--------------------------------------------------------
	NAME:			Compare

	FUNCTION:	determine if the league given by the path & name, is the
					same as this one

	INPUT:		cmpPath, cmpFileName -> strings of a league that will be
					loaded if this doesn't match this object

	RETURNS:		TRUE/FALSE if the same
	---------------------------------------------------------------------
*/

DYN_16
League::Compare (DYN_8 *cmpPath, DYN_8 *cmpFileName)
{
	DYN_16	value = FALSE;

	if (!stricmp (cmpPath, path))
		value = !stricmp (cmpFileName, fileName);

	return value;
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			DefLeagueFileSetup

	FUNCTION:	when INSTALL.exe runs, it unpacks _defleag.*, which is
					to be used as the default data for all future leagues.
					However, the user needs a league to start with, so
					all the relevant files are copied to dynamix.*

	INPUT:		if !exist dynamix.lge, make the league files

	RETURNS:		dynamix.*
	---------------------------------------------------------------------
*/

void
League::DefLeagueFileSetup ()
{
	if (!DOSFILE::exists (makeFileName (0, thisDirText, dynText, lgeText)))
	{
		DYN_8		destFName [MAXPATH];
		DYN_16	copied = TRUE;

		// .lgeE;

		// .lge
		makeFileName (destFName, thisDirText, dynText, lgeText);
		copied &= copyFile (destFName, makeFileName (0, thisDirText, defLeagueFile, lgeText));

		// .lgc
		copied &= LeagueChamps::Copy (destFName,
									thisDirText, dynText,
									thisDirText, defLeagueFile);

		// .pyr
		makeFileName (destFName, thisDirText, dynText, pyrText);
		copied &= copyFile (destFName, makeFileName (0, thisDirText, defLeagueFile, pyrText));

		// .pyf
		makeFileName (destFName, thisDirText, dynText, pyfText);
		copied &= copyFile (destFName, makeFileName (0, thisDirText, defLeagueFile, pyfText));

		sysAssert (copied, EXITMSG_LEAGUE_DEFFILES);
	}
}

/*
	---------------------------------------------------------------------
	NAME:			MakeCopy

	FUNCTION:	write this league out to the destination drive

	INPUT:		drive-> string containing drive letter

	RETURNS:		league files written to disk
	---------------------------------------------------------------------
*/

void
League::MakeCopy (DYN_8 *drive)
{
	FileType saveFile = data->fileType;

	if (saveFile == FILETYPE_ORIGINAL)
		saveFile = FILETYPE_MASTER;

	data->fileType = FILETYPE_COPY;

	DYN_8			saveDrive [2];
	saveDrive [0] = path [0];
	saveDrive [1] = path [2];

	path [0] = *drive;
	path [2] = 0;

	Save ();

	path [0] = saveDrive [0];
	path [2] = saveDrive [1];
	data->fileType = saveFile;
}

/*
	---------------------------------------------------------------------
	NAME:			ConfDivSetup

	FUNCTION:	when creating a new league, create the conference and
					division organization for a particular type
					read in the template file, creating conferences,
					divisions and teams as they are encountered

	INPUT:		League Object-> config

	RETURNS:		conferences & divisions created
	---------------------------------------------------------------------
*/

void
League::ConfDivSetup ()
{
//	  struct dosdate_t today;
//	  _dos_getdate (&today);
	_SYSTEMTIME stToday;
	GetSystemTime( &stToday );

	ReadBFile	rlf;
	sysAssert (rlf.Open (templateFile [data->config]) > 0, EXITMSG_LEAGUE_CREATE);
	sysAssert (rlf.Find (LGTText, 1) != -1L, EXITMSG_LEAGUE_CREATE);
	DYN_8			numConfs,
					cIds [16];
	rlf.Read (&numConfs, 1);
	rlf.Read (cIds, numConfs);

	data->numConfs = numConfs;
	for (DYN_8 i = 0; i < numConfs; i++)
	{
		Conference	*cPtr = new Conference (NULL, rlf, cIds [i], TRUE);
		vmAssert (cPtr);
		confList.linkin (cPtr);
	}

	rlf.Close ();

	data->fileType = FILETYPE_MASTER;
//	  data->inception = today.year;
	data->inception = (DYN_16)stToday.wYear;
	strcpy (data->name, newLeagueName);

	BuildTeamList (&teamList);
	ConfDivPlayersSetup ();
	
	// if league is compatible with current season, use same trophy
	// 12/10/96 mcqueen
	if( IsCurrentSeasonCompatible() )
	{
		League nflLeague( nflLeagueFile, LOAD_NONE );
		SetTrophy( nflLeague.GetTrophy() );
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SchedSetup

	FUNCTION:	now that the new league has been created (teams)
					create a schedule to go along with it

	INPUT:		League Object

	RETURNS:		schedule created
	---------------------------------------------------------------------
*/

void
League::SchedSetup ()
{
	data->schedule = new Schedule (this);
	vmAssert (data->schedule);
}

/*
	---------------------------------------------------------------------
	NAME:			StealTeam

	FUNCTION:	when setting up a new league, the user may choose
					a team from any league for any position
					this will set the team with the current id to the passed in
					team

	INPUT:		cId, dId, tId = which team to set data for
					lPtr-> league data will come from
					tPtr-> team data will come from

	RETURNS:		all team's data taken from srcTeam
	---------------------------------------------------------------------
*/

void
League::StealTeam (DYN_16 cId, DYN_16 dId, DYN_16 tId,
								League *lPtr, Team *tPtr)
{
//	  FILE *fh;

//	  char *myStr;
//	  DYN_16 myId;
//	  Roster *myRPtr;

	Team	*destTeam = GetTeam ((DYN_8)cId, (DYN_8)dId, (DYN_8)tId);
	destTeam->Steal (lPtr->path, lPtr->fileName, tPtr, STEAL_ALL);
}

/*
	---------------------------------------------------------------------
	NAME:			AssignDraftProfileNames

	FUNCTION:	Assigns each computer owned team a randomly selected
					default draft profile name.

	INPUT:		League Object

	RETURNS:		Each computer owned team's draft profile name has been
					randomly selected and set.
	---------------------------------------------------------------------
*/

void
League::AssignDraftProfileNames ()
{
	DYN_16	numTeams = teamList.size ();

	if (numTeams)
	{
		DYN_8 dpName [MAXFILE];

		teamList.start ();
		while (numTeams--)
		{
			Team *tPtr = teamList();

			// if source team didn't have one, assign one randomly
			// (of course, the team could have an INVALID one, but that's
			// another matter)
			tPtr->GetDraftProfileName (dpName);
			if (!dpName [0])
			{
				DraftProfile::GetRandomDefault (dpName);
				tPtr->SetDraftProfileName (dpName);
			}

			++teamList;
		}
	}
}
#endif	// LCDT

/*
	---------------------------------------------------------------------
	NAME:			LoadRoster

	FUNCTION:	re-read the team's roster info

	INPUT:		tPtr->team to re-load roster info for

	RETURNS:		team's roster initialized again
	---------------------------------------------------------------------
*/

void
League::LoadRoster (Team *tPtr)
{
	DYN_8		*ptr = makeFileName (0, path, fileName, lgeText);

	ReadBFile	rlf;
	Open (rlf, ptr);
	tPtr->LoadRoster (rlf);
}

//#if	  (LCDT != 3)
/*
	---------------------------------------------------------------------
	NAME:			Load/Delete/SavePlayers

	FUNCTION:	loading a team's roster first is controlled by the
					league (who keeps track of the file open/close management)

	INPUT:		team pointer or id
					(if none, load ALL team's rosters)

	RETURNS:		-> team's roster's players loaded
	---------------------------------------------------------------------
*/

void
League::LoadPlayers ()
{
	teamList.start ();

	while (TRUE)
	{
		LoadPlayers (teamList ());

		if (++teamList)
			break;
	}
}

Roster *
League::LoadPlayers (DYN_8 tId)
{
	Team	*tPtr = GetTeam (tId);

	if (tPtr)
		return LoadPlayers (tPtr);

	else
		return 0;
}
//#endif

Roster *
League::LoadPlayers (Team *tPtr)
{
	if (!data->fpPlayers)
	{
		data->fpPlayers = new PlayerRdWr ();
		vmAssert (data->fpPlayers);
	}

	if (!data->fpPlayers->IsOpen ())
	{
		DYN_8		buffer [MAXPATH];
		makeFileName (buffer, path, fileName, pyrText);
		sysAssert (data->fpPlayers->Open (buffer), EXITMSG_LEAGUE_ROSTER);
	}

	if (tPtr)
		return tPtr->LoadPlayers (data->fpPlayers);

	else
		return 0;
}

#if	(LCDT != 3)
void
League::DeletePlayers ()
{
	teamList.start ();

	while (TRUE)
	{
		DeletePlayers (teamList ());

		if (++teamList)
			break;
	}
}

void
League::DeletePlayers (DYN_8 tId)
{
	Team	*tPtr = GetTeam (tId);

	if (tPtr)
		DeletePlayers (tPtr);
}

void
League::DeletePlayers (Team *tPtr)
{
	if (tPtr)
		tPtr->DeletePlayers ();
}

void
League::SavePlayers ()
{
	if (data->fpPlayers && data->fpPlayers->IsOpen ())
	{
		// write out everybody loaded in memory
		short		pId = PlayerRdWr::BASE_PLAYER_ID;

		// first teams
		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();
			Roster	*rPtr = tPtr->LoadPlayers (0);

			if (rPtr)
				pId += rPtr->SavePlayers (*data->fpPlayers);

			#if	DEBUG
			saveMonoUpdate ((DYN_U_16) pId, (DYN_U_16) data->nextPlayerId);
			#endif

			if (++teamList)
				break;
		}

		// then college & free-agent lists
		if (data->cdpPtr)
		{
			pId += data->cdpPtr->SavePlayers (*data->fpPlayers);

			#if	DEBUG
			saveMonoUpdate ((DYN_U_16) pId, (DYN_U_16) data->nextPlayerId);
			#endif
		}

		if (data->fapPtr)
		{
			pId += data->fapPtr->SavePlayers (*data->fpPlayers);

			#if	DEBUG
			saveMonoUpdate ((DYN_U_16) pId, (DYN_U_16) data->nextPlayerId);
			#endif
		}
	}
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			AssignedFull

	FUNCTION:	check every team's roster for one that doesn't have
					every assigned slot filled
					(if the team is computer managed, they will fill the slots)

					GJW: 8-26-1993, only check those teams that have a
					game to play
					i.e. a team may have lost a player to CEI, or released
					an assigned player to sign a free agent, after the
					last week of the season, or after playing their game
					everyone else who has to play (playoff teams, or those
					with games still to go this week), must wait for this
					team to set up its roster, even though it doesn't have
					to play anymore

	INPUT:		teamList = list of teams to check

	RETURNS:		TRUE/FALSE if all teams are ready to play
	---------------------------------------------------------------------
*/

DYN_16
League::AssignedFull ()
{
	DYN_16			afFlag = TRUE;

	if ((data->stage >= STAGE_SEASON_START) && (data->stage < STAGE_PLAYOFFS_END))
	{
		// process all the teams
		// worst records go first, best go last (so start at end of list,
		// and go backwards)
		// when the playoffs start, sorting takes a long time,
		// and only a handful of teams are involved, so don't bother
		// sorting
		if (data->stage < STAGE_PLAYOFFS_START)
			SortTeamList (&teamList, SORT_RECORD);

		if (!teamList.end ())
		{
			// don't bother if season is over
			DYN_16	curWeek = data->schedule->GetWData (SCDWEEKDATA_CUR_WEEK, 0);

			if (curWeek != -1)
			{
				while (TRUE)
				{
					Team		*tPtr = teamList ();

					// don't bother checking teams that don't have games
					DYN_16	gameNum = data->schedule->FindTeamsGame ((DYN_8)(tPtr->GetId ()), (DYN_8)curWeek);
					if (gameNum != -1)
					{
						// or those already finished
						if (data->schedule->GetSGData (SGDATA_STATUS) != SCHED_DONE)
						{
							// if team has holes in its roster,
							if (!tPtr->AssignedFull ())
							{
								// fill them if computer managed
								if (tPtr->GetManage () == OWNER_COMPUTER)
								{
									// if free agents not yet loaded, bring 'em on down
									if (!pList [0])
										LoadDraftList (DRAFT_FREE_AGENT, TRUE);

									FillAssigned (tPtr);
								}

								// otherwise report some team isn't ready
								else
									afFlag = FALSE;
							}
						}
					}

					if (--teamList)
						break;
				}
			}
		}

		// if any computer managed team needed free agents, delete them now
		if (data->fapPtr)
		{
			data->fapPtr->Save (path, fileName);
			DeletePlayerList ();
			DeleteDraftList (DRAFT_FREE_AGENT);
		}
	}

	return afFlag;
}

/*
	---------------------------------------------------------------------
	NAME:			FillAssigned

	FUNCTION:	make all rosters fill players that are missing

	INPUT:		teamList = list of teams in this league

	RETURNS:		any open assigned slots are filled for ALL teams
	---------------------------------------------------------------------
*/

void
League::FillAssigned ()
{
	LoadDraftList (DRAFT_FREE_AGENT, TRUE);

	// process all the teams
	// worst records go first, best go last (so start at end of list,
	// and go backwards)
	SortTeamList (&teamList, SORT_RECORD);
	teamList.end ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		DYN_16	manage = tPtr->GetManage ();
		tPtr->SetManage (OWNER_COMPUTER);

		if (!tPtr->AssignedFull ())
			FillAssigned (tPtr);

		if (manage == OWNER_HUMAN)
			tPtr->SetManage ((DYN_8)manage);

		if (--teamList)
			break;
	}

	// delete draftee lists
	DeletePlayerList ();

	// free agent pool (never know who was signed/released)
	if (data->fapPtr)
		data->fapPtr->Save (path, fileName);

	DeleteDraftList (DRAFT_FREE_AGENT);

	Save ();
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			GenerateFAPoolPlayer

	FUNCTION:	when rosters are being forced to fill empty holes via
					FillAssigned (), it's possible that a computer teams needs
					a player, but the free agent pool is empty
					make a player of that type, and put him into the list

	INPUT:		pos = position of player to generate (GENERAL or SPECIFIC)
					listIndex = which list to stick player on

	RETURNS:		->created player
					free agent pool 1 player larger
	---------------------------------------------------------------------
*/

void
League::GenerateFAPoolPlayer (const RosPlayer::Pos pos, const DYN_16 listIndex)
{
	DYN_8		fnBuffer [PlayerRdWr::NAME_LEN + 1],
				lnBuffer [PlayerRdWr::NAME_LEN + 1];
	fnBuffer [0] = lnBuffer [0] = 0;

	{
		NamePlayer	ANP;
		ANP.GetName (fnBuffer, lnBuffer);
	}

	// add generated player to free agent list
	RosPlayer	*pPtr = data->fapPtr->GeneratePlayer (pos, data->nextPlayerId++, fnBuffer, lnBuffer);
	pPtr->Save (*data->fpPlayers);

	// GJW: 1-13-1994, make sure new player's data is available in file,
	// the RosPlayer 1779 occurs when a new player is put onto the end of the
	// file, and the computer is rebooted, or turned off, without the file
	// being flushed
	data->fpPlayers->Close ();
	DYN_8		buffer [MAXPATH];
	makeFileName (buffer, path, fileName, pyrText);
	sysAssert (data->fpPlayers->Open (buffer), EXITMSG_LEAGUE_ROSTER);

	// and local if it exists
	if (pList [0])
		pList [listIndex]->linkin (pPtr);
}

/*
	---------------------------------------------------------------------
	NAME:			Load/Delete/SavePlayersLW

	FUNCTION:	load LAST WEEK's roster and players (before this week's
					trades and free agents)
					this is so the boxscore screens can show stats for the
					team AS IT WAS, when it generated the stats
					(after the games are completed, teams alter their rosters
					by trading and free agents. When the box score checks the
					database for stats for each player on the team, it must know
					who was on the team WHEN IT WAS PLAYED, not who's on the team
					now)

	INPUT:		tPtr-> team to load roster for

	RETURNS:		->newly created roster with old players
	---------------------------------------------------------------------
*/

Roster *
League::LoadPlayersLW (Team *tPtr)
{
	Roster	*rPtr = 0;

	// try to open Last Week's file (contains only roster info (player id's))
	ReadBFile	rlf;

	// if file exists, load last week's ids
	if (rlf.Open (makeFileName (0, path, fileName, rstText)))
	{
		rPtr = new Roster (rlf, tPtr->GetId ());
		vmAssert (rPtr);
		rlf.Close ();
		rPtr->LoadPlayers (data->fpPlayers);
	}

	// otherwise use the current
	else
		rPtr = LoadPlayers (tPtr);

	return rPtr;
}

void
League::DeletePlayersLW (Roster *rPtr)
{
	delete rPtr;
}

void
League::SavePlayersLW ()
{
	WriteBFile	wlf;

	Open (wlf, makeFileName (0, path, fileName, rstText));

	if (!teamList.start ())
	{
		while (TRUE)
		{
			Team	*tPtr = teamList ();
			tPtr->SavePlayersLW (wlf);

			if (++teamList)
				break;
		}
	}
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			Load/SaveDraft

	FUNCTION:	load or save the Draft object (i.e. the one that controls the
					drafting order)

	INPUT:		none

	RETURNS:		-> Draft object
					(caller must cast appopropriately)
	---------------------------------------------------------------------
*/

Draft *
League::LoadDraft ()
{
	return new Draft (path, fileName);
}

void
League::SaveDraft (Draft *draftPtr)
{
	draftPtr->Save (path, fileName);
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			Load/DeleteDraftList

	FUNCTION:	load/delete one of the pools of draftable players

					GJW: 10-15-1994, be smart in pre-allocating the
					number of nodes

	INPUT:		dType = college or free agents
					fillFlag = should pList be filled?

	RETURNS:		-> group of players asked for
					(caller must cast appopropriately)
					pList filled in
	---------------------------------------------------------------------
*/

void *
League::LoadDraftList (DraftType dType, DYN_16 fillFlag)
{
	RosPlayerList	*rplPtr = 0;

	switch (dType)
	{
		#if	(LCDT == 0)
		case DRAFT_COLLEGE:
			if (!data->cdpPtr)
				data->cdpPtr = new CollegeDraftPool (path, fileName, *data->fpPlayers);
			rplPtr = (RosPlayerList *) data->cdpPtr;
			break;
		#endif

		case DRAFT_SUP:
		case DRAFT_FREE_AGENT:
			#if	DEBUG_PLIST
			debugPrintf ("--- new FreeAgentList ---\n");
			#endif

			if (!data->fapPtr)
				data->fapPtr = new FreeAgentPool (path, fileName, *data->fpPlayers);
			rplPtr = (RosPlayerList *) data->fapPtr;

			#if	DEBUG_PLIST
			debugPrintf ("--- new FreeAgentList done ---\n");
			#endif
			break;
	}

	// fill in the list sets if playerList loaded, and user's lists are
	// empty (i.e. each list is a group of players that play a particular
	// position)
	if (rplPtr && fillFlag && !pList [0])
	{
		#if	(LCDT == 0)
		DYN_16			*tcPtr = talentCounts [data->config];

		#endif

		RosPlayer::Pos *ppPtr = genPos;
		for (DYN_8 i = 0; i < Roster::MAX_DRAFT_LIST; i++, ppPtr++)
		{
			#if	(LCDT == 0)
			// GJW: 12-12-1994, players can't be added to college draft
			// (by teams releasing players), so allocate just the right amount
			DYN_16	nP = dType == DRAFT_COLLEGE ?
								rplPtr->BuildPlayerList (0, *ppPtr) :
								(DYN_16)((DYN_32 (*tcPtr) * 120) / 100L);

			#else
			DYN_16	nP = rplPtr->BuildPlayerList (0, *ppPtr);

			#endif

			// nodes come out of far, so requirements are less tight
			#if	OBSOLETE
					// get number of players in the list at this general position
					DYN_16	nP = rplPtr->BuildPlayerList (0, *ppPtr);

					#if	(LCDT == 0)
					// then figure out how much 'extra' space to allocate
					// if user is in college draft:
					//		if this is the college list, nP is right on, because user
					//		can't add anyone to the the list
					//		if this is the supplemental list, then pad each position
					//		by 10% for any team members that may be released
					if ((data->stage >= STAGE_CDRAFT_START) &&
							(data->stage < STAGE_CDRAFT_END))
					{
						if (dType != DRAFT_COLLEGE)
							nP = (110 * nP) / 100;
					}

					// if in supplemental draft, pad 5%
					else if ((data->stage >= STAGE_SDRAFT_START) &&
							(data->stage < STAGE_SDRAFT_END))
					{
						nP = (105 * nP) / 100;

						if (nP < *tcPtr)
							nP = *tcPtr;
					}

					// if in middle of season, add in a handful
					else
						nP += 30;
					#endif

					#if	DEBUG_PLIST
					debugPrintf ("--- pos list [%d] = %d---\n", i, nP);
					#endif
			#endif

			pList [i] = new GenList<RosPlayer> (nP, (DYN_16) 0);
			vmAssert (pList [i]);
			rplPtr->BuildPlayerList (pList [i], *ppPtr);
			RosPlayerList::SortPlayerList (pList [i]);

			#if	(LCDT == 0)
			tcPtr++;

			#endif

			#if	DEBUG_PLIST
			debugPrintf ("--- pos list [%d] done ---\n", i);
			#endif
		}
	}

	return (void *) rplPtr;
}

void
League::DeleteDraftList (DraftType dType)
{
	switch (dType)
	{
		#if	(LCDT == 0)
		case DRAFT_COLLEGE:
			if (data->cdpPtr)
			{
				delete data->cdpPtr;
				data->cdpPtr = 0;
			}
			break;
		#endif

		case DRAFT_SUP:
		case DRAFT_FREE_AGENT:
			if (data->fapPtr)
			{
				delete data->fapPtr;
				data->fapPtr = 0;
			}
			break;
	}
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			GetPlayerList/s

	FUNCTION:	return a pointer to the array of lists, or a list based
					on position

	INPUT:		index = which disk to return pointer to

	RETURNS:		-> array, or 1 element
	---------------------------------------------------------------------
*/

GenList<RosPlayer> *
League::GetPlayerList (DYN_16 index)
{
	return pList [index];
}

GenList<RosPlayer> **
League::GetPlayerLists ()
{
	return pList;
}

/*
	---------------------------------------------------------------------
	NAME:			DeletePlayerFromList

	FUNCTION:	player has been chosen during draft, so take him out
					of one of the lists

	INPUT:		pList-> which list
					pPtr-> which players

	RETURNS:		pList 1 smaller
	---------------------------------------------------------------------
*/

void
League::DeletePlayerFromList (GenList<RosPlayer> *pList, RosPlayer *pPtr)
{
	if (!pList->start ())
	{
		while (TRUE)
		{
			RosPlayer	*rpPtr = (*pList) ();
			if (rpPtr == pPtr)
			{
				pList->linkout ();
				break;
			}

			if (++(*pList))
				break;
		}
	}
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			DeletePlayerList

	FUNCTION:	delete the pList array of players
					don't bother if first list not loaded (none of them will be)

	INPUT:		pList = array of players lists

	RETURNS:		pList 1 smaller
	---------------------------------------------------------------------
*/

void
League::DeletePlayerList ()
{
	if (pList [0])
	{
		for (DYN_8 i = 0; i < Roster::MAX_DRAFT_LIST; i++)
			delete pList [i];

		pList [0] = 0;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			TradePreview

	FUNCTION:	a human owner is proposing or accepting a trade
					make sure that the players can be swapped between these
					2 teams:
					if (tp == TRADEPREV_MYOPEN) it's being proposed:
						both teams are human managed, or
						both players play the same position, or
						human's player is OPEN (computer will make roster adjustments
							if it likes the trade)
					otherwise:

	INPUT:		tId1, pId1, tId2, pId2 = team & player identifiers
														(proposer/proposee)
					tp = what to look for (trade is about to be proposed or accepted)

	RETURNS:		_OK if trade will work
					_MYOPEN or _OTHEROPEN if it won't
	---------------------------------------------------------------------
*/

League::TradePrev
League::TradePreview (DYN_16 t1Id, DYN_16 *p1Ids,
								DYN_16 t2Id, DYN_16 *p2Ids,
								TradePrev tp)
{
	Team	*t1Ptr = GetTeam ((DYN_8) t1Id),
			*t2Ptr = GetTeam ((DYN_8) t2Id);

	// this owner is PROPOSING a trade
	if (tp == TRADEPREV_MYOPEN)
	{
		// both teams are human owned (t1 is GUARANTEED to be)
		if (t2Ptr->GetManage () == OWNER_HUMAN)
			tp = TRADEPREV_OK;

		else
		{
			Roster		*r1Ptr = LoadPlayers (t1Ptr),
							*r2Ptr = LoadPlayers (t2Ptr);

			for (DYN_16 i = 0; i < TradeInfo::MAX_PIDS; i++)
			{
				DYN_16		p1Id = *(p1Ids + i),
								p2Id = *(p2Ids + i);

				// 'unbalanced' positions will be ok, so only check those
				// that are player-for-player
				if (p1Id && p2Id)
				{
					RosPlayer	*p1Ptr = r1Ptr->GetPlayer (p1Id),
									*p2Ptr = r2Ptr->GetPlayer (p2Id);
					DYN_16		p1Status = p1Ptr->GetStatus ();

					if (
							// check if both players play the same position
							(p1Ptr->GetPosGen () == p2Ptr->GetPosGen ()) ||

							// check if human's player is NOT ASSIGNED
							((p1Status == Roster::OPEN_SLOT) ||
							(p1Status == Roster::INACTIVE_SLOT))
						)
						tp = TRADEPREV_OK;
				}
			}
		}
	}

	// this owner is ACCEPTING a trade
	else
	{
		Roster	*r1Ptr = LoadPlayers (t1Ptr),
					*r2Ptr = LoadPlayers (t2Ptr);

		for (DYN_16 i = 0; i < TradeInfo::MAX_PIDS; i++)
		{
			DYN_16		p1Id = *(p1Ids + i),
							p2Id = *(p2Ids + i);

			// 'unbalanced' positions will be ok, so only check those
			// that are player-for-player
			if (p1Id && p2Id)
			{
				RosPlayer	*p1Ptr = r1Ptr->GetPlayer (p1Id),
								*p2Ptr = r2Ptr->GetPlayer (p2Id);

				// check if both players play the same position
				if (p1Ptr->GetPosGen () == p2Ptr->GetPosGen ())
					tp = TRADEPREV_OK;

				// if not, they'll have to be open (if proposer is a computer team,
				// it will adjust its roster accordingly to work the trade)
				else
				{
					DYN_16	status1 = p1Ptr->GetStatus (),
								status2 = p2Ptr->GetStatus ();

					// if trade involves 2 humans, both players will have to be open
					// check if human's player is OPEN
					// t2 MUST be, since they are the proposee, trying to accept
					if (t1Ptr->GetManage () == OWNER_HUMAN)
					{
						if ((status1 == Roster::OPEN_SLOT) || (status1 == Roster::INACTIVE_SLOT))
						{
							if ((status2 == Roster::OPEN_SLOT) || (status2 == Roster::INACTIVE_SLOT))
								tp = TRADEPREV_OK;

							else
								tp = TRADEPREV_MYOPEN;
						}
					}

					// if t2 is COMPUTER, then THIS team's player must be open
					else
					{
						if ((status2 == Roster::OPEN_SLOT) || (status2 == Roster::INACTIVE_SLOT))
							tp = TRADEPREV_OK;

						else
							tp = TRADEPREV_MYOPEN;
					}
				}
			}

			// an 'Unbalanced' trade, so side getting more players, must
			// have enough holes
			else if (p1Id)
			{
				// ACCEPTING (Human) team must have enough holes
				if (!r2Ptr->UnBalancedTrade (p1Ids, i, r1Ptr))
					tp = TRADEPREV_MYUNBALANCED;

				break;
			}

			else if (p2Id)
			{
				// PROPOSING (Human or Computer) team must have enough holes
				if ((t1Ptr->GetManage () == OWNER_HUMAN) &&
						!r1Ptr->UnBalancedTrade (p2Ids, i, r2Ptr))
					tp = TRADEPREV_OTHERUNBALANCED;

				break;
			}
		}
	}

	return tp;
}

/*
	---------------------------------------------------------------------
	NAME:			TradePlayers

	FUNCTION:	2 players are being swapped between teams or free agent
					list
					execute the roster moves

	INPUT:		tId1, pId1, tId2, pId2 = team & player identifiers
														(proposer/proposee)
					status = trade proposed/accepted/rejected

					if tId1 == -1 or 0, then the team is releasing a player TO the
					free agent list during the college or supplemental draft
						(i.e. signing a college or sup. draft choice)
					if p1Id is also 0, then the (computer managed) team is releasing
					someone, without getting another

					if tId2 == 0, then the team is choosing a player FROM the
					free agent list while releasing a player on the team (if
					pId1 is 0, they are just adding)

	RETURNS:		both team's rosters updated
	---------------------------------------------------------------------
*/

TradeStatus
League::TradePlayers (DYN_16 t1Id, DYN_16 *p1Ids,
								DYN_16 t2Id, DYN_16 *p2Ids,
								TradeStatus status)
{
	#if	(LCDT == FULL_LCDT)
	if (logFP)
		fprintf (logFP, "%2d, (%4d %4d %4d) FOR %2d, (%4d %4d %4d)%s\n",
								t1Id, *p1Ids, *(p1Ids + 1), *(p1Ids + 2),
								t2Id, *p2Ids, *(p2Ids + 1), *(p2Ids + 2),
								tradeText [status]);
	#endif

	TradeStatus		inStatus = status;

	// college/supplemental/free agent stuff
	if ((t1Id <= 0) || !t2Id)
	{
		DYN_16		listIndex;
		Team			*tPtr;
		Roster		*rPtr;
		RosPlayer	*p2Ptr = 0,
						*p1Ptr = 0;
		DYN_16		p1Id = *p1Ids,
						p2Id = *p2Ids;

		// remember if THIS CALL TO THIS ROUTINE loaded the free agents
		FreeAgentPool	*fPtr = data->fapPtr;
		if (!fPtr)
			LoadDraftList (DRAFT_FREE_AGENT, FALSE);

		// player TO free agent list while signing college draftee
		if (t1Id <= 0)
		{
			tPtr = GetTeam ((DYN_8) t2Id);
			vmAssert (tPtr);
			rPtr = tPtr->LoadPlayers (data->fpPlayers);
			p1Ptr = rPtr->GetPlayer (*p2Ids);

			// if t1Id == -1, team wants a college puke to take roster spot
			if (t1Id < 0)
			{
				p2Ptr = data->cdpPtr->GetPlayer (p1Id);
				data->cdpPtr->RemovePlayer (p2Ptr);

				#if	(LCDT == FULL_LCDT)
				if (logFP)
				{
					fprintf (logFP, "	  %Fs Grabs College: %s, %Fs %Fs\n",
									tPtr->GetName (),
									getSpecPosText (p2Ptr->GetPosSpec ()),
									p2Ptr->GetFirstName (),
									p2Ptr->GetLastName ());

					if (p2Id)
						fprintf (logFP, "	  Releases: %s %Fs %Fs\n",
									getSpecPosText (p1Ptr->GetPosSpec ()),
									p1Ptr->GetFirstName (),
									p1Ptr->GetLastName ());
					else
						fprintf (logFP, "	  Releases: no one\n");
				}
				#endif
			}

			// if t1Id == 0, team is releasing 1 free agent while signing another
			else
			{
				#if	(LCDT == FULL_LCDT)
				if (logFP)
				{
					if (p2Id)
						fprintf (logFP, "	  %Fs Releases: %s %Fs %Fs\n",
									tPtr->GetName (),
									getSpecPosText (p1Ptr->GetPosSpec ()),
									p1Ptr->GetFirstName (),
									p1Ptr->GetLastName ());
					else
						fprintf (logFP, "	  %Fs Releases: no one\n",
									tPtr->GetName ());
				}
				#endif

				// computer managed teams can just release to free agent list
				// without getting somebody in return
				if (p1Id)
				{
					p2Ptr = data->fapPtr->GetPlayer (p1Id);
					data->fapPtr->RemovePlayer (p2Ptr);

					#if	(LCDT == FULL_LCDT)
					if (logFP)
						fprintf (logFP, "	  Grab FA: %s %Fs %Fs\n",
										getSpecPosText (p2Ptr->GetPosSpec ()),
										p2Ptr->GetFirstName (),
										p2Ptr->GetLastName ());
					#endif
				}
			}
		}

		// player FROM free agent list
		else
		{
			tPtr = GetTeam ((DYN_8) t1Id);
			vmAssert (tPtr);
			rPtr = tPtr->LoadPlayers (data->fpPlayers);
			p1Ptr = rPtr->GetPlayer (p1Id);
			p2Ptr = data->fapPtr->GetPlayer (p2Id);

			data->fapPtr->RemovePlayer (p2Ptr);

			// update local list as well, by removing player
			listIndex = rPtr->PosToSlot ((RosPlayer::Pos) p2Ptr->GetPosGen ());
			pList [listIndex]->linkout ();

			// only record free-agent signings during the season
			if (data->stage > STAGE_SDRAFT_END)
			{
				// create a temporary if league hasn't started one itself
				TradeNews	*tn = data->tradeNews;
				if (!tn)
				{
					tn = new TradeNews (path, fileName);
					vmAssert (tn);
				}

				tn->SetTrade (t1Id, p1Ids, t2Id, p2Ids, inStatus);

				// if league didn't have one loaded originally, delete my temp
				if (!data->tradeNews)
					delete tn;
			}

			#if	(LCDT == FULL_LCDT)
			if (logFP)
				fprintf (logFP, "	  %Fs Grabs FA: %s %Fs %Fs\n",
								tPtr->GetName (),
								getSpecPosText (p2Ptr->GetPosSpec ()),
								p2Ptr->GetFirstName (),
								p2Ptr->GetLastName ());
			#endif
		}

		// update the team list
		if (p2Ptr || p1Ptr)
		{
			tPtr->AddPlayer (p2Ptr, p1Ptr);
			Save ();

			#if	(LCDT == FULL_LCDT)
			// update the college list
			if (t1Id < 0)
				data->cdpPtr->Save (path, fileName);
			#endif

			// no injured players can go to free agency, so heal them
			// (computer teams may release them if they are the only available
			// player)
			if (p1Ptr->GetInjPts ())
			{
				p1Ptr->SeasonReset (TRUE);
				p1Ptr->Save (*data->fpPlayers);
			}

			RosPlayer::Pos pos = p1Ptr->GetPosGen ();

			// update the free agent list
			data->fapPtr->AddPlayer (p1Ptr);
			if (data->saveEnable)
				data->fapPtr->Save (path, fileName);

			// update local lists as well
			// add released-from-team player to fa list
			// (don't add a released free agent to the college list)
			if ((t1Id >= 0) &&
				pList [0] &&
				(pos != RosPlayer::POS_NONE))
			{
				listIndex = rPtr->PosToSlot (pos);
				pList [listIndex]->linkin (p1Ptr);
				RosPlayerList::SortPlayerList (pList [listIndex]);
			}
		}

		// if this routine was responsible for load,
		// make sure to delete
		if (!fPtr)
			DeleteDraftList (DRAFT_FREE_AGENT);

		// any player moving to the free agent list, cannot be part of
		// a trade anymore
		TradeNews	*tn = data->tradeNews;
		if (!tn)
		{
			tn = new TradeNews (path, fileName);
			vmAssert (tn);
		}

		if (tn->KillPTrades (p1Id))
			tn->Save (path, fileName);

		// if league didn't have one loaded originally, delete my temp
		if (!data->tradeNews)
			delete tn;
	}

	#if	(LCDT == FULL_LCDT)
	// team to team
	else
	{
		// create a temporary if league hasn't started one itself
		TradeNews	*tn = data->tradeNews;
		if (!tn)
		{
			tn = new TradeNews (path, fileName);
			vmAssert (tn);
		}

		if (inStatus != TRADE_DELETED)
		{
			Team			*t1Ptr = GetTeam ((DYN_8) t1Id),
							*t2Ptr = GetTeam ((DYN_8) t2Id);
			Roster		*r1Ptr = LoadPlayers (t1Ptr),
							*r2Ptr = LoadPlayers (t2Ptr);

			RosPlayer	*p1Ptrs [TradeInfo::MAX_PIDS],
							*p2Ptrs [TradeInfo::MAX_PIDS],
							*pPtr;

			for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
			{
				DYN_16	p1Id = p1Ids [i],
							p2Id = p2Ids [i];

				if (p1Id)
					pPtr = r1Ptr->GetPlayer (p1Id);
				else
					pPtr = 0;

				p1Ptrs [i] = pPtr;

				if (p2Id)
					pPtr = r2Ptr->GetPlayer (p2Id);
				else
					pPtr = 0;

				p2Ptrs [i] = pPtr;
			}

			// if suggesting a trade
			if (inStatus == TRADE_PROPOSED)
			{
				status = t2Ptr->TradeEval (p1Ptrs, p2Ptrs);

				// if computer A.I. accepts it
				if (status != TRADE_PROPOSED)
				{
					// don't reload the temp tn when recursing
					TradeNews	*saveTN = data->tradeNews;
					data->tradeNews = tn;

					// come here again with new status
					TradePlayers (t1Id, p1Ids, t2Id, p2Ids, status);
					data->tradeNews = saveTN;
				}
			}
			else if (inStatus == TRADE_ACCEPTED)
			{
				// GJW: 8-27-1996
				// do this the brute-force way, trade each player one at a time
				for (DYN_32 i = 0; i < TradeInfo::MAX_PIDS; i++)
				{
					RosPlayer	*p1Ptr = p1Ptrs [i],
									*p2Ptr = p2Ptrs [i];

					if (p1Ptr || p2Ptr)
					{
						t1Ptr->AddPlayer (p2Ptr, p1Ptr);
						t2Ptr->AddPlayer (p1Ptr, p2Ptr);
						Save ();

						if (logFP)
							fprintf (logFP, "	  %Fs %s %Fs %Fs FOR %Fs %s %Fs %Fs [%d]\n",
											t1Ptr->GetName (),
											getSpecPosText (p1Ptr->GetPosSpec ()),
											p1Ptr->GetFirstName (),
											p1Ptr->GetLastName (),

											t2Ptr->GetName (),
											getSpecPosText (p2Ptr->GetPosSpec ()),
											p2Ptr->GetFirstName (),
											p2Ptr->GetLastName (),
											i);
					}
				}
			}
		}

		// trade can go from PROPOSED to ACCEPTED/REJECTED recursively
		// (i.e. computer managed team responds to the trade immediately),
		// so don't save the status twice (i.e. PROPOSED, then ACCEPTED),
		// just wait for final answer
		if (inStatus == status)
		{
			tn->SetTrade (t1Id, p1Ids, t2Id, p2Ids, inStatus);

			if (data->saveEnable)
				tn->Save (path, fileName);
		}

		// if league didn't have one loaded originally, delete my temp
		if (!data->tradeNews)
			delete tn;
	}

	if (logFP && (inStatus != TRADE_PROPOSED))
		fprintf (logFP, "completed\n\n");
	#endif

	return status;
}

/*
	---------------------------------------------------------------------
	NAME:			MakePlayer

	FUNCTION:	create a player object, without association with any
					any team or college/free agent list

	INPUT:		id of player to create

	RETURNS:		-> created player
					player's status set to -1 indicating caller must DELETE this
					object
	---------------------------------------------------------------------
*/

RosPlayer *
League::MakePlayer (const DYN_16 id)
{
	RosPlayer	*pPtr = new RosPlayer (*data->fpPlayers, id);
	pPtr->SetStatus (-1);
	return pPtr;
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			FindPlayer

	FUNCTION:	try to find the pointer to a player by searching all the
					rosters, college draft & free agent lists

	INPUT:		id = player to find
					loadFlag = TRUE/FALSE if player should be loaded if
					not found in memory already (if TRUE, and player has not been
					found, it will be the responsiblity of the caller to delete
					the player object thus created (the player's status will
					be set to -1, rather than the usual roster slot assignment)

	RETURNS:		-> found player
					if 0, the player has not yet been loaded from disk
	---------------------------------------------------------------------
*/

RosPlayer *
League::FindPlayer (const DYN_16 id, const DYN_16 loadFlag)
{
	RosPlayer	*pPtr = 0;

	// first see if the player is on a team
	teamList.start ();

	while (TRUE)
	{
		Team			*tPtr = teamList ();
		Roster		*rPtr = tPtr->LoadPlayers (0);

		// only search if this team's players have been loaded in memory
		if (rPtr)
		{
			pPtr = rPtr->GetPlayer (id);
			if (pPtr)
				break;
		}

		if (++teamList)
			break;
	}

	#if	(LCDT == 0)
	// if not found on any team, try the free agent lists and college
	if (!pPtr && data->fapPtr)
		pPtr = data->fapPtr->GetPlayer (id);

	if (!pPtr && data->cdpPtr)
		pPtr = data->cdpPtr->GetPlayer (id);

	// finally, if the player still hasn't been found, and he NEEDS to be,
	// load in from disk, and indicate so
	if (!pPtr && loadFlag)
		MakePlayer (id);
	#endif

	return pPtr;
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			LoadTrades

	FUNCTION:	create a TradeNews object associated with this league

	INPUT:		-> League object

	RETURNS:		-> TradeNews object associated with this league
	---------------------------------------------------------------------
*/

TradeNews *
League::LoadTrades ()
{
	return new TradeNews (path, fileName);
}

/*
	---------------------------------------------------------------------
	NAME:			CountTrades

	FUNCTION:	count the number of trades this week of the given status

	INPUT:		tradeNews-> trade news

	RETURNS:		number of trades of the given status
	---------------------------------------------------------------------
*/

DYN_16
League::CountTrades (TradeStatus status)
{
	TradeNews	*tn = data->tradeNews;

	if (!tn)
	{
		tn = new TradeNews (path, fileName);
		vmAssert (tn);
	}

	DYN_16		nt = tn->BuildTradeList (0, status, 0);

	// if league didn't have one loaded originally, delete my temp
	if (!data->tradeNews)
		delete tn;

	return nt;
}

/*
	---------------------------------------------------------------------
	NAME:			KillProposedTrades

	FUNCTION:	change status of all pending trades to rejected
					(user decided not to act on them)

	INPUT:		tradeNews-> trade news

	RETURNS:		number of trades of the given status
	---------------------------------------------------------------------
*/

void
League::KillProposedTrades ()
{
	TradeNews	*tn = data->tradeNews;
	if (!tn)
	{
		tn = new TradeNews (path, fileName);
		vmAssert (tn);
	}
	tn->KillProposed ();
	tn->Save (path, fileName);

	// if league didn't have one loaded originally, delete my temp
	if (!data->tradeNews)
		delete tn;
}

/*
	---------------------------------------------------------------------
	NAME:			GetTNews

	FUNCTION:	load and return pointer to team news for given team

	INPUT:		tId = which team to get data for

	RETURNS:		-> team News object
	---------------------------------------------------------------------
*/

TeamNews *
League::GetTNews (DYN_16 tId)
{
	return new TeamNews (path, fileName, tId);
}

/*
	---------------------------------------------------------------------
	NAME:			FlushTrades

	FUNCTION:	delete last week's trade file

	INPUT:		tradeNews-> trade news

	RETURNS:		.trn file deleted
	---------------------------------------------------------------------
*/

void
League::FlushTrades () const
{
	TradeNews::Flush (path, fileName);
}

/*
	---------------------------------------------------------------------
	NAME:			WeeklyRosterMoves

	FUNCTION:	After key periods (drafting, training camp, or a game)
					teams update their rosters by trading, signing free agents
					or putting/taking players on/off injured reserve

	INPUT:		stage of league (trading invalid at certain points)

	RETURNS:		all computer managed teams do any moves
	---------------------------------------------------------------------
*/

void
League::WeeklyRosterMoves (V_CCFP_FUNC callBack)
{
	#if	WEEKLY_MOVES
	#if	WEEKLY_STATS
	// update STATS for the week
	GenList<STRAT_STAT>	sList;
	STRAT_STAT				*ssPtr = (STRAT_STAT *)
									new DYN_8 [sizeof (STRAT_STAT) * teamList.size ()];
	setStatRankings (&teamList, data->statFile, &sList, ssPtr);
	#endif

	mono.printf (40, 6, "%-16s", "drafting");

	// prepare for FREE-AGENT list handling
	LoadDraftList (DRAFT_FREE_AGENT, TRUE);

	// then process all the teams
	// worst records go first, best go last (so start at end of list,
	// and go backwards)
	data->saveEnable = FALSE;
	SortTeamList (&teamList, SORT_RECORD);
	teamList.end ();

	// change in DL handling,
	// since a player is on DL FOREVER,
	// the roster will need to know if a player is hurt longer than
	// the games left to play
	DYN_16	curWeek = data->schedule->GetWData (SCDWEEKDATA_CUR_WEEK, 0),
				numReg = data->schedule->GetWData (SCDWEEKDATA_NUM_REGWEEKS, 0),
				midPoint = (numReg) / 2,
				weeksToGo = numReg - curWeek;
	if (weeksToGo < 0)
		weeksToGo = 0;
	Roster::SetWeeksToGo (weeksToGo);

	while (TRUE)
	{
		Team		*tPtr = teamList ();
		DYN_8		FAR *name = tPtr->GetName ();
		if (callBack)
			callBack (name);
		mono.printf (40, 7, "%-16Fs", name);

		// first teams manage their injured reserves
		tPtr->Manage (MANAGE_ROSTER, MANAGE_POSTGAME);

		// let teams grab supplemental players until they're happy
		DYN_16	listIndex = 0,
					numGrabs = 8;
		while (listIndex != -1)
		{
			if (--numGrabs == 0)
				numGrabs--;

			listIndex = tPtr->DraftSelect (pList, Roster::DSTYPE_WEEKLY);
			mono.printf (40, 8, "%-2d", listIndex);

			if (listIndex != -1)
			{
				Roster		*rPtr = LoadPlayers (tPtr);

				// remove chosen player from local list (it's always the top player)
				RosPlayer	*takenPPtr = (RosPlayer *) pList [listIndex]->linkout (),
								*releasedPPtr = rPtr->GetReleasePlayer (TRUE);

				// player gets onto the team
				DYN_16	p1Id = takenPPtr->GetId (),
							p2Id = releasedPPtr->GetId ();

				TradePlayers (0,
									&p1Id,
									tPtr->GetId (),
									&p2Id,
									TRADE_ACCEPTED);
			}
		}

		if (--teamList)
			break;
	}

	// don't need free agents stored by position any more
	DeletePlayerList ();

	// prepare for TRADES
	// (but only during the first half of the season)
	mono.printf (40, 6, "%-16s", "trading");
	if (curWeek < (midPoint + 1))
	{
		// remove last week's trades
		// (they have to be removed 1 week later than they are allowed)
		if (!data->tradeNews)
		{
			data->tradeNews = new TradeNews (path, fileName);
			vmAssert (data->tradeNews);
		}
		data->tradeNews->Flush ();

		if (curWeek < midPoint)
		{
			// each team can only make a certain number of trades per week,
			// either as propser or proposee
			DYN_16	tSize = teamList.size ();
			data->numTrades = new DYN_8 [tSize];

			if (data->numTrades)
			{
				memset (data->numTrades, MAX_TRADES_PER_WEEK, tSize);

				GenCursor<Team>	tCursor (&teamList);
				tCursor.end ();

				while (TRUE)
				{
					Team		*tPtr = tCursor ();
					DYN_8		FAR *name = tPtr->GetName ();
					if (callBack)
						callBack (name);
					mono.printf (40, 7, "%-16Fs", name);

					if (data->numTrades [tPtr->GetId () - 1])
						tPtr->Manage (MANAGE_ROSTER, MANAGE_WEEKLY);

					if (--tCursor)
						break;
				}

				delete [] data->numTrades;
			}
		}

		data->tradeNews->Save (path, fileName);
		delete data->tradeNews;
		data->tradeNews = 0;
	}

	// save away league state
	data->saveEnable = TRUE;
	Save ();


	// cleanup
	// free agent pool (never know who was signed/released)
	if (data->fapPtr)
		data->fapPtr->Save (path, fileName);
	DeleteDraftList (DRAFT_FREE_AGENT);

	DeletePlayers ();

	#if	WEEKLY_STATS
	delete ssPtr;
	#endif

	mono.printf (40, 6, "%-16s", "");
	mono.printf (40, 7, "%-16s", "");
	mono.printf (40, 8, "%-16s", "");
	mono.printf (40, 9, "%-16s", "");
	#endif	// WEEKLY_MOVES
}

/*
	---------------------------------------------------------------------
	NAME:			ManageWeekly

	FUNCTION:	eash roster will fill up its posDrafts array with the
					positions it requres
					the league will try to find an appropriate trade with
					another team

	INPUT:		id = what team is PROPOSING trades
					pArray = array of players team is willing to part with
					posWanted-> array of positions team is trying to get
									(in the order it needs them)

	RETURNS:		whether trades occured
	---------------------------------------------------------------------
*/

DYN_16
League::ManageWeekly (DYN_16 id, RosPlayer *pArray [],
								Roster::SlotId *slotsWanted)
{
	GenCursor<Team>	tCursor (&teamList);
	DYN_16				update = FALSE,
							nt = data->numTrades [id - 1];
	RosPlayer			*givePtrs [TradeInfo::MAX_PIDS];
	Roster::SlotId		sIds [TradeInfo::MAX_PIDS];
	memset (givePtrs, 0, sizeof (givePtrs));
	memset (sIds, 0, sizeof (sIds));

	// try for every position the team needs
	for (DYN_8 i = 0; (i < NUM_TRADE_POS) && nt; i++, slotsWanted++)
	{
		sIds [0] = *slotsWanted;

		// keep trying trades until attempts are exhausted,
		// or there ar no more players left
		for (DYN_8 i = 0; (i < NUM_TRADE_PLAYERS) && nt; i++)
		{
			RosPlayer	*pPtr = pArray [i];

			// if another player available
			if (pPtr)
			{
				// store in array of outgoing players (although there's only 1)
				givePtrs [0] = pPtr;

				// find a team to trade to
				tCursor.start ();
				while (nt)
				{
					Team			*ttPtr = tCursor ();
					DYN_16		tId = ttPtr->GetId ();
					mono.printf (40, 8, "%-16Fs", ttPtr->GetName ());

					// don't try to trade with yourself,
					// or a team that's already completed a trade
					if ((tId != id) && (data->numTrades [tId - 1]))
					{
						// get a player proposal from some other team
						RosPlayer	*getPtrs [TradeInfo::MAX_PIDS];
						DYN_16		wanna = ttPtr->WannaTrade (getPtrs, sIds, givePtrs);

						// if team has an offer, try to get it through
						if (wanna)
						{
							DYN_16		tId1,
											pId1s [TradeInfo::MAX_PIDS],
											tId2,
											pId2s [TradeInfo::MAX_PIDS];
							RosPlayer	*ptPtr = getPtrs [0];
							memset (pId1s, 0, sizeof (pId1s));
							memset (pId2s, 0, sizeof (pId2s));

							// if the other team is computer managed, then let
							// it evaluate the trade
							// (note that the proposer/proposee is swapped so that
							// the team looking for trades can evaluate one that
							// is proposed to it)
							if (ttPtr->GetManage () == OWNER_COMPUTER)
							{
								tId1 = tId;
								pId1s [0] = ptPtr->GetId ();
								tId2 = id;
								pId2s [0] = pPtr->GetId ();
							}

							// if not, a human user will have to
							else
							{
								tId1 = id;
								pId1s [0] = pPtr->GetId ();
								tId2 = tId;
								pId2s [0] = ptPtr->GetId ();
							}

							mono.printf (40, 9, "%-2d %-4d %-2d %-4d",
												tId1, pId1s [0], tId2, pId2s [0]);

							// GJW: 8-27-1996
							TradeStatus status = TradePlayers (tId1, pId1s,
																			tId2, pId2s,
																			TRADE_PROPOSED);
							if (status != TRADE_REJECTED)
							{
								// team can complete only so many trades per week
								if (status == TRADE_ACCEPTED)
								{
									nt--;
									data->numTrades [tId1 - 1]--;
									data->numTrades [tId2 - 1]--;
								}

								// if not rejected, this player is no longer on the trading
								// block
								pArray [i] = 0;
								break;
							}
						}
					}

					if (++tCursor)
						break;
				}
			}
		}
	}

	return update;
}

/*
	---------------------------------------------------------------------
	NAME:			HealPlayers

	FUNCTION:	prior to going to a game the teams' players heal 1 weeks
					worth of injuries
					GJW: 9-21-1993, stage = season running with the VERY FIRST
					set of games being 'sent out', as opposed to them being
					'read in' (since games can be paused)

	INPUT:		hTeam, vTeam-> 2 teams about to play a game

	RETURNS:		teams' players healed
	---------------------------------------------------------------------
*/

void
League::HealPlayers (Team *hTeam, Team *vTeam)
{
	if (data->stage < STAGE_SEASON_RUNNING)
		Sequence (STAGE_SEASON_RUNNING);

	// if this is the first game to be played this week:
	//		flush last week's hilights
	//		make sure any teams with 'byes' also have their players healed
	DYN_16	weekNum = data->schedule->GetWData (SCDWEEKDATA_CUR_WEEK, 0);
	if (!data->byesHealed && !data->schedule->GamesDoneThisWeek ())
	{
		// make sure this only gets called once per set of simulated games
		data->byesHealed = TRUE;
		FlushHilights ();
		FlushNews ();

		// but there aren't any byes in the playoffs,
		// just teams that didn't make it
		// GJW: 10-12-1994, it IS possible to have a bye in the playoffs,
		// if a team is a divisional winner, that doesn't have to deal
		// with a wild card game
		// if (weekNum < data->schedule->GetWData (SCDWEEKDATA_NUM_REGWEEKS, 0))
		// {
		if (!teamList.start ())
		{
			while (TRUE)
			{
				Team		*tPtr = teamList ();

				// if no game this week, then heal the players
				if (data->schedule->FindTeamsGame ((DYN_8)(tPtr->GetId ()), (DYN_8)weekNum) == -1)
				{
					tPtr->LoadPlayers (data->fpPlayers);
					tPtr->HealPlayers ();
				}

				if (++teamList)
					break;
			}
		}
	}

	// 2 weeks off before the 'super bowl',
	// GJW: 4-30-1993, when the schedule is only 17 weeks long
	DYN_16	extraRest = FALSE;
	if ((data->schedule->GetWData (SCDWEEKDATA_NUM_REGWEEKS, 0) == 17) &&
			(weekNum == (data->schedule->GetWData (SCDWEEKDATA_NUM_WEEKS, 0) - 1)))
		extraRest = TRUE;

	if (hTeam)
	{
		hTeam->LoadPlayers (data->fpPlayers);
		hTeam->HealPlayers ();
		if (extraRest)
			hTeam->HealPlayers ();
	}

	if (vTeam)
	{
		vTeam->LoadPlayers (data->fpPlayers);
		vTeam->HealPlayers ();
		if (extraRest)
			vTeam->HealPlayers ();
	}
}
#endif	// LCDT

/*
	---------------------------------------------------------------------
	NAME:			Injuries

	FUNCTION:	after each game put a 'hurt on' those players that
					have injury points

	INPUT:		numInj = number of injured players
					injPtr-> player ids & points
					hPtr, vPtr -> 2 teams involved in this game

	RETURNS:		player's injury points updated
	---------------------------------------------------------------------
*/

//#pragma  argsused
void
League::Injuries (DYN_16 numInj, Injury *injPtr,
						DYN_16 weekNum, Team *hPtr, Team *vPtr)
{
	#if	(LCDT == 4)
	TeamNews		tn (0, 0, 0);

	while (numInj--)
	{
		DYN_16	pId = injPtr->pId,
					pts = injPtr->points;

		// get a pointer to the player
		Team			*tPtr = hPtr;
		RosPlayer	*pPtr = tPtr->GetPlayer (pId);
		if (!pPtr)
		{
			tPtr = vPtr;
			pPtr = tPtr->GetPlayer (pId);
		}

		if (pPtr)
		{
			// GJW: 7-27-1993, no CEIs for single-season leagues,
			// just a big owie
			if ((pts == RosPlayer::INJURY_OVER) && (data->tol == TYPE_SS))
				pts = RosPlayer::INJURY_OVER / 2;

			pPtr->Injury (weekNum, pts);

			// a career-ending injury?
			// generate a replacement player in the free agent pool
			if (pts == RosPlayer::INJURY_OVER)
			{
				DYN_16	tId = tPtr->GetId ();

				tn.AddNews (tId, pId);

				if (!data->fapPtr)
					LoadDraftList (DRAFT_FREE_AGENT, TRUE);

				// save player's condition (won't be on the team anymore when
				// SavePlayers () is called)
				pPtr->Save (*data->fpPlayers);

				// then let team find a replacement
				DYN_16	listIndex = tPtr->DraftSelect (pList, Roster::DSTYPE_CEI);
				Roster	*rPtr = tPtr->LoadPlayers (0);

				// remove chosen player from local list (it's always the top player)
				if (listIndex != -1)
				{
					RosPlayer	*takenPPtr = (RosPlayer *) pList [listIndex]->linkout (),
									*releasedPPtr = rPtr->GetReleasePlayer (TRUE);

					DYN_16	p1Id = takenPPtr->GetId (),
								p2Id = releasedPPtr->GetId ();
					// player gets onto the team
					TradePlayers (0,
										&p1Id,
										tId,
										&p2Id,
										TRADE_ACCEPTED);
				}
			}
		}

		injPtr++;
	}

	// save any CEIs that may have occured
	tn.Save (path, fileName);

	// if CEI created a free agent pool, save and delete
	if (data->fapPtr)
	{
		data->fapPtr->Save (path, fileName);
		DeleteDraftList (DRAFT_FREE_AGENT);
		DeletePlayerList ();
	}
	#endif
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			KillFiles

	FUNCTION:	delete the files associated with a league
					if user is overwriting (i.e. creating a new one with the
					same name as an existing league), make sure the deleted
					league's data is not used by the league under construction

	INPUT:		pName, fName-> path & filename of league to delete
					(if 0, then use what's stored within this league)
					newFlag indicates if this league is under construction

	RETURNS:		TRUE/FALSE if league was deleted
	---------------------------------------------------------------------
*/

DYN_16
League::KillFiles (DYN_8 *pName, DYN_8 *fName)
{
	if (!pName && !fName)
	{
		pName = path;
		fName = fileName;
	}

	DYN_16	goFlag = TRUE;

	// if this is a new league, make sure no data that will be needed,
	// gets destroyed
	if (data->newFlag)
	{
		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();
			if (tPtr->NewLeagueCheck (pName, fName))
			{
				goFlag = FALSE;
				break;
			}

			if (++teamList)
				break;
		}
	}

	if (goFlag)
	{
		DYN_8				buffer [MAXPATH];

		// kill files associated with league structure
		makeFileName (buffer, pName, fName, ".*");
		DYN_16	done = (DYN_16)DOSFILE::findfirst (buffer);
		while (!done)
		{
			// only kill files KNOWN to be associated with leagues
			DYN_8		ok = FALSE,
						*extPtr = strchr ((DYN_8 *) DOSFILE::getFindName (), '.');
			for (DYN_8 i = 0; i < NUM_KILL_FILES; i++)
			{
				if (!stricmp (extPtr, deleteExts [i]))
				{
					ok = TRUE;
					break;
				}
			}

			if (ok)
			{
				makeFileName (buffer, pName, (DYN_8 *) DOSFILE::getFindName (), extPtr);
				unlink (buffer);
			}

			done = (DYN_16)DOSFILE::findnext ();
		}

		// then any game plan files
		DYN_8		*ptr = makeFileName (0, pName, PBText, "");
		makeFileName (buffer, ptr, fName, ".*");
		done = (DYN_16)DOSFILE::findfirst (buffer);
		while (!done)
		{
			DYN_8 *getFName = (DYN_8 *) DOSFILE::getFindName ();
			makeFileName (buffer, ptr, getFName, strchr (getFName, '.'));
			unlink (buffer);
			done = (DYN_16)DOSFILE::findnext ();
		}

		// then any game tapes
		FlushHilights ();

		// and yesterday's news
		FlushNews ();
	}

	return goFlag;
}

/*
	---------------------------------------------------------------------
	NAME:			GetChamps

	FUNCTION:	provide caller with a LeagueChamps object

	INPUT:		none

	RETURNS:		->LeagueChamps object created
	---------------------------------------------------------------------
*/

LeagueChamps *
League::GetChamps ()
{
	if (!data->champs)
		data->champs = new LeagueChamps (path, fileName, data->inception);

	return data->champs;
}

void
League::DeleteChamps ()
{
	if (data->champs)
	{
		delete data->champs;
		data->champs = 0;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			MakeProBowl

	FUNCTION:	pick the best of the best,
					and write out a new league

	INPUT:		team's players, and their statistics

	RETURNS:		newly created pro bowl team
	---------------------------------------------------------------------
*/

League *
League::MakeProBowl (const DYN_8 *path, const DYN_8 *fName)
{
	setMemAssertOk (FALSE);

	League	*lPtr = new League ();
	lPtr->data->config = CONFIG_2;
	lPtr->data->tol = TYPE_PROBOWL;
	lPtr->data->newFlag = FALSE;
	lPtr->ConfDivSetup ();
	strcpy (lPtr->data->name, proBowlName);

	lPtr->SetProBowlStrings (this);

	RasFile	pbFile;
	pbFile.Load ((DYN_8 *) path, (DYN_8 *) fName);
	Equation equ ("");

   // got 2 leages, this is the nfl league, lPtr is the new probowl league
	if (lPtr->SetProBowlPlayers (this, pbFile, equ))
		lPtr->Save ();

	else
	{
		delete lPtr;
		lPtr = 0;
	}

	setMemAssertOk (TRUE);

	return lPtr;
}

/*
	---------------------------------------------------------------------
	NAME:			GetHilights

	FUNCTION:	return whether any league game highlight tapes are
					out there

	INPUT:		directory of where tapes should be

	RETURNS:		TRUE/FALSE if tapes are found
	---------------------------------------------------------------------
*/

DYN_16
League::GetHilights ()
{
	DYN_8				*ptr = makeFileName (0, ltapeText, fileName, ".*");
	return !DOSFILE::findfirst (ptr);
}
#endif	// LCDT

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			MonoPrint

	FUNCTION:	print debugging league data to monchrome screen

	INPUT:		this-> League object

	RETURNS:		data sent to mono screen
	---------------------------------------------------------------------
*/

void
League::MonoPrint ()
{
	DYN_16	y = MONO_START_LINE;

	mono.printf (0, y++, "name:		 %-24s", name);
	mono.printf (0, y++, "path:		 %-32s", path);
	mono.printf (0, y++, "fileName:	 %-8s", fileName);
	mono.printf (0, y++, "config:		 %d", data->config);
	mono.printf (0, y++, "tol/nfl:	 %d/%d", data->tol, data->nflFlag);
	mono.printf (0, y++, "stage:		 %-3d", data->stage);
	mono.printf (0, y++, "fileType:	 %d", data->fileType);
	mono.printf (0, y++, "inception:	 %-5d", data->inception);
	mono.printf (0, y++, "numSeasons: %-3d", data->numSeasons);

	if (data->schedule)
		mono.printf (0, y++, "week:		 %-3d", data->schedule->GetWData (SCDWEEKDATA_CUR_WEEK, 0));
}
#endif

/*
	=====================================================================
									PRIVATE METHODS
	=====================================================================
*/

/*
	---------------------------------------------------------------------
	NAME:			Open/CloseLog

	FUNCTION:	open or close the file that logs trades

	INPUT:		logFP-> FILE openned or closed

	RETURNS:		file opened or closed
	---------------------------------------------------------------------
*/

FILE * NEAR
League::OpenLog ()
{
	if (!logFP)
		logFP = my_fopen ("tradelog.out", "at");

	return logFP;
}

void NEAR
League::CloseLog ()
{
	if (logFP)
	{
		my_fclose (logFP);
		logFP = 0;
	}
}



// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Open

	FUNCTION:	open a Read/WriteBFile

	INPUT:		rlf = ReadBFile, or wlf = WriteBFile reference
					name-> string of file to open

	RETURNS:		file opened,
					or assert message given
	---------------------------------------------------------------------
*/

void NEAR
League::Open (ReadBFile &rlf, const DYN_8 *name)
{
	sysAssert (rlf.Open (name), EXITMSG_LEAGUE_LOAD);
	sysAssert (rlf.Find (L02Text, 1) != -1L, EXITMSG_LEAGUE_BFIND);
}

void NEAR
League::Open (WriteBFile &wlf, const DYN_8 *name)
{
	sysAssert (wlf.Open (name) > 0, EXITMSG_LEAGUE_LOAD);
}

/*
	---------------------------------------------------------------------
	NAME:			Load

	FUNCTION:	read in a League file

					LeagueConfig			sizeof (DYN_8)
					numConfs					sizeof (DYN_8)
					tol						sizeof (DYN_8)
					stage						sizeof (DYN_8)
					fileType					sizeof (DYN_8)
					uniqueId					sizeof (DYN_16)
					nextPlayerId			sizeof (DYN_16)
					inception				sizeof (DYN_16)
					numSeasons				sizeof (DYN_8)
					conf block numbers	(numConfs * sizeof (DYN_8))

					name						strlen (name)
					trophy					strlen (trophy)
					commPN					strlen (commPN)

	INPUT:		this -> League object
					fullLoad = just league data, or all the conferences,
					divisions & teams too

	RETURNS:		new one loaded
	---------------------------------------------------------------------
b
*/

void NEAR
League::Load (DYN_8 *fName, FullLoad fullLoad)
{
	DYN_8		fullName [MAXPATH];

	getcwd (fullName, MAXPATH);
	Load (fullName, fName, fullLoad);
}

void NEAR
League::Load (DYN_8 *pName, DYN_8 *fName, FullLoad fullLoad)
{
	DYN_8		cIds [16],
				*ptr,
				nameBuff [MAXFILE + MAXEXT],
				fullName [MAXPATH],
				planfilename [MAXPATH];

	// if memory can't be granted, don't assert out
	setMemAssertOk (FALSE);

	memset (&pList, 0, sizeof (pList) +
								sizeof (name) +
								sizeof (trophy) +
								sizeof (path) +
								sizeof (fileName));
	data->saveEnable = TRUE;

	strcpy (path, pName);
	strcpy (data->path, pName);
	strcpy (nameBuff, fName);
	if (ptr = strchr (nameBuff, '.'))
		*ptr = 0;
	strcpy (fileName, nameBuff);
	strcpy (data->fileName, fileName);

	makeFileName (fullName, pName, fName, lgeText);
	makeFileName (planfilename, pName, fName, lg2Text );

	LeagueReadBFile	rlf;
	Open (rlf, fullName);

	LeagueDiskDataF ldd;
	rlf.Read (&ldd, sizeof (LeagueDiskDataF));
	 // -- copy the hard way, due to packing differences in enums
	LDDcpy( data, &ldd );
//	  memcpy (&data->config, &ldd.config, sizeof (LeagueDiskData));
	data->newFlag = FALSE;

	rlf.Read (cIds, data->numConfs);

	if (fullLoad)
	{
	#ifdef _DEBUG
		DWORD dwTime = GetTickCount();
	#endif

		switch (fullLoad)
		{
			case 4: //LOAD_LCDT:
				/* load conferences, divisions, and teams; no schedule or players */
				LoadConfs( planfilename, rlf, cIds );
				rlf.Close();
				BuildTeamList (&teamList);
				break;

			case LOAD_ALL:
				LoadConfs (planfilename,rlf, cIds);
				data->schedule = new Schedule (rlf, this);
				vmAssert (data->schedule);
				rlf.Close ();
				BuildTeamList (&teamList);

				#if	(LCDT != 3)
				LoadPlayers ((Team *) 0);
				#endif
				break;

			case LOAD_ROSTERS:
			case LOAD_TEAMS:
			{
				// since the number of teams is not know, keep loading until
				// some id does not produce a team

				DYN_16	id = 1,
							done = FALSE;

				while (!done)
				{
					Team	*tPtr;

					// include rosters for team?
//					  if (fullLoad == LOAD_ROSTERS)
						tPtr = new Team (planfilename, rlf, id, FALSE);

//					  else
//						  tPtr = new Team (rlf, id);

					vmAssert (tPtr);
					if (tPtr->GetId ())
					{
						teamList.linkin (tPtr);
						id++;
					}

					else
					{
						delete tPtr;
						done = TRUE;
					}
				}
				rlf.Close ();
				break;
			}
				
			default:
				break;
		}

	#ifdef _DEBUG
		mono.printf( 0, 2+fullLoad, "League::Load(%d) : %d ms elapsed", (int)fullLoad, GetTickCount()-dwTime );
	#endif
	}

	else
	{
		rlf.Close ();
		data->schedule = 0;
	}

	data->cdpPtr = 0;
	data->fapPtr = 0;

	if (data->stage == STAGE_NOBYEINFO)
		data->byes = BYES_NOTPICKED;

	setMemAssertOk (TRUE);
}

/*
	---------------------------------------------------------------------
	NAME:			LoadConfs

	FUNCTION:	read in each conf's data of the division

	INPUT:		rlf-> file to get info from
					cIds -> array of conference ids to load

	RETURNS:		conf blocks read in
	---------------------------------------------------------------------
*/

void NEAR
League::LoadConfs (char *planfilename, ReadBFile &rlf, DYN_8 *cIds)
{
	DYN_8 numConfs = data->numConfs;
	for (DYN_16 i = 0; i < numConfs; i++)
	{
		Conference	*cPtr = new Conference (planfilename, rlf, *cIds++, FALSE);
		vmAssert (cPtr);
		confList.linkin (cPtr);
	}
}

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Save

	FUNCTION:	write a "LGE:" block to the file
					"LGE:"
					LeagueConfig			sizeof (DYN_8)
					numConfs					sizeof (DYN_8)
					tol						sizeof (DYN_8)
					stage						sizeof (DYN_8)
					fileType					sizeof (DYN_8)
					uniqueId					sizeof (DYN_16)
					nextPlayerId			sizeof (DYN_16)
					inception				sizeof (DYN_16)
					numSeasons				sizeof (DYN_8)
					conf block numbers	(numConfs * sizeof (DYN_8))

					name						strlen (name)
					trophy					strlen (trophy)
					commPN					strlen (commPN)

	INPUT:		this -> League
					saveEnable = should league be written to disk
					(turned off during massive changes of data so that incremental
					changes aren't saved, like start of season)

	RETURNS:		file updated
	---------------------------------------------------------------------
*/

void NEAR
League::Save (char * planFile, WriteBFile &wlf)
{
	// first the league block
	wlf.OpenBlock	(L02Text);
	LeagueDiskDataF ldd;
	LDDcpy2( &ldd, data );
//	  memcpy (&ldd.config, &data->config, sizeof (LeagueDiskData));
	wlf.Write (&ldd, sizeof (LeagueDiskDataF));

	// then the conference ids
	DYN_8		numConfs = data->numConfs,
				cIds [16],
				*ptr = cIds;

	confList.start ();
	for (DYN_8 i = numConfs; i > 0; i--)
	{
		Conference	*cPtr = confList ();
		*ptr++ = (DYN_8)cPtr->GetId ();
		++confList;
	}
	wlf.Write (cIds, numConfs);
	wlf.CloseBlock ();

	// then the conferences themselves
	confList.start ();
	for (i = numConfs; i > 0; i--)
	{
		Conference	*cPtr = confList ();
		cPtr->Save (planFile, wlf);
		++confList;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SetConfPtr

	FUNCTION:	go through linked list and set pointer to conference

	INPUT:		index = number of links from start

	RETURNS:		conf -> conference
	---------------------------------------------------------------------
*/

void NEAR
League::SetConfPtr (DYN_8 index)
{
	confList.start ();
	confList += index;
	data->conf = confList ();
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			Start

	FUNCTION:	determine if it is ok to advance to STAGE_START
					(must have completed the previous year)
					if so, the following occurs:
					(those steps also used by the Single-Season leagues are
					indicated with a *)

					if this is the first attempt to start the new season:
					the league has played one more season
					all players are aged and retired

					computer-managed teams remove injured players,
					make trade proposals & sign free agents
					college draft pool is generated
					*seasons's schedule generated

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::Start ()
{
	// advance only if in the right previous stage
	if (data->stage >= STAGE_PLAYOFFS_END)
	{
		// if first time, get rid of the fogies
		if (data->stage == STAGE_PLAYOFFS_END)
		{
			data->numSeasons++;

			if (data->tol == TYPE_CONT)
				AgeAndRetire ();
		}

		// make sure all teams have dumped their injured reserves
		DYN_16	springCleaned = TRUE;
		if (data->stage != STAGE_NOBYEINFO)
		{
			springCleaned = TeamSpringCleaning ();
			data->byes = BYES_NOTPICKED;
		}

		// once IR has been taken care of, generate a draft order & schedule
		if (springCleaned)
		{
			// if this is a 28 team league, must choose whether to bye
			if ((data->config < CONFIG_28) ||
					(data->config == CONFIG_30) ||
					(data->byes != BYES_NOTPICKED))
			{
				if (data->tol == TYPE_CONT)
				{
					// make a new draft
					GenerateRecords (FALSE);
					SortTeamList (&teamList, SORT_RECORD);
					Draft draft;
					draft.Generate ((GenList<DYN_16> *) &teamList, data->schedule);
					draft.Save (path, fileName);
				}

				data->stage = STAGE_START;

				// GJW: 11-8-1995, for the 30 team league, it's NOT ok
				// for byes to be -1
				if (data->config == CONFIG_30)
					data->byes = BYES_NONE;
				data->schedule->GenerateNew (data->byes, data->config);
				GenerateRecords (TRUE);

				// update database
				GetStats (TRUE);

				// determine drafting strategies
				if (data->tol == TYPE_CONT)
					GenerateStrategies ();

				// clear old games & hilights
				data->statFile->Flush (STATFLUSH_WEEK);
				FlushHilights ();

				// clear old season (continuing leagues clear them out after
				// training camp)
				if (data->tol == TYPE_SS)
				{
					data->statFile->Flush (STATFLUSH_SEASON);
					data->statFile->Flush (STATFLUSH_CAREER);
				}

				CloseStats ();
			}

			else
				data->stage = STAGE_NOBYEINFO;
		}

		// if not, wait in this stage
		else
			data->stage = STAGE_TRANSITION;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			CDraftStart

	FUNCTION:	determine if it is ok to advance to STAGE_CDRAFT_START
					must have just started a league, or paused the draft

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::CDraftStart ()
{
	if ((data->stage == STAGE_START) || (data->stage == STAGE_CDRAFT_PAUSE))
		data->stage = STAGE_CDRAFT_START;
}

/*
	---------------------------------------------------------------------
	NAME:			CDraftPause

	FUNCTION:	determine if it is ok to advance to STAGE_CDRAFT_PAUSE
					must be in the middle of the draft

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::CDraftPause ()
{
	if (data->stage == STAGE_CDRAFT_START)
		data->stage = STAGE_CDRAFT_PAUSE;
}

/*
	---------------------------------------------------------------------
	NAME:			CDraftEnd

	FUNCTION:	determine if it is ok to advance to STAGE_CDRAFT_END
					must be in the midst of draft

					all college players that weren't taken, are moved to the
					free agent list

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::CDraftEnd ()
{
	if (data->stage == STAGE_CDRAFT_START)
	{
		data->stage = STAGE_CDRAFT_END;
		CollegeToFreeAgent ();
		data->cdpPtr->Save (path, fileName);
		data->fapPtr->Save (path, fileName);
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SDraftStart

	FUNCTION:	determine if it is ok to advance to STAGE_SDRAFT_START
					must have completed college draft, or paused this draft

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SDraftStart ()
{
	if (data->stage < STAGE_SDRAFT_END)
		data->stage = STAGE_SDRAFT_START;
}

/*
	---------------------------------------------------------------------
	NAME:			SDraftPause

	FUNCTION:	determine if it is ok to advance to STAGE_SDRAFT_PAUSE

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SDraftPause ()
{
	if (data->stage == STAGE_SDRAFT_START)
		data->stage = STAGE_SDRAFT_PAUSE;
}

/*
	---------------------------------------------------------------------
	NAME:			SDraftEnd

	FUNCTION:	determine if it is ok to advance to STAGE_SDRAFT_END

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SDraftEnd ()
{
	if (data->stage == STAGE_SDRAFT_START)
		data->stage = STAGE_SDRAFT_END;
}

/*
	---------------------------------------------------------------------
	NAME:			TrainingStart

	FUNCTION:	determine if it is ok to advance to STAGE_TRAINING_START
					if all the drafting is done
					if all the teams have submitted their training allocations

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::TrainingStart ()
{
	if (data->stage >= STAGE_SDRAFT_END)
	{
		DYN_16	tcFlag = TRUE;

		// check each team in the league
		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();

			LoadPlayers (tPtr);
			tPtr->Manage (MANAGE_TRAININGCAMP, 0);

			if (!tPtr->TrainAlloc ())
				tcFlag = FALSE;

			if (++teamList)
				break;
		}

		if (tcFlag)
			data->stage = STAGE_TRAINING_START;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			TrainingEnd

	FUNCTION:	determine if it is ok to advance to STAGE_TRAINING_END
					execute training camp on all players on all teams
					(note that free agents aren't modified!)

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::TrainingEnd ()
{
	if (data->stage == STAGE_TRAINING_START)
	{
		data->stage = STAGE_TRAINING_END;

		// clear old season
		GetStats (TRUE);
		data->statFile->Flush (STATFLUSH_SEASON);
		CloseStats ();

		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();

			tPtr->Train ();

			if (++teamList)
				break;
		}

		SavePlayers ();
		DeletePlayers ();
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SeasonStart

	FUNCTION:	determine if it is ok to advance to STAGE_SEASON_START
					if so, then the schedule week goes to 0 (from -1)

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SeasonStart ()
{
	data->stage = STAGE_SEASON_START;
	data->schedule->DetermineWeek ();
}

/*
	---------------------------------------------------------------------
	NAME:			SeasonRunning

	FUNCTION:	determine if it is ok to advance to STAGE_SEASON_RUNNING
					(i.e. the first game has been played)
					GJW: 9-21-1993, since games can be paused,
					set when first game has been sent to arcade

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SeasonRunning ()
{
	data->stage = STAGE_SEASON_RUNNING;
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			SeasonEnd

	FUNCTION:	determine if it is ok to advance to STAGE_SEASON_END

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::SeasonEnd ()
{
	data->stage = STAGE_SEASON_END;
}

/*
	---------------------------------------------------------------------
	NAME:			PlayoffStart

	FUNCTION:	determine if it is ok to advance to STAGE_PLAYOFFS_START

	INPUT:		current stage

	RETURNS:		new stage
	---------------------------------------------------------------------
*/

void NEAR
League::PlayoffStart ()
{
	data->stage = STAGE_PLAYOFFS_START;
	data->schedule->SetPlayoffGames (data->config);
}

/*
	---------------------------------------------------------------------
	NAME:			PlayoffEnd

	FUNCTION:	when last playoff game is finished:
					the stage is set to PLAYOFFS_END

	INPUT:		current stage

	RETURNS:		new stage (PLAYOFFS_END)
	---------------------------------------------------------------------
*/

void NEAR
League::PlayoffEnd ()
{
	data->stage = STAGE_PLAYOFFS_END;

	// write who the city that won it all
	// store champion in championship file
	// get pointer to the game
	data->schedule->SetSchedPtrs (data->schedule->GetWData (SCDWEEKDATA_NUM_WEEKS, 0) - 1, 0);
	Team		*hTeam = GetTeam ((DYN_8)data->schedule->GetSTData (STDATA_ID, SGTDATA_HTEAM)),
				*vTeam = GetTeam ((DYN_8)data->schedule->GetSTData (STDATA_ID, SGTDATA_VTEAM)),
				*wTeam,
				*lTeam;
	DYN_8		hScore = (DYN_8)data->schedule->GetSTData (STDATA_PF, SGTDATA_HTEAM),
				vScore = (DYN_8)data->schedule->GetSTData (STDATA_PF, SGTDATA_VTEAM),
				wScore,
				lScore;

	if (hScore > vScore)
	{
		wTeam = hTeam;
		lTeam = vTeam;
		wScore = hScore;
		lScore = vScore;
	}
	else
	{
		wTeam = vTeam;
		lTeam = hTeam;
		wScore = vScore;
		lScore = hScore;
	}

	LeagueChamps::WriteChamp (path, fileName,
										wTeam->GetName (), wScore,
										lTeam->GetName (), lScore);

	LoadPlayers ();
	if (!teamList.start ())
	{
		while (TRUE)
		{
			Team		*tPtr = teamList ();

			// force single-season players back to perfect health
			tPtr->SeasonReset (data->tol == TYPE_SS);

			if (++teamList)
				break;
		}
	}

	SavePlayers ();
	DeletePlayers ();
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			AgeAndRetire

	FUNCTION:	at the end of every season, everybody gets a little
					bit older

					for speed:
					first age, then save out players (they have all changed)
					and this file takes a while to update
					then retire, then save team's rosters and free agent list
					(much smaller files to save)

	INPUT:		all players on all teams or the free agent list

	RETURNS:		team's rosters depleted a little
	---------------------------------------------------------------------
*/

void NEAR
League::AgeAndRetire ()
{
	// Age all free agents
	LoadDraftList (DRAFT_FREE_AGENT, FALSE);
	data->fapPtr->Age ();

	// and team members
	teamList.start ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		Roster	*rPtr = LoadPlayers (tPtr);

		rPtr->Age ();
		if (++teamList)
			break;
	}

	// now save all aged player data
	SavePlayers ();

	// Retire all free agents and team members
	data->fapPtr->Retire ();
	data->fapPtr->Save (path, fileName);
	DeleteDraftList (DRAFT_FREE_AGENT);

	// delete team news (retirement annoucements will be saved)
	TeamNews		tn (0, 0, 0);
	tn.Flush (path, fileName);

	teamList.start ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		Roster	*rPtr = LoadPlayers (tPtr);

		rPtr->Retire (tn);
		if (++teamList)
			break;
	}

	// save any Retirements that may have occured
	tn.Save (path, fileName);

	DeletePlayers ();
	Save ();
}

/*
	---------------------------------------------------------------------
	NAME:			TeamSpringCleaning

	FUNCTION:	after the previous season is complete, and before the
					new one starts, each team must do some cleaning up:

					any players on IR must be re-assigned to a useful slot,
					or released into free-agnecy

					if all teams have removed their IR players,
					college draft pool is generated (based on number of
					players in each position)

	INPUT:		all the teams

	RETURNS:		TRUE/FALSE if all teams have re-assigned IR players
	---------------------------------------------------------------------
*/

DYN_16 NEAR
League::TeamSpringCleaning ()
{
	DYN_16	irFlag = TRUE,		// assume all teams will pass
				saveFlag = FALSE; // and none will alter their rosters

	FreeAgentPool	*fapPtr = data->fapPtr;
	if (!fapPtr)
		LoadDraftList (DRAFT_SUP, FALSE);

	// let computer managed teams move players off of IR
	data->saveEnable = FALSE;
	teamList.start ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		Roster	*rPtr = LoadPlayers (tPtr);

		saveFlag |= tPtr->Manage (MANAGE_ROSTER, MANAGE_PRESEASON);

		// find ANY team that still has players on IR
		if (irFlag && rPtr->GetNumIRPlayers ())
			irFlag = FALSE;

		if (++teamList)
			break;
	}

	data->saveEnable = TRUE;
	if (saveFlag)
		Save ();

	// save any free agents that may have been released
	if (data->fapPtr)
		data->fapPtr->Save (path, fileName);

	// then if all teams have no IR players, create college draft
	if (irFlag)
	{
		// don't generate college players if single-season
		if (data->tol == TYPE_CONT)
		{
			// all free agents are flushed
			data->fapPtr->Flush (path, fileName);

			DYN_16	posCounts [Roster::MAX_DRAFT_LIST];
			DeterminePlayerCounts (posCounts);
			GenerateCollegePlayers (posCounts);
			SavePlayers ();

			data->fpPlayers->Close ();
			LoadPlayers ((Team *) 0);
		}
	}

	// if agents were opened within this function,
	// delete them
	if (!fapPtr)
		DeleteDraftList (DRAFT_SUP);

	DeletePlayers ();
	return irFlag;
}

/*
	---------------------------------------------------------------------
	NAME:			DeterminePlayerCounts

	FUNCTION:	count the number of players at each position on every
					team (this will determine how many college draft choices
					need to be generated)

	INPUT:		posCounts-> array to store count for each position

	RETURNS:		posCounts [] filled in with number of qbs, rbs etc.
	---------------------------------------------------------------------
*/

void NEAR
League::DeterminePlayerCounts (DYN_16 *posCounts)
{
	memset (posCounts, 0, Roster::MAX_DRAFT_LIST * sizeof (DYN_16));

	teamList.start ();
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		Roster	*rPtr = LoadPlayers (tPtr);

		// count all players on the team, that match the given position,
		// that aren't empty or injured
		RosPlayer::Pos *ppPtr = genPos;
		DYN_16			i = 0;
		for (; i < Roster::MAX_DRAFT_LIST; i++, ppPtr++)
			posCounts [i] += rPtr->BuildPlayerList (0, *ppPtr,
																	FALSE,	// no empty assigned
																	FALSE,	// no empty open
																	FALSE,	// no inactive open
																	FALSE,	// no injured reserve
																	FALSE);	// don't care
		tPtr->DeletePlayers ();

		if (++teamList)
			break;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			GenerateCollegePlayers

	FUNCTION:	create the college draft roster & file
					(fills in the league talent pool)

	INPUT:		posCounts-> array of counts for each position

	RETURNS:		cdpPtr-> created college draft pool
	---------------------------------------------------------------------
*/

void NEAR
League::GenerateCollegePlayers (DYN_16 *posCounts)
{
	NamePlayer	ANP = NamePlayer ();
	data->cdpPtr = new CollegeDraftPool ();
	vmAssert (data->cdpPtr);

	// first, choose a 'blessed' position
	RosPlayer::Pos blessed = genPos [getPosRand () % Roster::MAX_DRAFT_LIST];
	mono.printf (40 + 16, MONO_START_LINE, "blessed: %3d", blessed);

	// then fill in all the gaps
	DYN_16	*totalCounts = talentCounts [data->config];

	RosPlayer::Pos *ppPtr = genPos;
	DYN_16			i = 0;
	for (; i < Roster::MAX_DRAFT_LIST; i++, ppPtr++)
		data->nextPlayerId = data->cdpPtr->MakePool (&ANP, *ppPtr,
															totalCounts [i] - posCounts [i],
															data->nextPlayerId, blessed);

	data->cdpPtr->Save (path, fileName);
	data->cdpPtr->SavePlayers (*data->fpPlayers);
	DeleteDraftList (DRAFT_COLLEGE);
}

/*
	---------------------------------------------------------------------
	NAME:			GenerateFAPlayers

	FUNCTION:	when a new league is created, generate a list of draftables
					(fills in the league talent pool)

	INPUT:		posCounts-> array of counts for each position

	RETURNS:		fapPtr-> created free agent draft pool
	---------------------------------------------------------------------
*/

void NEAR
League::GenerateFAPlayers (DYN_16 *posCounts)
{
	NamePlayer	ANP = NamePlayer ();
	data->cdpPtr = new CollegeDraftPool ();
	data->fapPtr = new FreeAgentPool ();
	vmAssert (data->cdpPtr && data->fapPtr);

	// there is no blessed position for free agents

	// then fill in all the gaps
	RosPlayer::Pos *ppPtr = genPos;
	DYN_16			*totalCounts = talentCounts [data->config],
						i = 0;
	for (; i < Roster::MAX_DRAFT_LIST; i++, ppPtr++)
		data->nextPlayerId = data->cdpPtr->MakePool (&ANP, *ppPtr,
															totalCounts [i] - posCounts [i],
															data->nextPlayerId, RosPlayer::POS_NONE);
	CollegeToFreeAgent ();
	data->fapPtr->Save (path, fileName);
}

/*
	---------------------------------------------------------------------
	NAME:			CollegeToFreeAgent

	FUNCTION:	transfer the list of college players to the free agent list
					(college draft is done, or a new league has been created)

	INPUT:		cdpPtr, fapPtr -> lists of players

	RETURNS:		cdpPtr-> no players left
					fapPtr-> cdp's players xferred
	---------------------------------------------------------------------
*/

void NEAR
League::CollegeToFreeAgent ()
{
	GenList<RosPlayer>	pList;
	data->cdpPtr->XferPlayerList (&pList);

	if (!pList.start ())
	{
		while (TRUE)
		{
			RosPlayer	*rpPtr = pList ();
			rpPtr->ModRatings (75, 75);
			data->fapPtr->AddPlayer (rpPtr);
			if (++pList)
				break;
		}
	}
}

/*
	---------------------------------------------------------------------
	NAME:			GenerateStrategies

	FUNCTION:	after completion of past season, each team determines what
					positions it should be drafting for this season

	INPUT:		teamList

	RETURNS:		each team's DraftStrategies determined
	---------------------------------------------------------------------
*/

void NEAR
League::GenerateStrategies ()
{
	GenList<STRAT_STAT>	sList;
	STRAT_STAT				*ssPtr = (STRAT_STAT *)
									new DYN_8 [sizeof (STRAT_STAT) * teamList.size ()];

	if (ssPtr)
	{
		setStatRankings (&teamList, data->statFile, &sList, ssPtr);

		// then have each team modify their current strategies
		// sList has been sorted in the order of the last setStatRanks () call,
		// so get the team ids from that list instead
		sList.start ();
		while (TRUE)
		{
			STRAT_STAT	*ssPtr = sList ();
			Team			*tPtr = GetTeam (ssPtr->tId);

			// have each team evaluate it's positions in the various categories
			tPtr->GenerateStrategies (ssPtr->ranks);

			if (++sList)
				break;
		}

		delete ssPtr;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			FillAssigned

	FUNCTION:	let the given team pick free agents until their assigned
					holes are filled

	INPUT:		tPtr-> team doing the filling
					pList-> free agent choice lists
					fapPtr-> free agent pool of players

	RETURNS:		team's roster full
	---------------------------------------------------------------------
*/

void NEAR
League::FillAssigned (Team *tPtr)
{
	Roster	*rPtr = LoadPlayers (tPtr);

	// let team keep picking until roster is full
	while (!tPtr->AssignedFull ())
	{
		DYN_16	listIndex = tPtr->DraftSelect (pList, Roster::DSTYPE_GAMEREADY);
		vmAssert (listIndex != -1);

		// remove chosen player from local list (it's always the top player)
		RosPlayer	*takenPPtr = (RosPlayer *) pList [listIndex]->linkout (),
						*releasedPPtr = rPtr->GetReleasePlayer (TRUE);

		// player gets onto the team
		DYN_16	p1Id = takenPPtr->GetId (),
					p2Id = releasedPPtr->GetId ();

		TradePlayers (0,
							&p1Id,
							tPtr->GetId (),
							&p2Id,
							TRADE_ACCEPTED);
	}

	tPtr->DeletePlayers ();
}

/*
	---------------------------------------------------------------------
	NAME:			ConfDivPlayersSetup

	FUNCTION:	when setting up a new league, the league is initialized
					so that all the teams have their players come from
					the default league (dynamix.lge)

	INPUT:		-> new League object

	RETURNS:		all team's player data taken from dynamix
	---------------------------------------------------------------------
*/

void NEAR
League::ConfDivPlayersSetup ()
{
	DYN_8		pName [MAXPATH];
	getcwd (pName, MAXPATH);

	ReadBFile	rlf;
	//	  GJW: 10-28-1995, use NFLPI95 as default league, when building a 30-teamer
	if( IsCurrentSeasonCompatible() )
		sysAssert (rlf.Open (nflLeagueFile) > 0, EXITMSG_LEAGUE_CREATE);
	else
		sysAssert (rlf.Open (defLeagueFile) > 0, EXITMSG_LEAGUE_CREATE);

	DYN_16	numTeams;
	switch (data->config)
	{
		case CONFIG_8:
			numTeams = 8;
			break;

		case CONFIG_10:
			numTeams = 10;
			break;

		case CONFIG_12:
			numTeams = 12;
			break;

		case CONFIG_18:
			numTeams = 18;
			break;

		case CONFIG_28:
			numTeams = 28;
			break;

		case CONFIG_30:
			numTeams = 30;
			break;

		case CONFIG_2:
			numTeams = 2;
			break;
	}

	for (DYN_16 i = 1; i <= numTeams; i++)
	{
		Team	*tPtr = GetTeam ((DYN_8)i);
		// GJW: 10-28-1995
		if( IsCurrentSeasonCompatible() )
			tPtr->Steal( rlf, pName, nflLeagueFile );
		else
			tPtr->Steal( rlf, pName, defLeagueFile );
	}
}

/*
	---------------------------------------------------------------------
	NAME:			NewLeagueFiles

	FUNCTION:	when a new league has been created:
					rosters for the new teams need to be created (with players
					receiving new ids)
					playbooks associated with the new teams have to be
					modified and renamed so that they belong to this league

	INPUT:		-> League object

	RETURNS:		all rosters & playbooks for each of the teams read in,
					modified, and written out
					rosters players id's modified
	---------------------------------------------------------------------
*/

void NEAR
League::NewLeagueFiles ()
{
	// copy all the players into a destination file
	PlayerRdWr	prw;
	sysAssert (prw.Create (makeFileName (0, path, fileName, pyrText)), EXITMSG_ROSPLAYER_SAVE);

	// for each team in the new league,
	// let each one make adjustments (roster & playbooks) as necessary
	DYN_16	nflValid = TRUE;

	teamList.start ();
	data->nextPlayerId = PlayerRdWr::BASE_PLAYER_ID;
	while (TRUE)
	{
		Team		*tPtr = teamList ();
		data->nextPlayerId = tPtr->NewLeagueInfo (prw,
																data->nextPlayerId,
																path, fileName,
																data->tol == TYPE_PROBOWL);
		nflValid &= tPtr->GetNFLId () != 0;

		if (++teamList)
			break;
	}

	data->nflFlag = (DYN_8)nflValid;

	// if this league is not, clear all individual teams that may have
	// their id set
	if (!nflValid)
	{
		teamList.start ();
		while (TRUE)
		{
			Team		*tPtr = teamList ();

			tPtr->ClearNFLId ();

			if (++teamList)
				break;
		}
	}

	prw.Close ();
}

/*
	---------------------------------------------------------------------
	NAME:			FlushHilights

	FUNCTION:	remove the last set of highlights

	INPUT:		none

	RETURNS:		all hilight tapes deleted
	---------------------------------------------------------------------
*/

void NEAR
League::FlushHilights ()
{
	DYN_8				buffer [MAXPATH];
	makeFileName (buffer, ltapeText, fileName, ".*");

	DYN_16	done = (DYN_16)DOSFILE::findfirst (buffer);
	makeFileName (buffer, ltapeText, (DYN_8 *) 0, (DYN_8 *) 0);
	while (!done)
	{
		DYN_8 *fName = (DYN_8 *) DOSFILE::getFindName (),
				*ptr = makeFileName (0, buffer, fName, strchr (fName, '.'));
		unlink (ptr);
		done = (DYN_16)DOSFILE::findnext ();
	}
}

/*
	---------------------------------------------------------------------
	NAME:			FlushNews

	FUNCTION:	delete the team news file

	INPUT:		none

	RETURNS:		team news file deleted
	---------------------------------------------------------------------
*/

void NEAR
League::FlushNews ()
{
	TeamNews tn (0, 0, 0);
	tn.Flush (path, fileName);
}

/*
	---------------------------------------------------------------------
	NAME:			SetProBowlStrings

	FUNCTION:	name the file and teams that are going to play
					filename is the next one of the form: ASGxxxx.lge
					(ASG = All Star Game)
					if the league has 2 conferences, those are the team
					names
					otherwise pick the next best thing

	INPUT:		lPtr -> to SOURCE league

	RETURNS:		new league's destination file & team names set
	---------------------------------------------------------------------
*/

void NEAR
League::SetProBowlStrings (const League *lPtr)
{
	// find the next available league name
	DYN_8				nameBuff [MAXFILE + MAXEXT];
	DYN_16			fileNum = 0;
	do
	{
		sprintf (nameBuff, "ASG%04d.LGE", fileNum++);
	} while (!DOSFILE::findfirst (nameBuff));

	getcwd (path, MAXPATH);

	// delete extension
	nameBuff [7] = 0;
	strcpy (fileName, nameBuff);

	// choose names for the teams
	// try to use conference names
	DYN_8		*t1Name,
				*t2Name;

	GenCursor<Conference>	cCursor ((GenList<Conference> *) &lPtr->confList);
	cCursor.start ();
	Conference	*cPtr = cCursor ();
	if (lPtr->data->numConfs > 1)
	{
		t1Name = cPtr->GetName ();

		++cCursor;
		cPtr = cCursor ();
		t2Name = cPtr->GetName ();
	}

	// use divisions
	else
	{
		Division *dPtr = cPtr->GetDiv (0);
		t1Name = dPtr->GetName ();

		dPtr = cPtr->GetDiv (1);
		t2Name = dPtr->GetName ();
	}

	teamList.start ();
	Team	*tPtr = teamList ();
	tPtr->SetName (t1Name);

	++teamList;
	tPtr = teamList ();
	tPtr->SetName (t2Name);
}

/*
	---------------------------------------------------------------------
	NAME:			SetProBowlPlayers

	FUNCTION:	choose which players are the best at their positions
					for each conference (or division)
					since memory is at a premium, I need to do one conference,
					and then the other, since in a 28 team league, there isn't
					enough available

					GJW: 9-17-1993, if not enough memory for player nodes,
					then delete statFile object (and c-tree), and let 'er rip
					again

					GJW: 10-12-1994, new ratings-editor method

	INPUT:		lPtr -> to SOURCE league
					pbFile, equ = reference to player ratings editor objects

	RETURNS:		new league's team's players chosen
	---------------------------------------------------------------------
*/

// switcharoo, this is the new probowl league, lPtr is the source league.
// c++ is so unambigous (if you can understand it)
DYN_16 NEAR
League::SetProBowlPlayers (League *lPtr, RasFile &pbFile, Equation &equ)
{
   TeamId tIndex ;

   DYN_16               ok = TRUE ;

   StatFile *pStatFile = lPtr->GetStats (FALSE);

   /* Check to see if the All Pro Teams can be used.  Otherwise use the old methed
      JWM 12/24/96
   */
//   GenList<Statistic> statList ;
   DYN_16 APTExists = pStatFile->BuildStatList(0, PSSTAT_APTEAM, 0) ;

   if (APTExists && (GetNumSub () == 2)) {  
char errstr[256] ;
      PlayerRdWr		PRW;
      RosPlayer		*playerPtr;
      // new vars for APT
      GenList<RosPlayer> *pAPTList ;
      pAPTList = new GenList<RosPlayer> (APT_COUNT, (DYN_16) 0);
      DYN_16 playerId, statId ;
      StatisticPOG *pStat ;

      int guysPerPos[RosPlayer::POS_MAX_SPEC] = {QB_COUNT,FB_COUNT,HB_COUNT,TE_COUNT,WR_COUNT,
                                                  C_COUNT, G_COUNT, T_COUNT,DE_COUNT,DT_COUNT,
                                                 LB_COUNT,CB_COUNT, S_COUNT, K_COUNT, P_COUNT} ;


	   // -- load the retired players
	   char szFileName[255];


	   strcpy(szFileName, lPtr->GetPath());
	   strcat(szFileName, "\\");
	   strcat(szFileName, lPtr->GetFName());
	   strcat(szFileName, ".pyr");

	   vmAssert(PRW.Open(szFileName)) ;

      // pick players for 2 teams in the new league
      data->nextPlayerId = PlayerRdWr::BASE_PLAYER_ID;
      for (tIndex = TEAM_HOME; (tIndex < TEAM_MAX) && ok; tIndex = TeamId(tIndex + 1))
      {
         if (ok)
         {
//               DYN_16 conf = ((pStat->Statistic::data.id >> 4) & 0xf) ;
//               if (conf == tIndex) {
            for (int posIndex = RosPlayer::POS_QB ; posIndex < RosPlayer::POS_MAX_SPEC ; posIndex++) {
               for (int count = 0 ; count < guysPerPos[posIndex] ; count++) {
                  statId = posIndex+1 | tIndex<<4 | count<<8 ;

                  pStat = (StatisticPOG*) pStatFile->GetStat(PSSTAT_APTEAM, statId) ;
                  playerId = pStat->data.playerId ;
                  playerPtr = new RosPlayer(PRW, playerId) ;
sprintf(errstr,
        "id = %4d %s %s %2d %2d",
        playerPtr->GetId (),
        playerPtr->GetFirstName(),
        playerPtr->GetLastName(),
        playerPtr->GetPosGen (),
        playerPtr->GetPosSpec ()
       ) ;

                  pAPTList->linkin(playerPtr) ;

               }
            }
            Team  *tPtr = GetTeam (tIndex + 1);
            data->nextPlayerId = tPtr->PickPlayers (pAPTList, data->nextPlayerId);

            // delete the pick list
            pAPTList->start() ;
            while (pAPTList->size() != 0) { // for each player in APT list
               // delete the players
               delete pAPTList->linkout() ;
            }
            ok = (data->nextPlayerId != 0);
         }

      }
      PRW.Close() ;
   }
   else {  // no all pro team use the old method
      lPtr->SortTeamList (&lPtr->teamList, SORT_ID);
      DYN_16               pCounts [RosPlayer::POS_MAX_SPEC];
      GenList<RosPlayer>  *pickList [RosPlayer::POS_MAX_SPEC];
      // pick players for 2 teams in the new league
      data->nextPlayerId = PlayerRdWr::BASE_PLAYER_ID;
      for (tIndex = TEAM_HOME; (tIndex < TEAM_MAX) && ok; tIndex = TeamId(tIndex + 1))
      {
         // count the number of players that play every specific position,
         // so that the lists can pre-allocate their number of nodes
         memset (pCounts, 0, sizeof (pCounts));
         lPtr->CountProBowlPlayers (pCounts, tIndex);

         // pre-allocate the number of nodes for each list
         memset (pickList, 0, sizeof (pickList));
         for (DYN_8 i = 0; (i < RosPlayer::POS_MAX_SPEC) && ok; i++)
         {
            pickList [i] = new GenList<RosPlayer> (pCounts [i], (DYN_16) 0);
            ok &= (pickList [i] != 0) && (pickList [i]->BlockAlloc () != 0);

            #if   DEBUG_PROBOWL
            debugPrintf ("List alloc [%d] %d %p\n", i, pCounts [i], pickList [i]);
            #endif
         }

         if (ok)
         {
            lPtr->GetProBowlPLists (pickList, tIndex);
            lPtr->SortProBowlPlayers (pickList, pbFile, equ);

            Team  *tPtr = GetTeam (tIndex + 1);
            data->nextPlayerId = tPtr->PickPlayers (pickList, data->nextPlayerId);
         }

         // don't need player lists anymore
         for (i = 0; i < RosPlayer::POS_MAX_SPEC; i++)
            delete pickList [i];
      }
   }
      


   


   lPtr->CloseStats ();

   // don't need the SOURCE league players any more
   lPtr->DeletePlayers ();

   // copy all the players into a destination file
   if (ok)
   {
      data->fpPlayers = new PlayerRdWr  ();
      sysAssert (data->fpPlayers->Create (makeFileName (0, path, fileName, pyrText)), EXITMSG_ROSPLAYER_SAVE);

      // save this league's players
      SavePlayers ();
      data->fpPlayers->Close ();
      delete data->fpPlayers;
      data->fpPlayers = 0;

      // organize the rosters for depth
      Team  *tPtr = GetTeam (1);
      tPtr->Manage (MANAGE_ROSTER, MANAGE_OWNERSHIP);
      tPtr = GetTeam (2);
      tPtr->Manage (MANAGE_ROSTER, MANAGE_OWNERSHIP);
   }

   DeletePlayers ();

   return ok;
}

/*
	---------------------------------------------------------------------
	NAME:			CountProBowlPlayers

	FUNCTION:	determine how many players of each SPECIFIC position
					there are in the source league for the pro bowl

	INPUT:		pCounts = array of counts to set
					cIndex = first/second set of teams
								(the teams are loaded in conference/division order,
								so take the first or second set)

	RETURNS:		total number of QBs, FBs, HBs, etc.
	---------------------------------------------------------------------
*/

void NEAR
League::CountProBowlPlayers (DYN_16 *pCounts, const DYN_16 cIndex)
{
	DYN_16				teamsToGet = teamList.size () / 2;
	GenCursor<Team>	tCursor (&teamList);

	// grab 1st or 2nd half
	if (cIndex)
		tCursor += teamsToGet;

	while (teamsToGet--)
	{
		Team		*tPtr = tCursor ();
		Roster	*rPtr = LoadPlayers (tPtr);

		rPtr->CountProBowlPlayers (pCounts);

		++tCursor;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			GetProBowlPLists

	FUNCTION:	for each team in this league,
					let teams fill the player list array

	INPUT:		pList = array of player lists for each team to fill
					cIndex = first/second set of teams
								(the teams are loaded in conference/division order,
								so take the first or second set)

	RETURNS:		all players on teams, with out injury, are put into
					the appropriate lists (indexed based on roster slots)
	---------------------------------------------------------------------
*/

void NEAR
League::GetProBowlPLists (GenList<RosPlayer> *pList [], const DYN_16 cIndex)
{
	DYN_16				teamsToGet = teamList.size () / 2;
	GenCursor<Team>	tCursor (&teamList);

	// grab 1st or 2nd half
	if (cIndex)
		tCursor += teamsToGet;

	while (teamsToGet--)
	{
		Team		*tPtr = tCursor ();
		Roster	*rPtr = LoadPlayers (tPtr);

		rPtr->BuildPlayerList (pList);

		++tCursor;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SortProBowlPlayers

	FUNCTION:	for each player in each list, get a pro bowl value
					once all the players have values generated,
					sort based on the highest values

	INPUT:		pList = array of player lists
					pbFile, equ = reference to player ratings editor objects

	RETURNS:		pList lists sorted based on player abilities
	---------------------------------------------------------------------
*/

void NEAR
League::SortProBowlPlayers (GenList<RosPlayer> *pickList [],
										RasFile &pbFile, Equation &equ)
{
	for (DYN_8 i = 0; i < RosPlayer::POS_MAX_SPEC; i++)
	{
		// pick the equation
		equ.SetStringTo (pbFile.GetEqString (i));

		// pick the correct list of players
		GenList<RosPlayer>	*pList = pickList [i];

		#if	DEBUG_PROBOWL
		debugPrintf ("List: %d, %p\n", i, pList);
		#endif

		if (pList)
		{
			// generate a pro bowl value for every player
			pList->start ();
			while (TRUE)
			{
				RosPlayer	*pPtr = (*pList) ();
				double		pbValue;

				mono.printf (50, 21, "%16s", pPtr->GetLastName ());

				// GetStatus () returns the team id
				equ.Calculate (pPtr, (DYN_U_8)pPtr->GetStatus (), &pbValue, TRUE);
				pPtr->SetProBowlId (pbValue);

				#if	DEBUG_PROBOWL
				debugPrintf ("	  player: %s, from: %d = %Ld\n",
									pPtr->GetLastName (),
									pPtr->GetStatus (),
									pPtr->GetProBowlId ());
				#endif

				if (++(*pList))
					break;
			}

			// now sort the players based on their generated points
			ListShellSort ((zDList *) pickList [i], i, proBowlCompare);
		}
	}
}
#endif	// LCDT








/*
	=====================================================================
									LOCAL UTILITIES
	=====================================================================
*/

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			compareFunc

	FUNCTION:	the function called by the generic shell sort to determine
					if the 2 list elements should be swapped

	INPUT:		iPtr, plusgPtr-> 2 teams
					sortType = which sort is happening

	RETURNS:		TRUE/FALSE if 2 teams should be swapped
					i.e. *iPtr < *plusgPtr
	---------------------------------------------------------------------
*/

DYN_16
compareFunc (void *iPtr, void *plusgPtr, DYN_16 sortType)
{
	Team		*itPtr = (Team *) iPtr,
				*pgtPtr = (Team *) plusgPtr;
	DYN_16	itId = itPtr->GetId (),
				pgtId = pgtPtr->GetId (),
				doTest = TRUE,
				doSwap = FALSE;
	DYN_8		*compPtr = compTable;

	// first, check if these 2 teams have been compared before
	if (compPtr)
	{
		compPtr = getCompTablePtr (compPtr, (DYN_8)itId, (DYN_8)pgtId);

		DYN_8 value = *compPtr;
		// test results (+/- 1)
		if (value)
		{
			doTest = FALSE;

			// if the first team has the higher id
			if (itId > pgtId)
			{
				// and is worse
				if (value > 0)
					doSwap = TRUE;
			}

			// the second team has the higher id
			else
			{
				// and is better
				if (value < 0)
					doSwap = TRUE;
			}
		}
	}

	if (doTest)
	{
		// perform specific swap test
		switch (sortType)
		{
			case League::SORT_ALPHA:
				doSwap = alphaCompare (itPtr, pgtPtr);
				break;

			case League::SORT_RECORD:
				if (poStage >= STAGE_PLAYOFFS_START)
					doSwap = playoffCompare (itPtr, pgtPtr);

				else
					doSwap = recordCompare (itPtr, pgtPtr);
				break;

			case League::SORT_ID:
				doSwap = idCompare (itPtr, pgtPtr);
				break;
		}

		// save results (+/- 1)
		if (compPtr)
		{
			// assume lower id is better
			DYN_8		value = 1;

			// if the first team has the higher id
			if (itId > pgtId)
			{
				// and is better
				if (!doSwap)
					value = -1;
			}

			// the second team has the higher id
			else
			{
				// and is worse
				if (doSwap)
					value = -1;
			}

			*compPtr = value;
		}
	}

	return doSwap;
}

/*
	---------------------------------------------------------------------
	NAME:			alphaCompare

	FUNCTION:	return which team comes alphabeticaly first

	INPUT:		t1, t2 -> teams

	RETURNS:		0 if t1 comes before t2
					!0 if t2 comes before t1
	---------------------------------------------------------------------
*/

static DYN_16 NEAR
alphaCompare (Team *tm1, Team *tm2)
{
	return strcmp (tm1->GetName (), tm2->GetName ()) > 0;
}

/*
	---------------------------------------------------------------------
	NAME:			recordCompare

	FUNCTION:	return which team comes first because of record
					1st winning percentage
					if tied, then more points for
					if tied, then least points against
					alphabetically

	INPUT:		t1, t2 -> teams

	RETURNS:		0 if t1 comes before t2
					!0 if t2 comes before t1
	---------------------------------------------------------------------
*/

static DYN_16 NEAR
recordCompare (Team *tm1, Team *tm2)
{
	DYN_16	r1 = tm1->WLTPct (),
				r2 = tm2->WLTPct ();

	// who has greater winning percentage?
	if (r1 != r2)
		return r2 > r1;

	// 1st tie-breaker (most points scored)
	r1 = tm1->GetPF ();
	r2 = tm2->GetPF ();
	if (r1 != r2)
		return r2 > r1;

	// 2nd tie-breaker (fewest points allowed)
	r1 = tm1->GetPA ();
	r2 = tm2->GetPA ();
	if (r1 != r2)
		return r1 > r2;

	// 3rd tie-breaker (alphabetical)
	return alphaCompare (tm1, tm2);
}

/*
	---------------------------------------------------------------------
	NAME:			playoffCompare

	FUNCTION:	return which team comes first because of record
					(in the following order):

					winning percentage
					head-to-head record

					same division							same conference
					won-lost-tied record vs div		w-l-t vs conf
					w-l-t vs conference					w-l-t vs common opponents
					w-l-t vs common opponents			net point conf games
					net points in div games				net points in all games
					net points in all games				alphabetically
					alphabetically

	INPUT:		t1, t2 -> teams

	RETURNS:		0 if t1 comes before t2
					!0 if t2 comes before t1
	---------------------------------------------------------------------
*/

static DYN_16 NEAR
playoffCompare (Team *tm1, Team *tm2)
{
	DYN_16	r1 = tm1->WLTPct (),
				r2 = tm2->WLTPct ();

	// who has greater winning percentage?
	if (r1 != r2)
		return r2 > r1;

	// 1st tie-breaker (head-to_head)
	DYN_16	t1Id = tm1->GetId (),
				t2Id = tm2->GetId ();
	r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_HEAD2HEAD);
	if (!r1)
	{

		// 2nd tie-breaker (dependent up divisional relationship)
		// make sure both teams are in the same division AND conference
		if ((tm1->GetCI () == tm2->GetCI ()) &&
			(tm1->GetDI () == tm2->GetDI ()))
		{
			// divisional tie-breakers
			r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_DIV);

			if (!r1)
			{
				r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_CONF);

				if (!r1)
				{
					r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_COMMON);

					if (!r1)
					{
						r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_DIV_POINTS);

						if (!r1)
							r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_ALL_POINTS);
					}
				}
			}
		}

		else
		{
			// conference tie-breakers
			r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_CONF);

			if (!r1)
			{
				r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_COMMON);

				if (!r1)
				{
					r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_CONF_POINTS);

					if (!r1)
						r1 = poSchedPtr->WinLossTie ((DYN_8)t1Id, (DYN_8)t2Id, SCDWLTDATA_ALL_POINTS);
				}
			}
		}
	}

	// if any of the tie-breakers solved anything, return so
	if (r1)
		return r1 == t2Id;

	// if not, do the scientific thing
	return alphaCompare (tm1, tm2);
}

/*
	---------------------------------------------------------------------
	NAME:			idCompare

	FUNCTION:	return which team comes first based on id
					LOWER numbers come first

	INPUT:		t1, t2 -> teams

	RETURNS:		0 if t1 comes before t2
					!0 if t2 comes before t1
	---------------------------------------------------------------------
*/

static DYN_16 NEAR
idCompare (Team *tm1, Team *tm2)
{
	return tm1->GetId () > tm2->GetId ();
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			proBowlCompare

	FUNCTION:	compare the 2 players based on their specific position's
					criteria
					the stats have been analyzed previously, and the player's
					overall score is kept in the ID area

	INPUT:		iPtr, plusgPtr-> 2 players
					sortType = SPECIFIC position of player (a don't care)

	RETURNS:		TRUE/FALSE if 2 players should be swapped
	---------------------------------------------------------------------
*/

//#pragma  argsused

static DYN_16
proBowlCompare (void FAR *iPtr, void FAR *plusgPtr, DYN_16 sortType)
{
	RosPlayer	*p1Ptr = (RosPlayer *) iPtr,
					*p2Ptr = (RosPlayer *) plusgPtr;

	DYN_16		swapEm;
	double		compare = p2Ptr->GetProBowlId () - p1Ptr->GetProBowlId ();

	if (compare)
		swapEm = compare > 0;
	else
		swapEm = p2Ptr->SumAbrs (0) > p1Ptr->SumAbrs (0);

	return swapEm;
}

/*
	---------------------------------------------------------------------
	NAME:			setStatRankings

	FUNCTION:	generate list of all used stats,
					then sort based on each team's performance

	INPUT:		tList-> list of teams
					sList-> where to put list of stat rankings
					ssPtr-> array of stat rankings to use (rather than new () each
						one)

	RETURNS:		sList set with teams in order
	---------------------------------------------------------------------
*/

static void NEAR
setStatRankings (GenList<Team> *tList, StatFile *statFile,
						GenList<STRAT_STAT> *sList, STRAT_STAT *ssPtr)
{
	// get a list of all relevant statistics
	tList->start ();
	while (TRUE)
	{
		Team		*tPtr = (*tList) ();
		DYN_16	tId = tPtr->GetId ();
		DYN_U_16 datBuff [REC_BUFF_SIZE];

		ssPtr->tId = (DYN_8)tId;
		statFile->GetStat (TSSTAT_RUSH, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_RUSH] = datBuff [STATOFFSET_YARDS];

		statFile->GetStat (TSSTAT_PASS, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_PASS] = datBuff [STATOFFSET_YARDS];

		statFile->GetStat (OTSSTAT_RUSH, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_ORUSH] = datBuff [STATOFFSET_YARDS];

		statFile->GetStat (OTSSTAT_PASS, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_OPASS] = datBuff [STATOFFSET_YARDS];

		statFile->GetStat (TSSTAT_SACK, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_SACK] = datBuff [STATOFFSET_NUMBER];

		statFile->GetStat (TSSTAT_FG, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_FG] = datBuff [STATOFFSET_YARDS];

		statFile->GetStat (TSSTAT_PUNT, tId, datBuff);
		ssPtr->ranks [Roster::STATLIST_PUNT] = datBuff [STATOFFSET_YARDS];

		if (sList->linkin (ssPtr))
			break;	// if destination list is full, stop
		ssPtr++;

		if (++(*tList))
			break;
	}

	// now convert raw info (total yards, etc.) into a league ranking
	// for each of the statistical categories
	for (DYN_16 index = Roster::STATLIST_PASS; index < Roster::STATLIST_MAX; index++)
		setStatRanks (sList, index);
}

/*
	---------------------------------------------------------------------
	NAME:			setStatRanks

	FUNCTION:	determine the rankings of all the teams based on some
					statistical category

	INPUT:		sList-> linked list of STRAT_STAT structures
					index = which rank [] to look at

	RETURNS:		rank [] converted from raw info to an index from
					0 to numTeams - 1
	---------------------------------------------------------------------
*/

static void NEAR
setStatRanks (GenList<STRAT_STAT> *sList, DYN_16 index)
{
	// first, sort list based on raw values
	ListShellSort ((zDList *) sList, index, statCompareFunc);

	// then convert raw values to rank numbers
	DYN_8		rank = 0;

	if (!sList->start ())
	{
		while (TRUE)
		{
			// get the next stat in list
			STRAT_STAT	*ssPtr = (*sList) ();

			// and set what index it is
			ssPtr->ranks [index] = rank++;

			if (++(*sList))
				break;
		}
	}
}

/*
	---------------------------------------------------------------------
	NAME:			statCompareFunc

	FUNCTION:	the function called by the generic shell sort to determine
					if the 2 list elements should be swapped

	INPUT:		iPtr, plusgPtr-> 2 STRAT_STATs
					index = which element of ranks [] to sort on

	RETURNS:		TRUE/FALSE if 2 teams should be swapped
	---------------------------------------------------------------------
*/

DYN_16
statCompareFunc (void *iPtr, void *plusgPtr, DYN_16 index)
{
	STRAT_STAT	*ssiPtr = (STRAT_STAT *) iPtr,
					*sspgPtr = (STRAT_STAT *) plusgPtr;
	DYN_U_16		iValue,
					pgValue;

	switch (index)
	{
		case Roster::STATLIST_PASS:
		case Roster::STATLIST_RUSH:
		case Roster::STATLIST_ORUSH:
		case Roster::STATLIST_OPASS:
		case Roster::STATLIST_SACK:
			iValue = ssiPtr->ranks [index];
			pgValue = sspgPtr->ranks [index];
			break;

		case Roster::STATLIST_FG:
			iValue = ssiPtr->ranks [index];
			pgValue = sspgPtr->ranks [index];
			break;

		case Roster::STATLIST_PUNT:
			iValue = ssiPtr->ranks [index];
			pgValue = sspgPtr->ranks [index];
			break;
	}

	return pgValue > iValue;
}
#endif




#if	DEBUG
/*
	---------------------------------------------------------------------
	NAME:			saveMonoUpdate

	FUNCTION:	write to the mono display how many percent of the players
					have been written

	INPUT:		pId = current player
					lastPlayerId = how many

	RETURNS:		%age written to mono display
	---------------------------------------------------------------------
*/

static void NEAR
saveMonoUpdate (DYN_U_16 pId, DYN_U_16 lastPlayerId)
{
	mono.printf (40, MONO_START_LINE, "%5d %5d", pId, lastPlayerId);
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			getCompTablePtr

	FUNCTION:	get a pointer to the comparision table based on the 2
					indices

					the table store the relationship between any 2 teams:
								team 1
								  1  2  3  4  5... 28
					team 2
								1 -  -  -  -  -	 -
								2 x  -  -  -  -	 -
								3 x  x  -  -  -	 -
							 ... x  x  x  x  x	 -
							  28 x  x  x  x  x	 -

					which forms a big triangle (there is no need to store
					3, 5 AND 5, 3 as 2 separate entries). Also, no team will
					be compared with itself, so all the indices can be scooted
					'left' 1 (0 based), and 'up' 2 (0 based, and no 1's row).

	INPUT:		base->table's start
					index1, index2 = team ids

	RETURNS:		->cell for these 2 teams
	---------------------------------------------------------------------
*/

static DYN_8 * NEAR
getCompTablePtr (DYN_8 *base, DYN_8 index1, DYN_8 index2)
{
	if (index1 > index2)
	{
		DYN_8	 temp = index1;
		index1 = index2;
		index2 = temp;
	}

	// convert to 0 based values
	index1--;
	index2 -= 2;
	DYN_8	 colHeight = MAX_TEAMS - 1;

	while (index1--)
	{
		base += colHeight;
		colHeight--;
		index2--;
	}

	return base + index2;
}

/*
	---------------------------------------------------------------------
	NAME:			defaultRules

	FUNCTION:	set the default league rules
					(for new leagues, or prior to 1.5 rules)

	INPUT:		grPtr->rules structure

	RETURNS:		default rules settings
	---------------------------------------------------------------------
*/

static void NEAR
defaultRules (League::GROUND_RULES FAR *grPtr)
{
	grPtr->qtrLength = QTR_15;
	grPtr->fatigue = TRUE;
	grPtr->fumbles = TRUE;
	grPtr->injuries = TRUE;
	grPtr->ints = TRUE;
	grPtr->penalties = TRUE;
	grPtr->clock45 = TRUE;
	grPtr->fastSim = FALSE;
}


static void LDDcpy( LeagData *pdata, LeagueDiskDataF *pldd )
{
	pdata->config		= (League::Config)pldd->config;
	pdata->numConfs	= pldd->numConfs;
	pdata->tol			= (League::Type) pldd->tol;
	pdata->stage		= pldd->stage;
	pdata->fileType	= (League::FileType) pldd->fileType;
	pdata->nflFlag		= pldd->nflFlag;
	pdata->uniqueId	= pldd->uniqueId;
	pdata->nextPlayerId	= pldd->nextPlayerId;
	pdata->inception	= pldd->inception;
	pdata->numSeasons = pldd->numSeasons;

	 // League::GROUND_RULES rules;
	pdata->rules.qtrLength	= (QtrLen) pldd->rules.qtrLength;
	pdata->rules.fatigue		= pldd->rules.fatigue;
	pdata->rules.fumbles		= pldd->rules.fumbles;
	pdata->rules.injuries	= pldd->rules.injuries;
	pdata->rules.ints			= pldd->rules.ints;
	pdata->rules.penalties	= pldd->rules.penalties;
	pdata->rules.clock45		= pldd->rules.clock45;
	pdata->rules.fastSim		= pldd->rules.fastSim;

	memcpy( pdata->name,	  pldd->name,	 League::NAME_LENGTH + 1 );
	memcpy( pdata->trophy, pldd->trophy, League::TROPHY_NAME_LENGTH + 1 );
	memcpy( pdata->commPN, pldd->commPN, Team::PN_NAME_LENGTH + 1 );

} // LDDCpy

static void LDDcpy2(LeagueDiskDataF *pldd, LeagData *pdata )
{
	pldd->config		= (DYN_8)pdata->config;
	pldd->numConfs		= pdata->numConfs;
	pldd->tol			= (DYN_8)pdata->tol;
	pldd->stage			= pdata->stage;
	pldd->fileType		= (DYN_8)pdata->fileType;
	pldd->nflFlag		= pdata->nflFlag;
	pldd->uniqueId		= pdata->uniqueId;
	pldd->nextPlayerId= pdata->nextPlayerId;
	pldd->inception	= pdata->inception;
	pldd->numSeasons	= pdata->numSeasons;

	 // League::GROUND_RULES rules;
	pldd->rules.qtrLength= (DYN_8) pdata->rules.qtrLength;
	pldd->rules.fatigue	= pdata->rules.fatigue;
	pldd->rules.fumbles	= pdata->rules.fumbles;
	pldd->rules.injuries = pdata->rules.injuries;
	pldd->rules.ints		= pdata->rules.ints;
	pldd->rules.penalties= pdata->rules.penalties;
	pldd->rules.clock45	= pdata->rules.clock45;
	pldd->rules.fastSim	= pdata->rules.fastSim;

	memcpy( pldd->name,	 pdata->name,	 League::NAME_LENGTH + 1 );
	memcpy( pldd->trophy, pdata->trophy, League::TROPHY_NAME_LENGTH + 1 );
	memcpy( pldd->commPN, pdata->commPN, Team::PN_NAME_LENGTH + 1 );

} // LDDCpy2



void League::ClosePlayerFile( void )
{
	if ( data->fpPlayers )
	{
		data->fpPlayers->Close ();
		delete data->fpPlayers;
		data->fpPlayers = 0;
	}
} // League::ClosePlayerFile

/*
	---------------------------------------------------------------------
	NAME:			IsCurrentSeasonCompatible

	FUNCTION:	Determine if a league is compatible with the league of
					of the current season (i.e. year of product release).
					Currently only checks if its a 28 or 30 team league.
					Used when creating a new league.

	INPUT:		league

	RETURNS:		TRUE if compatible
	---------------------------------------------------------------------
*/
// 12/10/96 mcqueen
int League::IsCurrentSeasonCompatible( void )
{
	if( data->config==CONFIG_30 )
		return TRUE;

	if( data->config==CONFIG_28 )
		return TRUE;
	
	return FALSE;
} /* IsCurrentSeasonCompatible */

// League.cpp

