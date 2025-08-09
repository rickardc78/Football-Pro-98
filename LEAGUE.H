/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


	league.h

	the League class

	newFlag		is this a league in the making?
	config		how many teams in this league
	tol			continuing/single-season indicator
	stage			where in the set of events (training camp, playoffs, etc.)
	fileType		original/copy/master
	uniqueId		unused currently
	inception	year league created
	numSeasons	how long it's been going
	numConfs		how many conferences (i.e. the next sub-system)
	confList		a list of conferences
	conf			-> currently chosen one
	schedule		-> schedule object
	calender		-> calender object

	name				string of this league's name
	trophy			what you win at the end
	commPN			commishioner's phone number
	path				path to the file this league read from
	fileName			and it's name

	league stages:
	START				all teams rosters are ready, college draft pool prepared,
						schedule generated
	CDRAFT_START	the college draft is under way
	CDRAFT_PAUSE
	CDRAFT_END
	SDRAFT_START	the supplemental draft is under way
	SDRAFT_PAUSE
	SDRAFT_END
	TRAINING_START training camp is under way
	TRAINING_END
	SEASON_START	all pre-season events have occured, waiting to start
	SEASON_RUNNING at least 1 game has been played
	SEASON_END		all regular season games are complete
	PLAYOFFS_START playoff games may have been played
	PLAYOFFS_END	all playoff games complete
	TRANSITION		prepare for new season, all guys have been aged & retired,
						but some teams still have guys on IR

	GJW: 1-17-1992, original creation
	GJW: 4-30-1993, 1 or 2 byes per season
	GJW: 5-5-1993, pro bowl type leagues
	GJW: 9-14-1994 LeagData implemenation
	GJW: 2-1-1995, 30 team leagues
	MMQ: 12-11-1996,	Added IsCurrentSeasonCompatible()
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef	_LEAGUE_H
#define	_LEAGUE_H

#include "Calender.h"
#include "CdPool.h"
#include "Confer.h"
#include "Draft.h"
#include "FaPool.h"
#include "LChamps.h"
#include "StatFile.h"
#include "TeamNews.h"

// size of arrays passed to ManageSeason
const NUM_TRADE_PLAYERS = 4;		// GJW: 4-21-1993, go a little quicker
const NUM_TRADE_POS = 2;

#define MAXPATH 260
#define MAXFILE 256
struct PlayerTradeInfo
{
	Team			*tPtr;
	RosPlayer	*pPtr;
};

struct Injury
{
	DYN_16	pId,
				points;
};

enum QtrLen
{
	QTR_5,
	QTR_10,
	QTR_15
};

typedef void	(*V_CCFP_FUNC) (const DYN_8 FAR *);

struct FAR	LeagData;
class			RasFile;
class			Equation;


#define STAGE_START					0
#define STAGE_CDRAFT_START			1
#define STAGE_CDRAFT_PAUSE			2
#define STAGE_CDRAFT_END			3
#define STAGE_SDRAFT_START			4
#define STAGE_SDRAFT_PAUSE			5
#define STAGE_SDRAFT_END			6
#define STAGE_TRAINING_START		7
#define STAGE_TRAINING_END			8
#define STAGE_SEASON_START			9
#define STAGE_SEASON_RUNNING	  10
#define STAGE_SEASON_END		  11
#define STAGE_PLAYOFFS_START	  12
#define STAGE_PLAYOFFS_END		  13
#define STAGE_TRANSITION		  14
#define STAGE_NOBYEINFO			  15

class League
{
public:
		enum
		{
			NAME_LENGTH = 24,
			TROPHY_NAME_LENGTH = 24,
		};

		enum FullLoad
		{
				LOAD_NONE,
				LOAD_ALL,
				LOAD_TEAMS,
				LOAD_ROSTERS,
				LOAD_LCDT,
		};

		enum Config
		{
				CONFIG_8,
				CONFIG_10,
				CONFIG_12,
				CONFIG_18,
				CONFIG_28,
				CONFIG_30,		// GJW: 1-31-1995, inserted a new config
				CONFIG_2,
				CONFIG_MAX
		};

		enum Type
		{
				TYPE_CONT,
				TYPE_SS,
				TYPE_PROBOWL,
				TYPE_MAX
		};

		enum Sort
		{
				SORT_ALPHA,
				SORT_RECORD,
				SORT_ID,
		};

//		  DYN_U_8 Stage;

		enum DraftType
		{
			DRAFT_COLLEGE,
			DRAFT_SUP,
			DRAFT_FREE_AGENT,
		};

		enum FileType
		{
				FILETYPE_MASTER,
				FILETYPE_ORIGINAL,
				FILETYPE_COPY,
		};

		enum TradePrev
		{
			TRADEPREV_OK,
			TRADEPREV_MYOPEN,
			TRADEPREV_OTHEROPEN,
			TRADEPREV_MYUNBALANCED,
			TRADEPREV_OTHERUNBALANCED,
			TRADEPREV_MAX,
		};

		enum Byes
		{
			BYES_NOTPICKED = -1,
			BYES_NONE,
			BYES_1,
			BYES_2,
		};

		typedef struct
		{
			QtrLen			  qtrLength;
			DYN_U_8			  fatigue	  : 1;
			DYN_U_8			  fumbles	  : 1;
			DYN_U_8			  injuries	  : 1;
			DYN_U_8			  ints		  : 1;
			DYN_U_8			  penalties	  : 1;
			DYN_U_8			  clock45	  : 1;
			DYN_U_8			  fastSim	  : 1;
		} GROUND_RULES;
		 // -- mirror structure for reading in the data in Win32
		typedef struct
		{
			DYN_8 /*QtrLen*/ qtrLength;
			DYN_U_8			  fatigue	  : 1;
			DYN_U_8			  fumbles	  : 1;
			DYN_U_8			  injuries	  : 1;
			DYN_U_8			  ints		  : 1;
			DYN_U_8			  penalties	  : 1;
			DYN_U_8			  clock45	  : 1;
			DYN_U_8			  fastSim	  : 1;
		} GROUND_RULESF;

private:
	// member functions

	// data members


protected:
	// member functions
	static FILE				* NEAR OpenLog ();
	static void				NEAR CloseLog ();

	void						NEAR Open (ReadBFile &rlf, const DYN_8 *name);
	void						NEAR Open (WriteBFile &wlf, const DYN_8 *name);
	void						NEAR Load (DYN_8 *fName, FullLoad fullLoad);
	void						NEAR Load (DYN_8 *pName, DYN_8 *fName, FullLoad fullLoad);
	void						NEAR LoadConfs (char *planfilename, ReadBFile &rlf, DYN_8 *cIds);
	void						NEAR Save (char *planfile, WriteBFile &wlf);
	void						NEAR SetConfPtr (DYN_8);

	void						NEAR Start ();
	void						NEAR CDraftStart ();
	void						NEAR CDraftPause ();
	void						NEAR CDraftEnd ();
	void						NEAR SDraftStart ();
	void						NEAR SDraftPause ();
	void						NEAR SDraftEnd ();
	void						NEAR TrainingStart ();
	void						NEAR TrainingEnd ();
	void						NEAR SeasonStart ();
	void						NEAR SeasonRunning ();
	void						NEAR SeasonEnd ();
	void						NEAR PlayoffStart ();
	void						NEAR PlayoffEnd ();

	void						NEAR AgeAndRetire ();
	DYN_16					NEAR TeamSpringCleaning ();
	void						NEAR DeterminePlayerCounts (DYN_16 *);
	void						NEAR GenerateCollegePlayers (DYN_16 *);
	void						NEAR GenerateFAPlayers (DYN_16 *);
	void						NEAR CollegeToFreeAgent ();
	void						NEAR GenerateStrategies ();

	void						NEAR FillAssigned (Team *tPtr);

	void						NEAR ConfDivPlayersSetup ();
	void						NEAR NewLeagueFiles ();

	void						NEAR FlushHilights ();
	void						NEAR FlushNews ();
	void						NEAR SetProBowlStrings (const League *lPtr);
	DYN_16					NEAR SetProBowlPlayers (League *lPtr,
																RasFile &pbFile,
																Equation &equ);
	void						NEAR CountProBowlPlayers (DYN_16 *pCounts, const DYN_16 cIndex);
	void						NEAR GetProBowlPLists (GenList<RosPlayer> *pList [],
																const DYN_16 cIndex);
	void						NEAR SortProBowlPlayers (GenList<RosPlayer> *pList [],
																	RasFile &pbFile,
																	Equation &equ);

	// data members
	static PoolAlloc		*pa;	// where to get memory from
	static FILE				*logFP;
	static Calender		calender;

	LeagData					*data;
	GenList<Conference>	confList;
	GenList<Team>			teamList;
	GenList<RosPlayer>	*pList [Roster::MAX_DRAFT_LIST];
	DYN_8						name [League::NAME_LENGTH + 1],
								trophy [League::TROPHY_NAME_LENGTH + 1],
								path [MAXPATH],
								fileName [MAXFILE];

public:
	// member functions
	static void				Init ();
	static void				DeInit ();
	static Team				*LoadTeam (DYN_8 *pName, DYN_8 *fName, DYN_16 tId);
	static void				SetTradeLog (const DYN_16 value);
	static DYN_16			PFNameValid (DYN_8 *path, DYN_8 *fName);
	static DYN_16			Loadable (const DYN_8 *fName);

	void						*operator new (size_t size);
	void						operator delete (void *t, size_t size);

								League ();
								League (DYN_8 *, FullLoad);
								League (DYN_8 *, DYN_8 *, FullLoad);
								~League ();

	DYN_16					BuildTeamList (GenList<Team> *);						// ALL teams in league
	DYN_16					BuildTeamList (GenList<Team> *, DYN_8);			// teams in conference
	DYN_16					BuildTeamList (GenList<Team> *, DYN_8, DYN_8);	// teams in division
	DYN_16					BuildIRTeamList (GenList<Team> *);					// teams w/IR guys
	DYN_16					BuildTCTeamList (GenList<Team> *);					// teams w/IR guys
	DYN_16					BuildTradeTeamList (GenList<Team> *tList,
															DYN_16 tId,
															RosPlayer **pPtrs,
															Roster::SlotId *sIds);		// teams interested
	void						SortTeamList (GenList<Team> *, Sort);				// how to sort list of teams

	Conference				*GetConf (DYN_8);
	Division					*GetDiv (DYN_8, DYN_8);
	Team						*GetTeam (DYN_8, DYN_8, DYN_8);
	Team						*GetTeam (DYN_8);
	Schedule					*GetSchedule ();
	Calender					*GetCalender ();
	StatFile					*GetStats (DYN_8);
	void						CloseStats ();

	DYN_8						GetNumSub ();	  // return numConfs

	Config					GetConfig () const;
	void						SetConfig (const Config value);
	Type						GetTOL () const;
	void						SetTOL (const Type value);
	DYN_U_8					  GetStage () const;
	void						SetStage (DYN_U_8 value);
	FileType					GetFileType () const;
	void						SetFileType (const FileType value);
	DYN_16					GetInception () const;
	DYN_16					GetNumSeasons () const;
	Byes						GetByes () const;
	void						SetByes (const Byes value);
	QtrLen					GetQtr () const;
	void						SetQtr (const QtrLen value);
	DYN_16					GetFatigue () const;
	void						SetFatigue (const DYN_16 value);
	DYN_16					GetFumbles () const;
	void						SetFumbles (const DYN_16 value);
	DYN_16					GetInjuries () const;
	void						SetInjuries (const DYN_16 value);
	DYN_16					GetInts () const;
	void						SetInts (const DYN_16 value);
	DYN_16					GetPenalties () const;
	void						SetPenalties (const DYN_16 value);
	DYN_16					Get45Sec () const;
	void						Set45Sec (const DYN_16 value);
	DYN_16					GetFastSim () const;
	void						SetFastSim (const DYN_16 value);
	DYN_16					GetNFLValid () const;
	void						SetNFLValid (const DYN_16 value);

	DYN_8						*GetName () const;
	void						SetName (const DYN_8 *string);
	DYN_8						*GetTrophy () const;
	void						SetTrophy (const DYN_8 *string);
	DYN_8						*GetCommPn () const;
	void						SetCommPn (const DYN_8 *string);
	DYN_8						*GetPath () const;
	void						SetPath (const DYN_8 *string);
	DYN_8						*GetFName () const;
	void						SetFName (const DYN_8 *string);

	void						Save ();
	void						Save (DYN_8 *fName);

	void						ComputerManage (const DYN_16 flag);
	void						GenerateRecords (DYN_16);
	DYN_16					Sequence (DYN_U_8);
	void						DetermineStage ();
	void						GameComplete (DYN_8 wIndex, DYN_8 gIndex,
													DYN_8 pfH, DYN_8 pfV,
													DYN_8 otFlag);
	void						GenerateNewSchedule (Byes byes);
	void						MakePool (DYN_16 generate, V_CCFP_FUNC callBack);

	DYN_16					Compare (DYN_8 *, DYN_8 *);

	void						DefLeagueFileSetup ();
	void						MakeCopy (DYN_8 *);
	void						ConfDivSetup ();
	void						SchedSetup ();
	void						StealTeam (DYN_16, DYN_16, DYN_16, League *, Team *);

	void						AssignDraftProfileNames ();

	void						LoadRoster (Team *tPtr);
	void						LoadPlayers ();
	Roster					*LoadPlayers (DYN_8 tId);
	Roster					*LoadPlayers (Team *tPtr);
	void						DeletePlayers ();
	void						DeletePlayers (DYN_8 tId);
	void						DeletePlayers (Team *tPtr);
	void						SavePlayers ();
	DYN_16					AssignedFull ();
	void						FillAssigned ();
	void						GenerateFAPoolPlayer (const RosPlayer::Pos pos, const DYN_16 listIndex);
	Roster					*LoadPlayersLW (Team *tPtr);
	void						DeletePlayersLW (Roster *rPtr);
	void						SavePlayersLW ();

	Draft						*LoadDraft ();
	void						SaveDraft (Draft *);
	void						*LoadDraftList (DraftType, DYN_16);
	void						DeleteDraftList (DraftType);
	GenList<RosPlayer>  *GetPlayerList (DYN_16);
	GenList<RosPlayer>  **GetPlayerLists ();
	void						DeletePlayerFromList (GenList<RosPlayer> *, RosPlayer *);
	void						DeletePlayerList ();
	TradePrev				TradePreview (DYN_16 t1Id, DYN_16 *p1Ids,
													DYN_16 t2Id, DYN_16 *p2Ids,
													TradePrev tp);
	TradeStatus				TradePlayers (DYN_16 t1Id, DYN_16 *p1Ids,
													DYN_16 t2Id, DYN_16 *p2Ids,
													TradeStatus status);
	RosPlayer				*FindPlayer (const DYN_16 id, const DYN_16 loadFlag);
	RosPlayer				*MakePlayer (const DYN_16 id);
	TradeNews				*LoadTrades ();
	DYN_16					CountTrades (TradeStatus);
	void						KillProposedTrades ();
	TeamNews					*GetTNews (DYN_16);
	void						FlushTrades () const;

	void						WeeklyRosterMoves (V_CCFP_FUNC callBack);
	DYN_16					ManageWeekly (DYN_16 id, RosPlayer *pArray [],
													Roster::SlotId *slotsWanted);
	void						HealPlayers (Team *hTeam, Team *vTeam);
	void						Injuries (DYN_16, Injury *, DYN_16, Team *, Team *);

	DYN_16					KillFiles (DYN_8 *path, DYN_8 *fName);

	LeagueChamps			*GetChamps ();
	void						DeleteChamps ();

	League					*MakeProBowl (const DYN_8 *path, const DYN_8 *fName);
	DYN_16					GetHilights ();

	void						MonoPrint ();

	void						ClosePlayerFile( void );

	int						IsCurrentSeasonCompatible();
	// data members
};

extern DYN_8 defLeagueFile [],
					dynText [];

#endif		// _LEAGUE_H
