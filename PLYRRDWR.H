/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   plyrrdwr.h

   the PlayerRdWr class

   GJW:	9-24-1994	ported over from baseball
	MMQ	14-Feb-97	Added PLYRRDWR_ACCESS to class definition.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _PLYRRDWR_H
#define  _PLYRRDWR_H

#include "dosfile.h"
#include "encrypt.h"

#define PLAYERRDWR_ACCESS			PROCACCESS

typedef struct 
{
   DYN_8    ratings [8/*RATING_MAX*/];
} PlyrRatings;

typedef struct
{
   DYN_16   points;
   DYN_8    type,
            week;
} PlyrInjuryData;

 // -- file read version
typedef struct 
{
   DYN_16			id;
   DYN_8				firstName [12/*NAME_LEN*/ + 1],
						lastName [12 /*NAME_LEN*/ + 1];
   PlyrRatings		pbr;
   PlyrInjuryData injury;
   DYN_8				posGen,
						posSpecific;
   PlyrRatings		abr;
   DYN_8				years;
} PlyrDiskDataF;

 // -- actual version (like RosPlayerData)
typedef struct 
{
   DYN_16			id;
   DYN_8				firstName [12/*NAME_LEN*/ + 1],
						lastName [12 /*NAME_LEN*/ + 1];
   PlyrRatings		pbr;
   PlyrInjuryData injury;
   int				posGen,			// these are defines(!!) in RosPlayer
						posSpecific;
   PlyrRatings		abr;
   DYN_8				years;
} PlyrDiskData;


class PLAYERRDWR_ACCESS PlayerRdWr
{
private:
   // member functions

   // data members
   PlyrDiskDataF      data;


protected:
   // member functions

   // data members
   DOSFILE        dFile;
   Encryption     encrypt;
   DYN_16         maxId;   // largest id of player in file
                           // (determined by file's size)

public:
   // member functions
                  PlayerRdWr (void);
                  ~PlayerRdWr (void);

   DYN_16         Open (const DYN_8 *fName);
   DYN_16         IsOpen ();
   DYN_16         Create (const DYN_8 *fName);
   DYN_16         Close ();
   DYN_16         Read (const DYN_16 id);
   DYN_16         Write ();

   void           GetData (void FAR *dest) const;
   void           SetData (void FAR *src);

   DYN_16         GetMaxId () const;

 // data members (hopefully none!)
	enum
	{
	   NAME_LEN = 12,
	   BASE_PLAYER_ID = 100,
	   BASE_FILE_ID = BASE_PLAYER_ID - 1,
	};
	
	enum Rating
	{
	      RATING_AC,     // acceleration
  		   RATING_AG,     // agility
	      RATING_DI,     // discipline
	      RATING_EN,     // endurance
	      RATING_HA,     // hands
  	      RATING_IN,     // intelligence
	      RATING_SP,     // speed
	      RATING_ST,     // strength
	      RATING_MAX,
	};


};

// inline functions
inline DYN_16
PlayerRdWr::GetMaxId () const
{
   return maxId;
}

#endif   // _PLYRRDWR_H
