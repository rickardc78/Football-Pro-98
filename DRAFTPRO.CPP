/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


	draftpro.cpp

	This module contains the DraftProfile class.

	09/02/94 TJS:  Created.\

	CAM	25 JUL 96	Fixed Load and Saved to deal with the DraftProfileChoice
							 structure correctly.  (It had enum's, which changed 
							 size from Watcom to VC++
	CAM	27 FEB 97	Renamed MAXFILE to MAXDEFUALTFILENAME to avoid
							 confusiopn with the real MAXFILE

컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


#if (LCDT == 0)


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


// #include <dir.h>
#include <memory.h>
#include <string.h>

#include "ports.h"
#include "dyndefs.h"

#include "draftpro.h"
#include "fileutil.h"
#include "lgefile.h"
#include "rosplyr.h"
#include "roster.h"

#include "fbmu.h"


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Consts
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */
#define MAXDEFAULTFILENAME 9

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Enumerations
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Classes and structures
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


DYN_8		draftProfile_ext[] = ".DP";


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Local data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


static DYN_8	draftProfile_block[]	= "DP0:";

static DYN_16	draftProfile_asCounts[ Roster::MAX_DRAFT_LIST ] =
{
	Roster::NUM_QB_SLOTS,
	Roster::NUM_RB_SLOTS,
	Roster::NUM_REC_SLOTS,
	Roster::NUM_OL_SLOTS,
	Roster::NUM_DL_SLOTS,
	Roster::NUM_LB_SLOTS,
	Roster::NUM_DB_SLOTS,
	Roster::NUM_K_SLOTS,
	Roster::NUM_P_SLOTS,
};

static DYN_8*	draftProfile_defaultFiles[] =
{
	"DEFAULT0",
	"DEFAULT1",
	"DEFAULT2",
	"DEFAULT3",
	"DEFAULT4",
	"DEFAULT5",
	"DEFAULT6",
	"DEFAULT7",
	"DEFAULT8",
	"DEFAULT9",

	NULL
};


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Local function prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Public member functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/*
	---------------------------------------------------------------------
	NAME:			DraftProfile

	FUNCTION:	This is the class constructor.  It puts the object into
					a known state that's the same as the object state after
					Reset() is called.

	INPUT:		void

	RETURNS:		void
	---------------------------------------------------------------------
*/

DraftProfile::DraftProfile( void )
{
	Reset();
}

/*
	---------------------------------------------------------------------
	NAME:			operator==

	FUNCTION:	The comparision operator.

	INPUT:		rhs
						The draft profile that will be compared.

	RETURNS:		DYN_8
						If both draft profiles are the same, TRUE will be
						returned.  Otherwise, FALSE will be returned.
	---------------------------------------------------------------------
*/

DYN_8	DraftProfile::operator==( const DraftProfile& rhs )
{
	return ( numChoicesValid == rhs.numChoicesValid &&
				!memcmp( choice,
							rhs.choice,
							sizeof( DraftProfileChoice ) * NUM_ROUNDS ) );
}

/*
	---------------------------------------------------------------------
	NAME:			Reset

	FUNCTION:	Clears the position and rating criteria for each choice
					for each round (i.e. sets up a clean slate).  Afterwards,
					IsComplete() will return FALSE until each round contains
					a valid choice set by PutChoice().

	INPUT:		void

	RETURNS:		void
	---------------------------------------------------------------------
*/

void DraftProfile::Reset( void )
{
	DYN_16					round;
	DraftProfileChoice*	choicePtr;

	for ( round = NUM_ROUNDS, choicePtr = choice;
			round;
			round--, choicePtr++ )
	{
		choicePtr->pos	= RosPlayer::POS_NONE;
		choicePtr->r1	= RosPlayer::NO_RATING;
		choicePtr->r2	= RosPlayer::NO_RATING;
	}

	numChoicesValid = 0;
}

/*
	---------------------------------------------------------------------
	NAME:			GetChoice

	FUNCTION:	Get a specific round's position to be filled and rating
					criteria.

	INPUT:		round
						A DYN_16 in the range [0, NUM_ROUNDS) that represents
						what round's choice to get.
					aChoice
						Pointer to a DraftProfileChoice that will be
						initialized with the specified round's choice.

	RETURNS:		DYN_8
						If the round and aChoice were valid, returns TRUE and
						initializes aChoice with the choice cooresponding to
						the specified round.  Otherwise, FALSE is retured.
					aChoice
						On success the specified DraftProfileChoice contains
						a copy of the choice cooresponding to the specified
						round.
	---------------------------------------------------------------------
*/

DYN_8 DraftProfile::GetChoice( const DYN_16 round,
								 DraftProfileChoice* aChoice ) const
{
	if ( round >= 0 && round < NUM_ROUNDS && aChoice )
	{
		*aChoice = choice[ round ];

		return TRUE;
	}

	return FALSE;
}

/*
	---------------------------------------------------------------------
	NAME:			PutChoice

	FUNCTION:	Put a specific round's position to be filled and rating
					criteria.

	INPUT:		round
						A DYN_16 in the range [0, NUM_ROUNDS) that represents
						what round's choice to put.
					aChoice
						Pointer to a DraftProfileChoice that contains the
						specified round's new choice.  For each choice field
						(position, rating 1 or rating 2) if it's not
						initialized, it should contain the values
						RosPlayer::POS_NONE or RosPlayer::NO_RATING.

	RETURNS:		DYN_8
						If the round and aChoice were valid, returns TRUE and
						the choice cooresponding to specified round is
						initialized with aChoice.  Otherwise, FALSE is
						retured.
	---------------------------------------------------------------------
*/

DYN_8	DraftProfile::PutChoice( const DYN_16 round,
								 const DraftProfileChoice* aChoice )
{
	if ( round >= 0 &&
		  round < NUM_ROUNDS &&
		  aChoice &&
		  ( ( aChoice->pos >= 0 &&
				aChoice->pos < RosPlayer::POS_MAX_SPEC ) ||
			 aChoice->pos == RosPlayer::POS_NONE ) &&
		  ( aChoice->r1 < RosPlayer::RATING_MAX ||
			 aChoice->r1 == RosPlayer::NO_RATING ) &&
		  ( aChoice->r2 < RosPlayer::RATING_MAX ||
			 aChoice->r2 == RosPlayer::NO_RATING ) )
	{
		DYN_8	invalidChoice = ( aChoice->pos == RosPlayer::POS_NONE ||
										aChoice->r1 == RosPlayer::NO_RATING ||
										aChoice->r2 == RosPlayer::NO_RATING );

		DraftProfileChoice*	choicePtr = &choice[ round ];

		// Was the choice previously not fully specified?
		if ( choicePtr->pos == RosPlayer::POS_NONE ||
			  choicePtr->r1 == RosPlayer::NO_RATING ||
			  choicePtr->r2 == RosPlayer::NO_RATING )
		{
			if ( !invalidChoice )
				numChoicesValid++;
		}
		else if ( invalidChoice )
			numChoicesValid--;

		*choicePtr = *aChoice;

		return TRUE;
	}

	return FALSE;
}

/*
	---------------------------------------------------------------------
	NAME:			Load

	FUNCTION:	Loads a draft profile from disk using the specified path
					and file name.

	INPUT:		pName
						The path name of the draft profile to load.
					fName
						The file name of the draft profile to load.

	RETURNS:		DYN_8
						If the specified draft profile was loaded, TRUE is
						returned.  Otherwise, FALSE is returned and the object
						state is reset.
	---------------------------------------------------------------------
*/

DYN_8	DraftProfile::Load( DYN_8* pName, DYN_8* fName )
{
	DYN_8			result = FALSE;
	ReadBFile	bFile;
	DraftProfileChoiceF	dpcTmp[ NUM_ROUNDS ];

	if ( bFile.Open( makeFileName( 0,
											 pName,
											 fName,
											 draftProfile_ext ) ) )
	{
		if ( bFile.Find( draftProfile_block, 1 ) != -1 )
		{
			if ( bFile.Read( dpcTmp, sizeof( DraftProfileChoiceF ) * NUM_ROUNDS ) == 1 )
			{
				 // -- copy temp stuff to real thing
				for ( int i = 0; i < NUM_ROUNDS; i++ )
				{
					choice[i].pos	= static_cast<RosPlayer::Pos>(dpcTmp[i].pos);
					choice[i].r1	= static_cast<RosPlayer::Rating>(dpcTmp[i].r1);
					choice[i].r2	= static_cast<RosPlayer::Rating>(dpcTmp[i].r2);
				}

				if ( bFile.Read( &numChoicesValid, sizeof( DYN_8 ) ) == 1 )
					result = TRUE;
			}
		}
//		if ( bFile.Find( draftProfile_block, 1 ) != -1 &&
//			  // 09/22/94 TJS:  Glen's Read function does not return the
//			  //						number of items read as advertised.  It
//			  //						always returns 1.
//			  bFile.Read( choice, sizeof( DraftProfileChoice ) * NUM_ROUNDS ) == 1 &&
//			  bFile.Read( &numChoicesValid, sizeof( DYN_8 ) ) == 1 )
//
////			  bFile.Read( choice, sizeof( DraftProfileChoice ) * NUM_ROUNDS ) == ( sizeof( DraftProfileChoice ) * NUM_ROUNDS ) &&
////			  bFile.Read( &numChoicesValid, sizeof( DYN_8 ) ) == sizeof( DYN_8 ) )
//
//			result = TRUE;

		bFile.Close();
	}

	if ( !result )
		Reset();

	return result;
}

/*
	---------------------------------------------------------------------
	NAME:			Save

	FUNCTION:	Saves a draft profile to disk using the specified path
					and file name.

	INPUT:		pName
						The path name of the draft profile to save.
					fName
						The file name of the draft profile to save.

	RETURNS:		DYN_8
						If the specified draft profile was saved, TRUE is
						returned.  Otherwise, FALSE is returned.
	---------------------------------------------------------------------
*/

DYN_8	DraftProfile::Save( DYN_8* pName, DYN_8* fName ) const
{
	DYN_8			result = FALSE;
	WriteBFile	bFile;
	DraftProfileChoiceF	dpcTmp[ NUM_ROUNDS ];

	if ( bFile.Open( makeFileName( 0,
											 pName,
											 fName,
											 draftProfile_ext ) ) )
	{
//		if ( bFile.OpenBlock( draftProfile_block ) &&
//#if 1
//			  // 09/22/94 TJS:  Glen's Write function does not return the
//			  //						number of items written as advertised.  It
//			  //						always returns 1.
//			  bFile.Write( choice, sizeof( DraftProfileChoice ) * NUM_ROUNDS ) == 1 &&
//			  bFile.Write( &numChoicesValid, sizeof( DYN_8 ) ) == 1 &&
//#else
//			  bFile.Write( choice, sizeof( DraftProfileChoice ) * NUM_ROUNDS ) == ( sizeof( DraftProfileChoice ) * NUM_ROUNDS ) &&
//			  bFile.Write( &numChoicesValid, sizeof( DYN_8 ) ) == sizeof( DYN_8 ) &&
//#endif
//			  bFile.CloseBlock() )
//			result = TRUE;

		 // -- copy profiles to tmp area for disk copy
		for ( int i = 0; i < NUM_ROUNDS; i++ )
		{
			dpcTmp[i].pos	= choice[i].pos;
			dpcTmp[i].r1	= choice[i].r1;
			dpcTmp[i].r2	= choice[i].r2;
		}

		if ( bFile.OpenBlock( draftProfile_block ) &&
			  // 09/22/94 TJS:  Glen's Write function does not return the
			  //						number of items written as advertised.  It
			  //						always returns 1.
			  bFile.Write( dpcTmp, sizeof( DraftProfileChoiceF ) * NUM_ROUNDS ) == 1 &&
			  bFile.Write( &numChoicesValid, sizeof( DYN_8 ) ) == 1 &&
			  bFile.CloseBlock() )
			result = TRUE;

		bFile.Close();
	}

	return result;
}

/*
	---------------------------------------------------------------------
	NAME:			GetASR

	FUNCTION:	Determines how many Assigned Slots are Remaining in the
					draft profile with the exclusion of one round which is
					treated as empty.

	INPUT:		roundEmpty
						The specified round is considered to be empty or
						invalid when determining the counts.
					asrCounts[]
						An array of DYN_16s that will contain counts of how
						many assigned slots in the draft profile have not been
						filled.

	RETURNS:		DYN_8
						If both roundEmpty and asrCounts are valid, asrCounts
						will contain counts of how many assigned slots are
						remaining in the draft profile (the roundEmpty round
						is treated as empty in this calculation) and TRUE will
						be returned.  Otherwise, FALSE is returned.
	---------------------------------------------------------------------
*/

DYN_8	DraftProfile::GetASR( DYN_16 roundEmpty,
							 DYN_16 asrCounts[ Roster::MAX_DRAFT_LIST ] ) const
{
	if ( roundEmpty >= 0 &&
		  roundEmpty < NUM_ROUNDS &&
		  asrCounts )
	{
		// Initally no position are filled in any slot
		memset( asrCounts, 0, sizeof( DYN_16 ) * Roster::MAX_DRAFT_LIST );

		DYN_16					round;
		DraftProfileChoice*	choicePtr = ( DraftProfileChoice* ) choice;

		// Determine how many positions are filled for each slot by
		// examining each round.
		for ( round = 0;
				round < NUM_ROUNDS;
				round++, choicePtr++ )
			if ( round != roundEmpty && !IsEmpty( choicePtr ) )
				asrCounts[ Roster::PosToSlot( choicePtr->pos ) ]++;

		DYN_16	slot;
		DYN_16*	asCountPtr	= draftProfile_asCounts;
		DYN_16*	asrCountPtr	= asrCounts;

		// Now determine for each slot how many positions are empty.
		for ( slot = Roster::MAX_DRAFT_LIST;
				slot;
				slot--, asCountPtr++, asrCountPtr++ )
		{
			DYN_16	remaining = *asCountPtr - *asrCountPtr;

			*asrCountPtr = ( remaining > 0 ) ? remaining : 0;
		}

		return TRUE;
	}

	return FALSE;
}

// GJW: 10-8-1994
/*
   ---------------------------------------------------------------------
   NAME:       GetData

   FUNCTION:   rather than query for every single choice during the
               draft, just have object copy the info into a buffer

   INPUT:      dest-> where to copy the info

   RETURNS:    destination filled for the user
   ---------------------------------------------------------------------
*/

void DraftProfile::GetData (void FAR *dest)
{
   if (dest)
      memcpy (dest, choice, sizeof (choice));
}

/*
   ---------------------------------------------------------------------
	NAME:			IsDefault

	FUNCTION:	Determines if the specified file name is the same as one
					of the default draft profile file names.

	INPUT:		fName
						The name of the file to check.

	RETURNS:		DYN_8
						A boolean that if TRUE, indicates that the specified
						file name is a default draft profile file name.  If it
						is not, FALSE is returned.
   ---------------------------------------------------------------------
*/

DYN_8	DraftProfile::IsDefault( const DYN_8* fName )
{
	if ( fName )
	{
		DYN_8**	filePtr = draftProfile_defaultFiles;

		while ( *filePtr )
		{
			if ( !strnicmp( fName, *filePtr, MAXDEFAULTFILENAME - 1 ) )
				return TRUE;

			filePtr++;
		}
	}

	return FALSE;
}

/*
   ---------------------------------------------------------------------
	NAME:			GetRandomDefault

	FUNCTION:	Gets the name of a randomly selected default draft
					profile.

	INPUT:		fName
						A pointer to a string that will contain the randomly
						chosen default draft profile name.

	RETURNS:		fName
						If fName is valid, the string it points to will
						contain a copy of one of the default draft profile
						names.
   ---------------------------------------------------------------------
*/

void DraftProfile::GetRandomDefault( DYN_8* fName )
{
	if ( fName )
	{
		DYN_8		fileCount	= 0;
		DYN_8**	filePtr		= draftProfile_defaultFiles;

		// Determine how many default draft profile names are available
		while ( *filePtr )
		{
			fileCount++;
			filePtr++;
		}

		// Make sure there is some default names
		if ( fileCount )
		{
			// Randomly pick one of the default names
			DYN_8*	dpName = draftProfile_defaultFiles[ getPosRand () % fileCount ];

			// Make a copy of the selected default name
			strcpy( fName, dpName );
		}
		else
			fName[ 0 ] = 0;
	}
}


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Protected member functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Private member functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Local functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


#endif


// DraftPro.cpp
