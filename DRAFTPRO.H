/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


	draftpro.h

	This module contains the DraftProfile class.

	09/02/94 TJS:  Created
	
	CAM	25 JUL 96	Added DraftProfileChoiceF for reading from the disk
							 properly
							 .
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


#ifndef  _DRAFTPRO_H
#define  _DRAFTPRO_H


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


#include "ports.h"
#include "dyndefs.h"

#include "rosplyr.h"
#include "roster.h"


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Constants
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Enumerations
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Classes and structures
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


struct DraftProfileChoice
{
	RosPlayer::Pos		pos;	// Position to fill
	RosPlayer::Rating	r1,	// Rating criterion 1
							r2;	// Rating criterion 2
};
struct DraftProfileChoiceF
{
	DYN_8 /*RosPlayer::Pos*/		pos;	// Position to fill
	DYN_8 /*RosPlayer::Rating*/	r1,	// Rating criterion 1
											r2;	// Rating criterion 2
};

class DraftProfile
{
	public:

      // Types

		enum
		{
			NUM_ROUNDS	= 53,
		};

   
      // Member functions

				DraftProfile (void);

		DYN_8	operator==( const DraftProfile& rhs );

		void	Reset (void);
		DYN_8	GetChoice( const DYN_16 round,
							  DraftProfileChoice* aChoice ) const;
		DYN_8	PutChoice( const DYN_16 round,
							  const DraftProfileChoice* aChoice );

		DYN_8	Load( DYN_8* pName, DYN_8* fName );
		DYN_8	Save( DYN_8* pName, DYN_8* fName ) const;

		DYN_8	IsComplete (void) const;

		DYN_8	GetASR( DYN_16 roundEmpty,
						  DYN_16 asrCounts[ Roster::MAX_DRAFT_LIST ] ) const;

      // GJW: 10-8-1994
      void  GetData (void FAR *dest);

		static DYN_8	IsEmpty( const DraftProfileChoice* aChoice );

		static DYN_8	IsDefault( const DYN_8* fName );

		static void		GetRandomDefault( DYN_8* fName );

	private:

      // Data members

		DraftProfileChoice	choice[ NUM_ROUNDS ];
		DYN_8						numChoicesValid;
};


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Global data
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


extern DYN_8	draftProfile_ext[];


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Global function prototypes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
	Inline Functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */


inline DYN_8
DraftProfile::IsComplete (void) const
{
	return ( numChoicesValid == NUM_ROUNDS );
}

inline DYN_8
DraftProfile::IsEmpty( const DraftProfileChoice* aChoice )
{
	return ( aChoice->pos == RosPlayer::POS_NONE ||
				aChoice->r1 == RosPlayer::NO_RATING ||
				aChoice->r2 == RosPlayer::NO_RATING );
}


#endif


