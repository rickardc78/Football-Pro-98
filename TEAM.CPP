/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


	team.cpp

	GJW: 1-15-1992, original creation
	GJW: 4-9-1992, password encryption
	GJW: 6-10-1992, added LCDT define for controlling what get compiled
						 for the various libraries:
							shell (LCDT = 0)
							arcade (LCDT = A_LCDT)
							editor (LCDT = ED_LCDT)
	GJW: 3-24-1993, new format on disk (T00:)
	GJW: 4-7-1993, TeamMem functionality
	GJW: 9-26-1993, new format on disk (T01:) AND it's encrypted
	GJW: 10-18-1994, pn [0] is the nfl id of the team
	GJW: 7-18-1995, jersey & sock stripe support added (T02:)
	GJW: 8-30-1995, default team data includes stripes

	MDB	MAR 96		Football '97
	CAM	29 MAY 96	Added SetId for MultiPlayer play
	CAM	11 JUL 96	Added Home Jersey preference to LG2 data file
	CAM	14 AUG 96	Added Uniform Number color preferences to LG2 data file
	CAM	17 OCT 96	Changed default uniform number from helmet to pants
	CAM	6 NOV 96		Added default setting of uniform numbers for a new team
	MMQ	10 DEC 96   Added CopyExtra() which consolidates copying of number
							colors and plans/profiles, called from ::Copy, and from
							::Steal when creating a new league.

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	#includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include			<stdio.h>
#include			<dos.h>
#include			<string.h>
#include			<io.h>

#include			<ports.h>
#include			"dyndefs.h"
#include		<vm_mylib.h>

#include			"assertvm.h"
#include			"fileutil.h"
#include			"team.h"
#include			"lcdtutil.h"
#include			"poolaloc.h"
#include			"fbmudefs.h"
#include			"fbmufunc.h"
#include			"fbmu.h"
#include			"game.h"

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
#define MAXPATH 260
#define MAXFILE 256
#define MAXEXT		5 //256
#define	PRINT_PLAYERS_ARR 0

 //	8 game plans, home jersey, uni number colors
const int iADDITIONALDATASIZE = (GAMEPLANSIZE*8) + sizeof(DYN_8) + 2*sizeof(Team::UniformNum);
const int iADDITIONALDATASIZETEAMDATA	= (sizeof(GamePlan)*8) + sizeof(DYN_8) + 2*sizeof(Team::UniformNum);

// GJW: 7-18-1995, changed this struct
struct TeamDiskData
{
		DYN_8					id,
								confIndex, divIndex, teamIndex,
								cityId;
		DYN_8					stadiumType;
		DYN_8					ownerType;
		DYN_8					pm, am,
								manage;
		Palette::RGB		helmet,
								ltJersey,
								dkJersey,
								ltPants,
								dkPants,
								ltJerseyStripe,
								dkJerseyStripe,
								ltPantsStripe,
								dkPantsStripe,
								ltSockStripe,
								dkSockStripe;
		Team::ProPct		passOff,
								rushOff,
								passDef,
								rushDef;
		DYN_8					name [Team::NAME_LENGTH + 1],
								mascot [Team::NAME_LENGTH + 1],
								abbr [Team::ABBR_LENGTH + 1],
								stadium [Team::NAME_LENGTH + 1],
								coach [Team::COACH_NAME_LENGTH + 1],
								password [Team::PASSWORD_LENGTH + 1],
								pn [Team::PN_NAME_LENGTH + 1],
								draftPro [Team::PASSWORD_LENGTH + 1];
};

struct NewTeamDiskData
{
		DYN_8					id,
								confIndex,
								divIndex,
								teamIndex,
								manage,
								cityId;
		DYN_8					stadiumType;
		Palette::RGB		helmet,
								ltJersey,
								dkJersey,
								ltPants,
								dkPants;
		DYN_8					name [Team::NAME_LENGTH + 1],
								mascot [Team::NAME_LENGTH + 1],
								abbr [Team::ABBR_LENGTH + 1],
								stadium [Team::NAME_LENGTH + 1],
								coach [Team::COACH_NAME_LENGTH + 1];

		// GJW: 8-30-1995
		Palette::RGB		ltJerseyStripe,
								dkJerseyStripe,
								ltPantsStripe,
								dkPantsStripe,
								ltSockStripe,
								dkSockStripe;
};

// GJW: 7-18-1995, changed this struct
struct FAR TeamData
{
	void						FAR * operator new (size_t size);
	void						operator delete (void FAR *t, size_t size);

								TeamData ();
								~TeamData ();

	static PoolAlloc		*pa;	// where to get memory from

	struct NewInfo
	{
		DYN_16	rId;
		DYN_8		path [MAXPATH],
					file [MAXFILE + MAXEXT];
	};

	// new for 97 jwm
	GamePlan				gpOffCoach1, gpOffGamePlan1, gpDefCoach1, gpDefGamePlan1,
							gpOffCoach2, gpOffGamePlan2, gpDefCoach2, gpDefGamePlan2 ;
	// 10 JUL 96 CAM added jersey preference
	DYN_8					jersey;
	 // -- 14 AUG 96 CAM added uniform number preferences
	Team::UniformNum	uiLight;
	Team::UniformNum	uiDark;

	DYN_8					id;
	DYN_8					passValid;
	DYN_16				userID;

	NewInfo				FAR *niPtr;
	Roster				*roster;

							// *** this stuff gets compared by Compare ()
	DYN_8					confIndex, divIndex, teamIndex;
	DYN_8					cityId;
	DYN_8					stadiumType;
	DYN_8					ownerType;
	DYN_8					pm, am;
	DYN_8					manage;
	Palette::RGB		helmet,
							ltJersey,
							dkJersey,
							ltPants,
							dkPants,
							ltJerseyStripe,
							dkJerseyStripe,
							ltPantsStripe,
							dkPantsStripe,
							ltSockStripe,
							dkSockStripe;
	Team::ProPct		passOff,
							rushOff,
							passDef,
							rushDef;
	DYN_8					name [Team::NAME_LENGTH + 1],
							mascot [Team::NAME_LENGTH + 1],
							abbr [Team::ABBR_LENGTH + 1],
							stadium [Team::NAME_LENGTH + 1],
							coach [Team::COACH_NAME_LENGTH + 1],
							password [Team::PASSWORD_LENGTH + 1],
							pn [Team::PN_NAME_LENGTH + 1],
							draftPro [Team::PASSWORD_LENGTH + 1];
							// *** this stuff gets compared by Compare ()

	DYN_16				wins, losses, ties;
	DYN_16				ptsF, ptsA;

	DYN_8					copyFlag;

	enum
	{
		COMP_SIZE = sizeof (DYN_8) + sizeof (DYN_8) + sizeof (DYN_8) +
						sizeof (DYN_8) +
						sizeof (DYN_8) +
						sizeof (DYN_8) +
						sizeof (DYN_8) + sizeof (DYN_8) +
						sizeof (DYN_8) +
						11 * sizeof (Palette::RGB) +
						4 * sizeof (Team::ProPct) +
						0
	};
};

const MAX_NEW_TEAMS = 32;
const NUM_TEAM_ALLOC = (30 * 3) + 1;
const NUM_TEAM_ROSTER_ALLOC = (2 * NUM_TEAM_ALLOC);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	local utility prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

static void NEAR encrypt (DYN_8 *ptr);

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

INCLUDE_ASSERT;

// where to get memory from
PoolAlloc		*Team::pa,
					*Team::paePtr,
					*TeamData::pa;

static DYN_8	TMTText [] = "TMT:";

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:				Team

	FUNCTION:		Team object CONSTRUCTOR
						(2 types:
						1st loads info from disk
						2nd just creates space)

	INPUT:			fp -> vm_bfile with data to extract
						id = team id to extract from file
						newFlag = if TRUE, creating a new league, so the FILE
						is a pointer to the TEMPLATE file, not all of the info
						is contained within

						or
						-> Team to make copy of

	RETURNS:			-> created Team object
	---------------------------------------------------------------------
*/

Team::Team (char *planfilename, ReadBFile &rlf, DYN_16 tId, DYN_16 newFlag)
{
	dataPtr = new TeamData;

	#if	(LCDT == 0)

	if (newFlag)
	{
		if (rlf.Find (TMTText, tId) != -1)
		{
			NewTeamDiskData	ntdd;
			rlf.Read (&ntdd, sizeof (NewTeamDiskData));

			dataPtr->id = ntdd.id;
			dataPtr->confIndex = ntdd.confIndex;
			dataPtr->divIndex = ntdd.divIndex;
			dataPtr->teamIndex = ntdd.teamIndex;
			dataPtr->cityId = ntdd.cityId;
			dataPtr->manage = ntdd.manage;
			dataPtr->stadiumType = ntdd.stadiumType;
			dataPtr->helmet = ntdd.helmet;
			dataPtr->ltJersey = ntdd.ltJersey;
			dataPtr->dkJersey = ntdd.dkJersey;
			dataPtr->ltPants = ntdd.ltPants;
			dataPtr->dkPants = ntdd.dkPants;

			// GJW: 8-30-1995
			dataPtr->ltJerseyStripe = ntdd.ltJerseyStripe;
			dataPtr->dkJerseyStripe = ntdd.dkJerseyStripe;
			dataPtr->ltPantsStripe = ntdd.ltPantsStripe;
			dataPtr->dkPantsStripe = ntdd.dkPantsStripe;
			dataPtr->ltSockStripe = ntdd.ltSockStripe;
			dataPtr->dkSockStripe = ntdd.dkSockStripe;

			memcpy (dataPtr->name, ntdd.name, NAME_LENGTH + 1 );
			memcpy( dataPtr->mascot, ntdd.mascot, NAME_LENGTH + 1 );
			memcpy( dataPtr->abbr, ntdd.abbr, ABBR_LENGTH + 1 );
			memcpy( dataPtr->stadium, ntdd.stadium, NAME_LENGTH + 1 );
			memcpy( dataPtr->coach, ntdd.coach, COACH_NAME_LENGTH + 1 );

			dataPtr->roster = new Roster ();
			vmAssert (dataPtr->roster);
		}

		// -- Default these to better values than zero
		GSDefAdditional();
	}

	else
	{
	#endif
		Load (planfilename, rlf, tId);
		dataPtr->wins = -1;	// indicate schedule not read yet

		dataPtr->roster = new Roster (rlf, dataPtr->id);
		vmAssert (dataPtr->roster);

	#if	(LCDT == 0)

	}

	#endif
}

Team::Team (char *planfilename, ReadBFile &rlf, DYN_16 tId)
{
	dataPtr = new TeamData;
	Load (planfilename, rlf, tId);
}

Team::Team (Team *srcTeam)
{
	dataPtr = new TeamData;

	Copy (srcTeam);

	dataPtr->copyFlag = TRUE;
}

Team::Team ( void )
{
	dataPtr = new TeamData;
}

/*
	---------------------------------------------------------------------
	NAME:				~Team

	FUNCTION:		Team object DESTRUCTOR
						don't destroy the roster if this team is a copy of another

	INPUT:			-> Team object

	RETURNS:			none
	---------------------------------------------------------------------
*/

Team::~Team ( void )
{
	if (dataPtr->niPtr)
		paePtr->Delete (dataPtr->niPtr);

	delete dataPtr;
}




/*
	---------------------------------------------------------------------
	NAME:			TeamData

	FUNCTION:	construct the TeamData object

	INPUT:		none

	RETURNS:		object constructed, and stadium too
	---------------------------------------------------------------------
*/

TeamData::TeamData ( void )
{
}

TeamData::~TeamData ( void )
{
	if (roster && !copyFlag)
		delete roster;
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

void *Team::operator new (size_t size)
{
	// make sure this isn't some derived class
	vmAssert (pa && (size == sizeof (Team)));
	void	*t = pa->New ();
	vmAssert (t);

	memset (t, 0, sizeof (Team));
	return t;
}

void Team::operator delete (void *t, size_t size)
{
	// make sure this isn't some derived class
	vmAssert (pa && (size == sizeof (Team)));
	pa->Delete (t);
}



void FAR *TeamData::operator new (size_t size)
{
	// if there is no allocation object, or the size if wrong (indicating
	// a derived class), allocate out of conventional memory
	// only process items that are of the expected size
	vmAssert (pa && (size == sizeof (TeamData)));

	void	FAR *t = pa->New ();
	vmAssert (t);

	// return a 'clean slate' to the constructor
	memset (t, 0, sizeof (TeamData));
	return t;
}

void TeamData::operator delete (void FAR *t, size_t size)
{
	// only process items that are of the expected size
	vmAssert (pa && (size == sizeof (TeamData)));
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

	FUNCTION:	initialize the pointer that operators new & delete
					use for allocating Team object memory
					initialize pointer for TeamData memory management

	INPUT:		none

	RETURNS:		PoolAlloc object for Team created
						(pointer is passed to Roster, and below)
					PoolAlloc object for TeamData created
	---------------------------------------------------------------------
*/

void Team::Init ( void )
{
	paePtr = new PoolAlloc (sizeof (TeamData::NewInfo), MAX_NEW_TEAMS);
	pa = new PoolAlloc (sizeof (Team), NUM_TEAM_ROSTER_ALLOC);
	TeamData::pa = new PoolAlloc (sizeof (TeamData), NUM_TEAM_ALLOC);
	vmAssert (pa && TeamData::pa && paePtr);

	// continue on down the chain of command
	Roster::Init (pa);
}
void Team::DeInit ()
{
	delete paePtr;
	delete pa;
	delete TeamData::pa;

	// continue on down the chain of command
	Roster::DeInit ();
}

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Save

	FUNCTION:	write a "T02:" block to the file

	INPUT:		wlf-> file to write data to

	RETURNS:		file updated
	---------------------------------------------------------------------
*/

void Team::Save (char *planFile, WriteBFile &wlf)
{
	#if	(LCDT == 0) || (LCDT == 4)

	wlf.OpenBlock (T02Text);

	TeamDiskData	tdd;
	tdd.id = dataPtr->id;
	tdd.confIndex = dataPtr->confIndex;
	tdd.divIndex = dataPtr->divIndex;
	tdd.teamIndex = dataPtr->teamIndex;
	tdd.cityId = dataPtr->cityId;
	tdd.stadiumType = dataPtr->stadiumType;
	tdd.ownerType = dataPtr->ownerType;
	tdd.pm = dataPtr->pm;
	tdd.am = dataPtr->am;
	tdd.manage = dataPtr->manage;
	tdd.helmet = dataPtr->helmet;
	tdd.ltJersey = dataPtr->ltJersey;
	tdd.dkJersey = dataPtr->dkJersey;
	tdd.ltPants = dataPtr->ltPants;
	tdd.dkPants = dataPtr->dkPants;
	tdd.ltJerseyStripe = dataPtr->ltJerseyStripe;
	tdd.dkJerseyStripe = dataPtr->dkJerseyStripe;
	tdd.ltPantsStripe = dataPtr->ltPantsStripe;
	tdd.dkPantsStripe = dataPtr->dkPantsStripe;
	tdd.ltSockStripe = dataPtr->ltSockStripe;
	tdd.dkSockStripe = dataPtr->dkSockStripe;
	memcpy (&tdd.passOff, &dataPtr->passOff, 4 * sizeof (ProPct));
//	  memcpy (tdd.name, dataPtr->name,
//					  NAME_LENGTH + 1 +			  // name
//					  NAME_LENGTH + 1 +			  // mascot
//					  ABBR_LENGTH + 1 +			  // abbr
//					  NAME_LENGTH + 1 +			  // stadium
//					  COACH_NAME_LENGTH + 1 +	  // coach
//					  PASSWORD_LENGTH + 1 +		  // password
//					  PN_NAME_LENGTH + 1 +		  // pn
//					  PASSWORD_LENGTH + 1 +		  // draftPro
//					  0);
	memcpy( tdd.name,		dataPtr->name,		NAME_LENGTH + 1 );
	memcpy( tdd.mascot,	dataPtr->mascot,	NAME_LENGTH + 1 );
	memcpy( tdd.abbr,		dataPtr->abbr,		ABBR_LENGTH + 1 );
	memcpy( tdd.stadium, dataPtr->stadium, NAME_LENGTH + 1 );
	memcpy( tdd.coach,	dataPtr->coach,	COACH_NAME_LENGTH + 1 );
	memcpy( tdd.password,dataPtr->password,PASSWORD_LENGTH + 1 );
	memcpy( tdd.pn,		dataPtr->pn,		PN_NAME_LENGTH + 1 );
	memcpy( tdd.draftPro,dataPtr->draftPro,PASSWORD_LENGTH + 1 );


	encrypt (&tdd.id);
	wlf.Write (&tdd, sizeof (TeamDiskData));
	wlf.CloseBlock ();

	dataPtr->roster->Save (wlf);
	SaveAdditional(planFile) ;
	#endif	// LCDT
}

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			SavePlayersLW

	FUNCTION:	save the roster for this team, after a week in the league
					has started
					the box score screens need to know who was on the team
					AT THE TIME of the game

	INPUT:		wlf-> file to write roster to

	RETURNS:		this team's roster written to the last week file
	---------------------------------------------------------------------
*/

void Team::SavePlayersLW (WriteBFile &wlf)
{
	dataPtr->roster->Save (wlf);
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			Get/SetData

	FUNCTION:	return private data requested by user

	INPUT:		this -> Team

	RETURNS:		index of this team
	---------------------------------------------------------------------
*/

DYN_16 Team::GetCI ()
{
	return dataPtr->confIndex;
}

void Team::SetCI (DYN_16 value)
{
	dataPtr->confIndex = (char)value;
}

DYN_16 Team::GetDI ()
{
	return dataPtr->divIndex;
}

void Team::SetDI (DYN_16 value)
{
	dataPtr->divIndex = (char)value;
}

DYN_16 Team::GetTI ()
{
	return dataPtr->teamIndex;
}

void Team::SetTI (DYN_16 value)
{
	dataPtr->teamIndex = (char)value;
}

DYN_16 Team::GetCity ()
{
	return dataPtr->cityId;
}

void Team::SetCity (DYN_16 value)
{
	dataPtr->cityId = (char)value;
}

DYN_16 Team::GetWins ()
{
	return dataPtr->wins;
}

void Team::SetWins (DYN_16 value)
{
	dataPtr->wins = value;
}

DYN_16 Team::GetLosses ()
{
	return dataPtr->losses;
}

void Team::SetLosses (DYN_16 value)
{
	dataPtr->losses = value;
}

DYN_16 Team::GetTies ()
{
	return dataPtr->ties;
}

void Team::SetTies (DYN_16 value)
{
	dataPtr->ties = value;
}

DYN_16 Team::GetPF ()
{
	return dataPtr->ptsF;
}

void Team::SetPF (DYN_16 value)
{
	dataPtr->ptsF = value;
}

DYN_16 Team::GetPA ()
{
	return dataPtr->ptsA;
}

void Team::SetPA (DYN_16 value)
{
	dataPtr->ptsA = value;
}

DYN_16 Team::GetUser ()
{
	return dataPtr->userID;
}

void Team::SetUser (DYN_16 value)
{
	dataPtr->userID = value;
}

DYN_8 Team::GetStadiumType ()
{
	return dataPtr->stadiumType;
}

void Team::SetStadiumType (DYN_8 value)
{
	dataPtr->stadiumType = value;
}

DYN_8 Team::GetOwner ()
{
	return dataPtr->ownerType;
}

void Team::SetOwner (DYN_8 value)
{
	dataPtr->ownerType = value;
}

DYN_16 Team::GetPM ()
{
	return dataPtr->pm;
}

void Team::SetPM (DYN_16 value)
{
	dataPtr->pm = (char)value;
}

DYN_16 Team::GetAM ()
{
	return dataPtr->am;
}

void Team::SetAM (DYN_16 value)
{
	dataPtr->am = (char)value;
}

DYN_8 Team::GetManage ()
{
	return dataPtr->manage;
}

void Team::SetManage (DYN_8 value)
{
	dataPtr->manage = value;
}

DYN_16 Team::GetPassValid ()
{
	return dataPtr->passValid;
}

void Team::SetPassValid (DYN_16 value)
{
	dataPtr->passValid = (char)value;
}

DYN_16 Team::GetId ()
{
	return dataPtr->id;
}
void Team::SetId ( DYN_16 id )
{
	dataPtr->id = (DYN_8)id;
}

Roster *Team::GetRPtr()
{
	return dataPtr->roster;
}

DYN_16 Team::GetNFLId () const
{
	return dataPtr->pn [0];
}

void Team::ClearNFLId ()
{
	dataPtr->pn [0] = 0;
}

/*
	---------------------------------------------------------------------
	NAME:			Get/SetData

	FUNCTION:	return -> Team's string info

	INPUT:		sData = type of string wanted

	RETURNS:		->name of team
	---------------------------------------------------------------------
*/

DYN_8 FAR *Team::GetName ()
{
	return dataPtr->name;
}

void Team::SetName (DYN_8 FAR *string)
{
	strncpy (dataPtr->name, string, NAME_LENGTH);
	dataPtr->name [NAME_LENGTH] = 0;
}

DYN_8 FAR *Team::GetMascot ()
{
	return dataPtr->mascot;
}

void Team::SetMascot (DYN_8 FAR *string)
{
	strcpy (dataPtr->mascot, string);
}

DYN_8 FAR *Team::GetAbbr ()
{
	return dataPtr->abbr;
}

void Team::SetAbbr (DYN_8 FAR *string)
{
	strcpy (dataPtr->abbr, string);
}

DYN_8 FAR *Team::GetStadiumName ()
{
	return dataPtr->stadium;
}

void Team::SetStadiumName (DYN_8 FAR *string)
{
	strcpy (dataPtr->stadium, string);
}

DYN_8 FAR *Team::GetPassword ()
{
	return dataPtr->password;
}

void Team::SetPassword (DYN_8 FAR *string)
{
	strcpy (dataPtr->password, string);
}

DYN_8 FAR *Team::GetCoach ()
{
	return dataPtr->coach;
}

void Team::SetCoach (DYN_8 FAR *string)
{
	strcpy (dataPtr->coach, string);
}

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			Get/SetRGB???

	FUNCTION:	return pointer to, or set RGB triplet of certain jersey
					piece

	INPUT:		rgb-> triplet to set

	RETURNS:		-> current rgb settings
	---------------------------------------------------------------------
*/

void Team::GetRGBHelmet (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->helmet);
}

void Team::SetRGBHelmet (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->helmet, rgb);
}

void Team::GetRGBLightJersey (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->ltJersey);
}

void Team::SetRGBLightJersey (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->ltJersey, rgb);
}

void Team::GetRGBDarkJersey (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->dkJersey);
}

void Team::SetRGBDarkJersey (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->dkJersey, rgb);
}

void Team::GetRGBLightPants (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->ltPants);
}

void Team::SetRGBLightPants (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->ltPants, rgb);
}

void Team::GetRGBDarkPants (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->dkPants);
}

void Team::SetRGBDarkPants (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->dkPants, rgb);
}

// GJW: 7-18-1995, added these 12 functions
void Team::GetRGBLightJerseyStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->ltJerseyStripe);
}

void Team::SetRGBLightJerseyStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->ltJerseyStripe, rgb);
}

void Team::GetRGBDarkJerseyStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->dkJerseyStripe);
}

void Team::SetRGBDarkJerseyStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->dkJerseyStripe, rgb);
}

void Team::GetRGBLightPantsStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->ltPantsStripe);
}

void Team::SetRGBLightPantsStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->ltPantsStripe, rgb);
}

void Team::GetRGBDarkPantsStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->dkPantsStripe);
}

void Team::SetRGBDarkPantsStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->dkPantsStripe, rgb);
}

void Team::GetRGBLightSockStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->ltSockStripe);
}

void Team::SetRGBLightSockStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->ltSockStripe, rgb);
}

void Team::GetRGBDarkSockStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (rgb, &dataPtr->dkSockStripe);
}

void Team::SetRGBDarkSockStripe (Palette::RGB FAR *rgb)
{
	Palette::CopyRGB (&dataPtr->dkSockStripe, rgb);
}
#endif

void Team::GetDraftProfileName (DYN_8 FAR *string) const
{
	strcpy (string, dataPtr->draftPro);
}

void Team::SetDraftProfileName (const DYN_8 FAR *string)
{
	strcpy (dataPtr->draftPro, string);
}

DYN_8 Team::GetJersey( void )
{
	return ( dataPtr->jersey );
}

void	Team::SetJersey( DYN_8 jsy )
{
	dataPtr->jersey = jsy;
}

Team::UniformNum Team::GetNumberColorIndexDark( void )
{
	return ( dataPtr->uiDark );
}

Team::UniformNum Team::GetNumberColorIndexLight( void )
{
	return ( dataPtr->uiLight );
}

void Team::SetNumberColorIndexDark( Team::UniformNum idxD )
{
	dataPtr->uiDark = idxD;
}

void Team::SetNumberColorIndexLight( Team::UniformNum idxL )
{
	dataPtr->uiLight = idxL;
}

/*
	---------------------------------------------------------------------
	NAME:			WLTPct

	FUNCTION:	get percentage of games won

					GJW: 7-19-1992, had to change % won to distinguish
					between 3-5-0 and 3-4-1:
					games won	games lost	games tied	total pts	%
						* 4			* 0			* 1
					3	= 12		5	= 0		0	= 0		(12 + 0 + 0) / 8 = 1.5
					3	= 12		4	= 0		1	= 1		(12 + 0 + 1) / 8 = 1.625
					2	= 8		4	= 0		2	= 2		(8	 + 0 + 2) / 8 = 1.25

	INPUT:		tPtr-> team

	RETURNS:		games won/games played
	---------------------------------------------------------------------
*/

DYN_16 Team::WLTPct ( void )
{
	// get number of wins, losses, ties (summed = total games)
	DYN_16	w = dataPtr->wins,
				l = dataPtr->losses,
				t = dataPtr->ties,
				tg = w + l + t;

	if (tg)
		// each win is worth 4 points, a tie worth 1
		// (this assumes that 4 ties would equal 1 win when calculating
		// percentage of games won, but I'm pretty sure there will never
		// be 4 ties in a season (even just 1 is rare))
		return ((((4 * w) + t) * 100) / tg);

	// if you haven't played any games yet, you are just slightly better
	// than someone who has played and lost (you've won 1 percent of your games)
	else
		return 1;
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			Get/SetTrainAllocs

	FUNCTION:	return or modify a block of training allocation percentages
					(data is handled as a group per position slot (i.e. a
					pointer to the training allocations for QB is returned, or
					modified, not just one of the ratings))

	INPUT:		sId = which slot

	RETURNS:		block of percentages returned or updated
	---------------------------------------------------------------------
*/

Roster::TrainAllocs *Team::GetTrainAllocs (Roster::SlotId sId)
{
	return dataPtr->roster->GetTrainAllocs (sId);
}

void Team::SetTrainAllocs (Roster::SlotId sId, Roster::TrainAllocs *taPtr)
{
	dataPtr->roster->SetTrainAllocs (sId, taPtr);
}

/*
	---------------------------------------------------------------------
	NAME:			trainAlloc

	FUNCTION:	determine if the given team has allocated 100% of time for
					each slot

	INPUT:		tPtr-> team to check

	RETURNS:		TRUE/FALSE if training camp is set for this team
	---------------------------------------------------------------------
*/

DYN_16 Team::TrainAlloc ()
{
	return dataPtr->roster->TrainAlloc ();
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			GenerateRecord

	FUNCTION:	determine number of wins, losses, ties, points for &
					against from the schedule

	INPUT:		schedule-> where to get data from
					wins (if -1, then this hasn't been done yet, if it has,
					don't bother doing it again)

	RETURNS:		wins, losses, ties, ptsF & ptsA
	---------------------------------------------------------------------
*/

void Team::GenerateRecord (Schedule *schedule)
{
	if (dataPtr->wins == -1)
	{
		dataPtr->wins = schedule->GetTData (SCDTEAMDATA_WINS, dataPtr->id);
		dataPtr->losses = schedule->GetTData (SCDTEAMDATA_LOSSES, dataPtr->id);
		dataPtr->ties = schedule->GetTData (SCDTEAMDATA_TIES, dataPtr->id);
		dataPtr->ptsF = schedule->GetTData (SCDTEAMDATA_PF, dataPtr->id);
		dataPtr->ptsA = schedule->GetTData (SCDTEAMDATA_PA, dataPtr->id);
	}
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			Compare

	FUNCTION:	determine if 2 teams have the same data

	INPUT:		this, tPtr -> 2 teams

	RETURNS:		0 if the same
					<> 0 if different
	---------------------------------------------------------------------
*/

DYN_16 Team::Compare (Team *tPtr)
{
	GamePlan *gp1 = &(dataPtr->gpOffCoach1),
				*gp2 = &(tPtr->dataPtr->gpOffCoach1) ;

	return	memcmp (&dataPtr->confIndex,
								&tPtr->dataPtr->confIndex,
								TeamData::COMP_SIZE)							 ||
				strcmp (dataPtr->name, tPtr->dataPtr->name)			 ||
				strcmp (dataPtr->mascot, tPtr->dataPtr->mascot)		 ||
				strcmp (dataPtr->abbr, tPtr->dataPtr->abbr)			 ||
				strcmp (dataPtr->stadium, tPtr->dataPtr->stadium)	 ||
				strcmp (dataPtr->coach, tPtr->dataPtr->coach)		 ||
				strcmp (dataPtr->password, tPtr->dataPtr->password) ||
				strcmp (dataPtr->pn, tPtr->dataPtr->pn)				 ||
				strcmp (dataPtr->draftPro, tPtr->dataPtr->draftPro) ||
				// compare 8 gameplan files and paths
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				strcmp ((gp1)->GetFName(),	 (gp2)->GetFName())	||
				strcmp ((gp1++)->GetPath(), (gp2++)->GetPath()) ||
				// compare additional
				(dataPtr->jersey != tPtr->dataPtr->jersey)		||
				(dataPtr->uiDark != tPtr->dataPtr->uiDark)		||
				(dataPtr->uiLight != tPtr->dataPtr->uiLight)
				;
} // Team::Compare

/*
	---------------------------------------------------------------------
	NAME:			Copy

	FUNCTION:	duplicate the given team's info into this team's info

	INPUT:		srcTeam-> team to get info from

	RETURNS:		this-> data obtained from source team
	---------------------------------------------------------------------
*/

void Team::Copy (Team *srcTeam)
{
	DYN_8 cFlag = dataPtr->copyFlag;

	// gameplans are just pointers so copy them later
	memcpy (&(dataPtr->id), &(srcTeam->dataPtr->id),
		sizeof (TeamData)- iADDITIONALDATASIZETEAMDATA );

	// consolidate copying of new data 12/10/96 mcqueen
	CopyExtra( srcTeam );

	dataPtr->copyFlag = cFlag;
}


/*
	---------------------------------------------------------------------
	NAME:			CopyExtra

	FUNCTION:	duplicate the given team's "new" info into this team's info

	INPUT:		srcTeam-> team to get info from

	RETURNS:		this-> data obtained from source team
	
	NOTE:			Called by ::Copy and ::Steal
					12/10/96 mcqueen
	---------------------------------------------------------------------
*/
void Team::CopyExtra( Team *srcTeam )
{
	// -- copy "new" data
	dataPtr->jersey = srcTeam->dataPtr->jersey;	// 11 JUL 96
	dataPtr->uiDark = srcTeam->dataPtr->uiDark;	// 14 AUG 96
	dataPtr->uiLight = srcTeam->dataPtr->uiLight;// 14 AUG 96

	GamePlan *gp1 = &(dataPtr->gpOffCoach1);
	GamePlan *gp2 = &(srcTeam->dataPtr->gpOffCoach1);

	// copy 8 gameplan files and paths
	for( int i=0; i<8; i++, gp1++, gp2++ )
	{
		gp1->SetFName( gp2->GetFName() );
		gp1->SetPath( gp2->GetPath() );
	}
} /* CopyExtra */

#if	(LCDT == 0)
// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Steal

	FUNCTION:	when setting up a new league, a user may fill in team
					data by selecting 'DEFAULT'
					the default league file is passed in
					the team that this team should get its data from is
					kept in the MANAGE byte

	INPUT:		srcFP -> file with source team in it (just players &
						playbook)
					pName, fName -> strings of the league that src team belongs
						to (for getting rosters & playbooks)
					(This version is called during ConfDivSetup ())

					or
					pName, fName -> strings of the league that src team belongs
						to (for getting rosters & playbooks)
					srcTeam-> team to get info from
					tst = Steal all info from source team flag

					GJW: 4-9-1992, get roster info too, by saving away the
						full path & id number to the roster, which will be
						read in and converted when this league is saved

	RETURNS:		this team's data setup like the default team's
	---------------------------------------------------------------------
*/

void Team::Steal( ReadBFile &rlf, DYN_8 *pName, DYN_8 *fName )
{
	// make sure the DESTINATION team gets a block of memory for
	// saving info BEFORE the 'junk' team is constructed/destructed
	DYN_8		planfilename [MAXPATH];
	makeFileName (planfilename, pName, fName, ".lg2");
	SaveNewInfo (0, 0, 0);

	// construct the given source team temporarily
	Team defTeam (planfilename, rlf, dataPtr->manage, FALSE);
//	Steal( pName, fName, &defTeam, STEAL_PONLY );
// steal extra stuff too (number colors, plans/profiles) 12/10/96 mcqueen
	Steal( pName, fName, &defTeam, STEAL_EXTRA );

	// teams built from default data are computer owned & operated as well
	// (i.e. manage no longer contains steal id)
	dataPtr->ownerType = OWNER_COMPUTER;
	dataPtr->manage = OWNER_COMPUTER;
}

void Team::Steal (DYN_8 *pName, DYN_8 *fName, Team *srcTeam, DYN_8 tst)
{
	switch( tst )
	{
	case STEAL_ALL:
		dataPtr->cityId = srcTeam->dataPtr->cityId;
		dataPtr->stadiumType = srcTeam->dataPtr->stadiumType;
		dataPtr->ownerType = srcTeam->dataPtr->ownerType;
		dataPtr->pm = srcTeam->dataPtr->pm;
		dataPtr->am = srcTeam->dataPtr->am;
		dataPtr->manage = srcTeam->dataPtr->manage;
		dataPtr->helmet = srcTeam->dataPtr->helmet;
		dataPtr->ltJersey = srcTeam->dataPtr->ltJersey;
		dataPtr->dkJersey = srcTeam->dataPtr->dkJersey;
		dataPtr->ltPants = srcTeam->dataPtr->ltPants;
		dataPtr->dkPants = srcTeam->dataPtr->dkPants;
		dataPtr->ltJerseyStripe = srcTeam->dataPtr->ltJerseyStripe;
		dataPtr->dkJerseyStripe = srcTeam->dataPtr->dkJerseyStripe;
		dataPtr->ltPantsStripe = srcTeam->dataPtr->ltPantsStripe;
		dataPtr->dkPantsStripe = srcTeam->dataPtr->dkPantsStripe;
		dataPtr->ltSockStripe = srcTeam->dataPtr-> ltSockStripe;
		dataPtr->dkSockStripe = srcTeam->dataPtr-> dkSockStripe;
		Roster::SlotId sId;
		for (sId = Roster::QB_SLOT; sId < Roster::P_SLOT; sId = Roster::SlotId(sId + 1))
			SetTrainAllocs (sId, srcTeam->GetTrainAllocs (sId));

		memcpy (&dataPtr->passOff, &srcTeam->dataPtr->passOff, 4 * sizeof (ProPct));

		strcpy (dataPtr->name, srcTeam->dataPtr->name);
		strcpy (dataPtr->mascot, srcTeam->dataPtr->mascot);
		strcpy (dataPtr->abbr, srcTeam->dataPtr->abbr);
		strcpy (dataPtr->stadium, srcTeam->dataPtr->stadium);
		strcpy (dataPtr->coach, srcTeam->dataPtr->coach);
		strcpy (dataPtr->password, srcTeam->dataPtr->password);
		strcpy (dataPtr->pn, srcTeam->dataPtr->pn);
		strcpy (dataPtr->draftPro, srcTeam->dataPtr->draftPro);
	
		// fall through
	case STEAL_EXTRA:
		// consolidate copying of new data 12/10/96 mcqueen
		CopyExtra( srcTeam );

		// fall through
	default:
		// and always save path for player's info
		SaveNewInfo (pName, fName, srcTeam->dataPtr->id);
		break;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			NewLeagueInfo

	FUNCTION:	when a newly created league is being saved, go and get
					the roster & playbook info previously saved away, and
					create modified (new) rosters & playbooks

	INPUT:		fpNew-> player destination file
					pId = first new player id on this roster
					dPath, dName -> string of league destination file
					niPtr-> stuff saved away when team was created

	RETURNS:		nextPlayerId
					new rosters & playbooks
					niPtr-> deleted
	---------------------------------------------------------------------
*/

DYN_16 Team::NewLeagueInfo (PlayerRdWr &prw,
							DYN_16 pId,
							DYN_8 *dPath, DYN_8 *dName,
							DYN_8 proBowlFlag)
{
	// save stuff from new Info area, before it is deleted
	DYN_16	rId = dataPtr->niPtr->rId;
	DYN_8		sPath [MAXPATH],
				sName [MAXFILE + MAXEXT];
	strcpy (sPath, dataPtr->niPtr->path);
	strcpy (sName, dataPtr->niPtr->file);

	paePtr->Delete (dataPtr->niPtr);
	dataPtr->niPtr = 0;

	// copy over roster's players
	dataPtr->roster->LoadNewPlayers (sPath, sName, rId, dataPtr->id);
	pId = dataPtr->roster->SaveNewPlayers (prw, pId);

	// copy over game plan files (exhibition & league)
	if (!proBowlFlag)
	{
		CopyGamePlan (dPath, dName, sPath, sName, rId, 'F');
		CopyGamePlan (dPath, dName, sPath, sName, rId, 'M');
	}

	return pId;
}

/*
	---------------------------------------------------------------------
	NAME:			NewLeagueCheck

	FUNCTION:	when a newly created league is being saved, and an old
					league is being deleted, make sure that one the new team's
					data is not obtained from the league being deleted

	INPUT:		pName, fName -> strings of league being deleted

	RETURNS:		TRUE/FALSE if this team USES data from a league about
					to be deleted
	---------------------------------------------------------------------
*/

DYN_16 Team::NewLeagueCheck (DYN_8 *pName, DYN_8 *fName)
{
	DYN_8 *ptr,
			buffer [16];

	// remove the ".lge" part
	strcpy (buffer, dataPtr->niPtr->file);
	ptr = strchr (buffer, '.');
	if (ptr)
		*ptr = 0;

	return !(strcmp (pName, dataPtr->niPtr->path) || strcmp (fName, buffer));
}
#endif	// LCDT

/*
	---------------------------------------------------------------------
	NAME:			LoadRoster

	FUNCTION:	re-read the team's roster info

	INPUT:		rlf-> file to get data from

	RETURNS:		team's roster initialized again
	---------------------------------------------------------------------
*/

void Team::LoadRoster (ReadBFile &rlf)
{
	if (!dataPtr->roster)
	{
		dataPtr->roster = new Roster (rlf, dataPtr->id);
		vmAssert (dataPtr->roster);
	}

	else
		dataPtr->roster->LoadRoster (rlf, dataPtr->id);
}

/*
	---------------------------------------------------------------------
	NAME:			Load/DeletePlayers

	FUNCTION:	have roster read in the players,
					or delete the ones it has created

	INPUT:		fp -> file to get player info from (if 0, then just
					inquiring if players have actually been loaded)
					maxPId = maximum legal player id for this league

	RETURNS:		roster/FALSE indicator if rosters loaded or not
	---------------------------------------------------------------------
*/

Roster *Team::LoadPlayers (PlayerRdWr *prwPtr)
{
	return dataPtr->roster->LoadPlayers (prwPtr) ? dataPtr->roster : 0;
}

void Team::DeletePlayers ()
{
	dataPtr->roster->DeletePlayers ();
}

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			PickPlayers

	FUNCTION:	let the roster fill itself out, from the array of player
					lists

	INPUT:		pList = array of player lists, so roster can grab the best

	RETURNS:		roster filled with players
	---------------------------------------------------------------------
*/

DYN_16 Team::PickPlayers (GenList<RosPlayer> *pList [], DYN_16 newId)
{
	return dataPtr->roster->PickPlayers (pList, newId, dataPtr->id);
}

/* new for All Pro Team JWM 12/26/96
   ---------------------------------------------------------------------
   NAME:       PickPlayers

   FUNCTION:   fill the roster from the all pro team

   INPUT:      List of the all proteam players

   RETURNS:    roster filled with players
   ---------------------------------------------------------------------
*/

DYN_16 Team::PickPlayers (GenList<RosPlayer> *pList, DYN_16 newId)
{
   return dataPtr->roster->PickPlayers (pList, newId, dataPtr->id);
}

/*
	---------------------------------------------------------------------
	NAME:			Manage

	FUNCTION:	perform the AI of computer management for some portion
					of the team

	INPUT:		mData = what type of management to perform
					subType = what element of the type
					for example:
						mData = TEAMMANAGE_ROSTER (roster manipulations)
							subType = MANAGE_PRESEASON, MANAGE_CDRAFT

	RETURNS:		if the team has computer management, actions performed
	---------------------------------------------------------------------
*/

DYN_16 Team::Manage (DYN_16 mData, DYN_16 subType)
{
	DYN_16	update = FALSE;

	if (dataPtr->manage == OWNER_COMPUTER)
	{
		switch (mData)
		{
			case MANAGE_ROSTER:
				update = AIRoster (subType);
				break;

			case MANAGE_TRAININGCAMP:
				dataPtr->roster->AITrain ();
				update = TRUE;
				break;
		}
	}

	return update;
}

/*
	---------------------------------------------------------------------
	NAME:			Train

	FUNCTION:	modify all players abrs based on percentage of time
					devoted to each attribute

	INPUT:		TrainAllocs, players

	RETURNS:		all players' attributes bumped upwards
	---------------------------------------------------------------------
*/

void Team::Train ( void )
{
	dataPtr->roster->Train ();
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			AddPlayer

	FUNCTION:	a player is being added to the team via:
					a college draftee
					a trade
					a free agent signing

					the 2 players are swapped (i.e. one is removed from the
					team, the other added)
					(the destination slot is always OPEN)

	INPUT:		newPPtr-> player being added to the roster
					oldPPtr-> player leaving

	RETURNS:		pointer to the player leaving
					newPPtr-> joins the team
					oldPPTr-> looking for work
	---------------------------------------------------------------------
*/

RosPlayer *Team::AddPlayer (RosPlayer *newPPtr, RosPlayer *oldPPtr)
{
	return dataPtr->roster->AddPlayer (newPPtr, oldPPtr, dataPtr->manage == OWNER_COMPUTER);
}

/*
	---------------------------------------------------------------------
	NAME:			DraftSelect

	FUNCTION:	have the roster pick who it needs

	INPUT:		pList -> array of draft choices
					collegeFlag = draft type (college/supplemental)

	RETURNS:		index of pList to select top player from
					-1 means quit (roster is full, or this team is human
					controlled)
	---------------------------------------------------------------------
*/

DYN_16 Team::DraftSelect (GenList<RosPlayer> *pList [], Roster::DraftSelectType dsFlag)
{
	DYN_16	value = -1,
				manage = dataPtr->manage;

	// when handling CEI players,
	// always call roster to release player, but don't sign another if human
	// owned
	if (dsFlag == Roster::DSTYPE_CEI)
		manage = OWNER_COMPUTER;

	// if team is computer managed, OR, needs to handle CEI, call the
	// roster manager. If CEI, roster will only release a player, not sign
	// a new one
	if (manage == OWNER_COMPUTER)
		value = dataPtr->roster->DraftSelect (pList, dsFlag,
												dataPtr->manage == OWNER_COMPUTER);

	return value;
}
#endif

#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			TradeEval

	FUNCTION:	have the roster decide if trade is good or bad

	INPUT:		p1Ptr, p2Ptr -> players involved

	RETURNS:		roster's evaluation of trade
	---------------------------------------------------------------------
*/

TradeStatus
Team::TradeEval (RosPlayer **p1Ptr, RosPlayer **p2Ptr)
{
	TradeStatus ts = TRADE_PROPOSED;

	if (dataPtr->manage == OWNER_COMPUTER)
		ts = dataPtr->roster->TradeEval (p1Ptr, p2Ptr);

	return ts;
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			GetPlayer

	FUNCTION:	get a pointer to the player at the given SLOT & POSITION or,
					with the given ID

	INPUT:		sId = which slot
					index = which one
					(or id)

	RETURNS:		-> player
					NULL = invalid sId/index
	---------------------------------------------------------------------
*/

RosPlayer *Team::GetPlayer (Roster::SlotId sId, DYN_16 index)
{
	return dataPtr->roster->GetPlayer (sId, index);
}

RosPlayer *Team::GetPlayer (DYN_16 id)
{
	return dataPtr->roster->GetPlayer (id);
}
#endif


#if	(LCDT == 0)
/*
	---------------------------------------------------------------------
	NAME:			WannaTrade

	FUNCTION:	call the roster to decide if a trade for the given player
					would be a good thing

	INPUT:		pos = position another team wants
					ptPtr-> player team is willing to give up

	RETURNS:		->Player THIS team would be willing to give up
					0 means GO TO HELL!
	---------------------------------------------------------------------
*/

DYN_16
Team::WannaTrade (RosPlayer **dest,
						Roster::SlotId *sIds, RosPlayer **ptPtr)
{
	return dataPtr->roster->WannaTrade (dest, sIds, ptPtr);
}

/*
	---------------------------------------------------------------------
	NAME:			Get/SetRatWeights

	FUNCTION:	return/set roster's ratingWeights block

	INPUT:		rwPtr-> weights to set

	RETURNS:		->current settings
	---------------------------------------------------------------------
*/

RatingWeights *Team::GetRatWeights ()
{
	return dataPtr->roster->GetRatWeights ();
}

void Team::SetRatWeights (RatingWeights *rwPtr)
{
	dataPtr->roster->SetRatWeights (rwPtr);
}

/*
	---------------------------------------------------------------------
	NAME:			RemoveNoTrades

	FUNCTION:	remove all players the team isn't interested in parting
					with, if computer controlled

	INPUT:		pList-> list of players at a certain general position

	RETURNS:		roster called to weed list
	---------------------------------------------------------------------
*/

DYN_16 Team::RemoveNoTrades (GenList<RosPlayer> *pList, const RosPlayer *pPtr) const
{
	return dataPtr->roster->RemoveNoTrades (pList, pPtr, dataPtr->manage == OWNER_COMPUTER);
}

/*
	---------------------------------------------------------------------
	NAME:			BuildTradePlayerList

	FUNCTION:	Used by TEAMRSTR.CPP to combine functionality of several
					functions previously called seperately that eventually
					fill a list with players from a team that are available
					for trade.

					GJW: 9-9-1996, modified for multi-player trades

	INPUT:		tom = TradeOffer manager reference
					sIds-> position per trade slot
					pPtrs-> players per trade slot

	RETURNS:		number of trades added to tom
	---------------------------------------------------------------------
*/

DYN_32
Team::BuildTradePlayerList (TOManager &tom,
										Roster::SlotId *sIds,
										const RosPlayer **pPtrs)
{
	return dataPtr->roster->BuildTradePlayerList (tom, sIds, pPtrs,
													dataPtr->manage == OWNER_COMPUTER);
}

/*
	---------------------------------------------------------------------
	NAME:			AssignedFull

	FUNCTION:	Determine if all assigned spots contain players
					(necessary step for calling arcade game or play editor)
					(return the results from Roster's checking)

	INPUT:		none

	RETURNS:		TRUE/FALSE if all assigned slots are filled
	---------------------------------------------------------------------
*/

DYN_16 Team::AssignedFull ( void )
{
	return dataPtr->roster->AssignedFull ();
}

/*
	---------------------------------------------------------------------
	NAME:			GenerateStrategies

	FUNCTION:	call the roster to figure out what drafting strategies
					should be implemented this upcomming season

	INPUT:		ranks-> array of league rankings for stats

	RETURNS:		roster's drafting strategies set, based on last year's
					statistical placement within the league
	---------------------------------------------------------------------
*/

void Team::GenerateStrategies (DYN_U_16 *ranks)
{
	dataPtr->roster->GenerateStrategies (ranks);
}

/*
	---------------------------------------------------------------------
	NAME:			HealPlayers

	FUNCTION:	call the roster to have players heal themselves

	INPUT:		team's roster

	RETURNS:		injured players are healed
	---------------------------------------------------------------------
*/

void Team::HealPlayers ( void )
{
	dataPtr->roster->HealPlayers ();
}
#endif

#if	(LCDT == 0) || (LCDT == 4)
/*
	---------------------------------------------------------------------
	NAME:			SeasonReset

	FUNCTION:	at the end of the last season, everybody gets fully healed

	INPUT:		team's roster

	RETURNS:		injured players are healed
	---------------------------------------------------------------------
*/

void Team::SeasonReset (DYN_16 force)
{
	dataPtr->roster->SeasonReset (force);
}
#endif

/*
	---------------------------------------------------------------------
	NAME:			Print

	FUNCTION:	print team data to screen

	INPUT:		magazine -> structure

	RETURNS:		string printed to buffptr
	---------------------------------------------------------------------
*/

void Team::Print ( void )
{
	#if	PRINT_PLAYERS_ARR
	#if	(LCDT == 0)

	DYN_8		buffer [16];

	sprintf (buffer, "league28.a%02d", dataPtr->id);
	Printer	APrinter (buffer);
	dataPtr->roster->Print (&APrinter);
	#endif
	#endif
}






/*
	=====================================================================
									PRIVATE METHODS
	=====================================================================
*/

// GJW: 7-18-1995, changed this function
/*
	---------------------------------------------------------------------
	NAME:			Load

	FUNCTION:	read in a Team file

	INPUT:		tId = id of team to find in file

	RETURNS:		new one loaded
	---------------------------------------------------------------------
*/

void Team::Load (char *planfilename, ReadBFile &rlf, DYN_16 tId)
{
	if (rlf.Find (T02Text, tId) != -1)
	{
		TeamDiskData	tdd;
		rlf.Read (&tdd, sizeof (TeamDiskData));
		encrypt (&tdd.id);

		dataPtr->id = tdd.id;
		dataPtr->confIndex	= tdd.confIndex;
		dataPtr->divIndex		= tdd.divIndex;
		dataPtr->teamIndex	= tdd.teamIndex;
		dataPtr->cityId		= tdd.cityId;
		dataPtr->stadiumType = tdd.stadiumType;
		dataPtr->ownerType	= tdd.ownerType;

		dataPtr->pm = tdd.pm;
		dataPtr->am = tdd.am;
		dataPtr->manage = tdd.manage;
		dataPtr->helmet = tdd.helmet;

		dataPtr->ltJersey = tdd.ltJersey;
		dataPtr->dkJersey = tdd.dkJersey;
		dataPtr->ltPants	= tdd.ltPants;
		dataPtr->dkPants	= tdd.dkPants;
		dataPtr->ltJerseyStripe = tdd.ltJerseyStripe;
		dataPtr->dkJerseyStripe = tdd.dkJerseyStripe;
		dataPtr->ltPantsStripe	= tdd.ltPantsStripe;
		dataPtr->dkPantsStripe	= tdd.dkPantsStripe;
		dataPtr->ltSockStripe	= tdd.ltSockStripe;
		dataPtr->dkSockStripe	= tdd.dkSockStripe;

		memcpy (&dataPtr->passOff, &tdd.passOff,	4 * sizeof (ProPct));
		memcpy (dataPtr->name,		tdd.name,		NAME_LENGTH + 1 );
		memcpy( dataPtr->mascot,	tdd.mascot,		NAME_LENGTH + 1 );
		memcpy( dataPtr->abbr,		tdd.abbr,		ABBR_LENGTH + 1 );
		memcpy( dataPtr->stadium,	tdd.stadium,	NAME_LENGTH + 1 );
		memcpy( dataPtr->coach,		tdd.coach,		COACH_NAME_LENGTH + 1 );
		memcpy( dataPtr->password, tdd.password,	PASSWORD_LENGTH + 1 );
		memcpy( dataPtr->pn,			tdd.pn,			PN_NAME_LENGTH + 1 );
		memcpy( dataPtr->draftPro, tdd.draftPro,	PASSWORD_LENGTH + 1 );

		GetAdditional(planfilename) ;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SaveNewInfo

	FUNCTION:	when a league is being created, new teams are created as well
					the team's roster and playbook information (i.e. where they
					come from) is kept around until the league is actually saved,
					then the files are loaded, modified, and saved under new
					names

	INPUT:		pName, fName -> full pathname to the source team
					rId = roster id to copy later (if 0, then the caller just
							want to allocate memory for filling later)

	RETURNS:		team->dataPtr->niPtr set
	---------------------------------------------------------------------
*/

#if	(LCDT == 0)

void Team::SaveNewInfo (DYN_8 *pName, DYN_8 *fName, DYN_16 rId)
{
	if (!dataPtr->niPtr)
	{
		dataPtr->niPtr = (TeamData::NewInfo FAR *) paePtr->New ();
		vmAssert (dataPtr->niPtr);
	}

	if (rId)
	{
		strcpy (dataPtr->niPtr->path, pName);
		strcpy (dataPtr->niPtr->file, fName);
		dataPtr->niPtr->rId = rId;
	}
}

/*
	---------------------------------------------------------------------
	NAME:			AIRoster

	FUNCTION:	perform computer owned AI for roster manipulations
					based on the event
					if the roster has been loaded, call the manager

	INPUT:		mrData = ManageRoster (i.e. what kind of moves to make)

	RETURNS:		roster updated
	---------------------------------------------------------------------
*/

DYN_16 Team::AIRoster (DYN_16 mrData)
{
	DYN_16	update = FALSE;

	if (dataPtr->roster)
		update = dataPtr->roster->Manage (mrData);

	return update;
}

/*
	---------------------------------------------------------------------
	NAME:			CopyGamePlan

	FUNCTION:	copy an exhibition or league game plan to the NEW team

	INPUT:		dPath, dName-> destination file strings
					sPath, sName-> source file strings
					rId = destination id
					elFlag = exhibition/league

	RETURNS:		gameplans copied if found
	---------------------------------------------------------------------
*/

void Team::CopyGamePlan (DYN_8 *dPath, DYN_8 *dName, DYN_8 *sPath, DYN_8 *sName,
							DYN_16 rId, DYN_8 elFlag)
{
	DYN_8		buffer [MAXPATH],
				extBuff [MAXEXT],
				*ptr = makeFileName (0, sPath, PBText, "");
	sprintf (extBuff, ".%c%02d", elFlag, rId);
	makeFileName (buffer, ptr, sName, extBuff);

	FILE	*sFP = my_fopen (buffer, "rb");
	if (sFP)
	{
		ptr = makeFileName (0, dPath, PBText, "");
		sprintf (extBuff, ".%c%02d", elFlag, dataPtr->id);
		makeFileName (buffer, ptr, dName, extBuff);
		FILE	*dFP = my_fopen (buffer, "wb");

		if (dFP)
		{
			copyFile (dFP, sFP);
			my_fclose (dFP);
		}

		my_fclose (sFP);
	}
}

/*
	---------------------------------------------------------------------
	NAME:			SetProPct

	FUNCTION:	Sets profile percentages (weights, really) for a team

	INPUT:		profile weights, offense and defense, pass and rush,
						aggressive, balanced, conservative

	RETURNS:		None.
	---------------------------------------------------------------------
*/
void Team::SetProPct(Team::ProPct r_o, Team::ProPct p_o,
					 Team::ProPct r_d, Team::ProPct p_d)
{
	dataPtr->rushOff = r_o;
	dataPtr->passOff = p_o;
	dataPtr->rushDef = r_d;
	dataPtr->passDef = p_d;
}

/*
	---------------------------------------------------------------------
	NAME:			GetProPct

	FUNCTION:	Gets profile percentages (weights, really) for a team

	INPUT:		profile weights, offense and defense, pass and rush,
						aggressive, balanced, conservative

	RETURNS:		None.

	DPS Added 10/5/94
	---------------------------------------------------------------------
*/
void Team::GetProPct(Team::ProPct* r_o, Team::ProPct* p_o,
					 Team::ProPct* r_d, Team::ProPct* p_d)
{
	*r_o = dataPtr->rushOff;
	*p_o = dataPtr->passOff;
	*r_d = dataPtr->rushDef;
	*p_d = dataPtr->passDef;

}
#endif









/*
	=====================================================================
									LOCAL UTILITIES
	=====================================================================
*/

static void encrypt (DYN_8 *ptr)
{
	DYN_8		xor = 0x69 * (*ptr++);
	DYN_16	size = sizeof (TeamDiskData) - sizeof (DYN_8);

	while (size--)
	{
		*ptr = *ptr ^ xor++;
		ptr++;
	}
}

void Team::GSDefAdditional ( void )
{
	char szDir[256] = "";

	 // -- Take off the drive letter for a relative path!
	if ( cwd[1] == ':' )
	{
		memset( szDir, 0, sizeof(szDir) );
		memcpy( szDir, &cwd[2], (strlen(cwd) - 2) );
	}
	else
	{
		strcpy( szDir, cwd );
	}

	 // -- Initialize plans/profiles
	GamePlan		*pPtr = &(dataPtr->gpOffCoach1) ;
	for (DYN_16 i = 0; i < Game::MAX_PLANS; i++)
	{
		DYN_8		fName [MAXFILE + MAXEXT];
		// odd indices are plans
		if (i & 1)
			// first 2 of each set are offense, second defense
			sprintf (fName, "%s.PLN", i & 2 ? "DEF" : "OFF");

		// even indices are profiles
		else
			sprintf (fName, "%s%d.PRF",
					// first 2 of each set are offense, second defense
					i & 2 ? "DEF" : "OFF",

					// if i <= 3, half indicator is 0
					// if i >= 4, half indicator is 1
					(i >> 2) + 1);

//		pPtr->SetPath (cwd);
		pPtr->SetPath ( szDir );
		pPtr->SetFName (fName);
		pPtr++;
	}

	 // -- Initialize the rest
	dataPtr->jersey  = JSY_DARK;
	dataPtr->uiDark  = UNIFORMNUM_PANTS;
	dataPtr->uiLight = UNIFORMNUM_PANTS;
}

GamePlan *
Team::gpOffCoach1()
{
	return &(dataPtr->gpOffCoach1) ;
}
GamePlan *
Team::gpOffGamePlan1()
{
	return &(dataPtr->gpOffGamePlan1) ;
}
GamePlan *
Team::gpDefCoach1()
{
	return &(dataPtr->gpDefCoach1) ;
}
GamePlan *
Team::gpDefGamePlan1()
{
	return &(dataPtr->gpDefGamePlan1) ;
}
GamePlan *
Team::gpOffCoach2()
{
	return &(dataPtr->gpOffCoach2) ;
}
GamePlan *
Team::gpOffGamePlan2()
{
	return &(dataPtr->gpOffGamePlan2) ;
}
GamePlan *
Team::gpDefCoach2()
{
	return &(dataPtr->gpDefCoach2) ;
}
GamePlan *
Team::gpDefGamePlan2()
{
	return &(dataPtr->gpDefGamePlan2) ;
}


void Team::GetAdditional (char *planfilename)
{
	FILE *fp ;
	char buff[iADDITIONALDATASIZE] ;
	char *Ptr ;
	long offset ;
	DYN_8 tmp = 0;

	if (planfilename == NULL) {
		goto error ;
	}
	fp = fopen(planfilename,"rb") ;
	if ((fp == (FILE *) -1) || (fp == (FILE *) 0)) {
		goto error ;
	}
	offset = iADDITIONALDATASIZE * (dataPtr->id - 1) ;
	if(fseek(fp,offset,SEEK_SET) != 0) {
		goto error ;
	}
	if (fread(buff, 1, iADDITIONALDATASIZE, fp) != iADDITIONALDATASIZE) {
		goto error ;
	}
	fclose(fp) ;

	Ptr = buff ;
	memcpy(gpOffCoach1()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpOffGamePlan1()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpDefCoach1()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpDefGamePlan1()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpOffCoach2()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpOffGamePlan2()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpDefCoach2()->GetPath(),Ptr,GAMEPLANSIZE) ;
	Ptr += GAMEPLANSIZE ;
	memcpy(gpDefGamePlan2()->GetPath(),Ptr,GAMEPLANSIZE) ;

	 // -- get past the last one
	Ptr += GAMEPLANSIZE ;

	 // -- get the rest
	memcpy( &tmp, Ptr, sizeof(tmp) );
	SetJersey( tmp );
	Ptr += sizeof(tmp);

	UniformNum uitmp;
	memcpy( &uitmp, Ptr, sizeof(uitmp) );
	SetNumberColorIndexDark( uitmp );
	Ptr += sizeof(uitmp);

	memcpy( &uitmp, Ptr, sizeof(uitmp) );
	SetNumberColorIndexLight( uitmp );
	Ptr += sizeof(uitmp);

	return ;

error:
	// if no extra file exist get the defaults
	GSDefAdditional() ;
	if (fp) {
		fclose(fp) ;
	}
	return ;
}

void
Team::SaveAdditional (char *planfilename)
{
	if (planfilename == NULL) {
		return ;
	}
	FILE *fp = fopen(planfilename,"r+b") ;
	long offset ;

	// if file doesn't exist make a new if we are writing the first team
	if (((fp == (FILE*) -1) || (fp == (FILE*) 0)) && (dataPtr->id == 1)) {
		fp = fopen(planfilename,"wb") ;
	}

	if ((fp == (FILE*) -1) || (fp == (FILE*) 0)) {
		return ;
	}

	offset = iADDITIONALDATASIZE * (dataPtr->id - 1) ;
	if(fseek(fp,offset,SEEK_SET) != 0) {
		return ;
	}

	fwrite(gpOffCoach1()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpOffGamePlan1()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpDefCoach1()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpDefGamePlan1()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpOffCoach2()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpOffGamePlan2()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpDefCoach2()->GetPath(),GAMEPLANSIZE,1,fp) ;
	fwrite(gpDefGamePlan2()->GetPath(),GAMEPLANSIZE,1,fp) ;

	DYN_8 tmp = 0;
	 // -- added 11 JUL 96 CAM
	tmp = GetJersey();
	fwrite( &tmp, sizeof(DYN_8), 1, fp );

	UniformNum uitmp;
	uitmp = GetNumberColorIndexDark();
	fwrite( &uitmp, sizeof(UniformNum), 1, fp );

	uitmp = GetNumberColorIndexLight();
	fwrite( &uitmp, sizeof(UniformNum), 1, fp );

	fclose(fp) ;
	return ;
}

 // Team.cpp
