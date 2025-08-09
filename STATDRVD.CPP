/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   statdrvd.cpp

   the Derived Statistic classes

   GJW: 7-14-1992, original creation
   GJW: 9-30-1994, 2 point conversions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   #includes
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#include       <string.h>

#include       <ports.h>
#include       "dyndefs.h"

#include       "assertvm.h"
#include       "fileutil.h"
#include       "statdrvd.h"
#include       "fbmu.h"

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
INCLUDE_ASSERT;

/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   functions
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*
   ---------------------------------------------------------------------
   NAME:          StatisticSum

   FUNCTION:      StatisticSum object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticSum object
   ---------------------------------------------------------------------
*/

StatisticSum::StatisticSum ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticSum

   FUNCTION:      StatisticSum object DESTRUCTOR

   INPUT:         -> StatisticSum object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticSum::~StatisticSum ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the Statistic object

   INPUT:      fp-> FILE to get data from
               t = statistic type
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticSum::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.total = getShort (fp);
   data.best = getShort (fp);
   data.tds = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticSum::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_YARDS]) = data;
}

void
StatisticSum::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticSum::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticSum::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_YARDS]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticSum::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_YARDS];

   dPtr->total += data.total;

   if ((DYN_16) data.best > (DYN_16) dPtr->best)
      dPtr->best = data.best;

   dPtr->tds += data.tds;
}



/*
   ---------------------------------------------------------------------
   NAME:          StatisticPAT

   FUNCTION:      StatisticPAT object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPAT object
   ---------------------------------------------------------------------
*/

StatisticPAT::StatisticPAT ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPAT

   FUNCTION:      StatisticPAT object DESTRUCTOR

   INPUT:         -> StatisticPAT object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPAT::~StatisticPAT ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPAT object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPAT::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.made = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPAT::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_MADE]) = data;
}

void
StatisticPAT::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticPAT::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticPAT::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_MADE]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPAT::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_MADE];

   dPtr->made += data.made;
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:          StatisticPOG

   FUNCTION:      StatisticPAT object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPAT object
   ---------------------------------------------------------------------
*/

StatisticPOG::StatisticPOG ()
{
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPOG

   FUNCTION:      StatisticPOG object DESTRUCTOR

   INPUT:         -> StatisticPOG object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPOG::~StatisticPOG ()
{
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPOG object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPOG::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.playerId = getShort (fp) ;
   data.teamId = getShort (fp) ;
   data.opponentTeamId = getShort (fp) ;
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPOG::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_PLAYERID]) = data;
}

void
StatisticPOG::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticPOG::FillBuff (ptr);
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticPOG::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_PLAYERID]);
}

// HOF JWM 9/12/96
/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPOG::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_PLAYERID];

   dPtr->playerId = data.playerId;
}





/*
   ---------------------------------------------------------------------
   NAME:          StatisticFG

   FUNCTION:      StatisticFG object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticFG object
   ---------------------------------------------------------------------
*/

StatisticFG::StatisticFG ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticFG

   FUNCTION:      StatisticFG object DESTRUCTOR

   INPUT:         -> StatisticFG object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticFG::~StatisticFG ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticFG object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticFG::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.range10 = getShort (fp);
   data.range20 = getShort (fp);
   data.range30 = getShort (fp);
   data.range40 = getShort (fp);
   data.range50 = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticFG::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_RANGE10]) = data;
}

void
StatisticFG::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticFG::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticFG::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_RANGE10]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticFG::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Data  *dPtr = (Data *) &ptr [STATOFFSET_RANGE10];

   // each word (2 bytes) contains attemps & made in a byte
   DYN_U_8     att = (dPtr->range10 >> 8) + (data.range10 >> 8),
               made = (dPtr->range10 & 0xff) + (data.range10 & 0xff);

   dPtr->range10 = (att << 8) | made;

   att = (dPtr->range20 >> 8) + (data.range20 >> 8),
   made = (dPtr->range20 & 0xff) + (data.range20 & 0xff);
   dPtr->range20 = (att << 8) | made;

   att = (dPtr->range30 >> 8) + (data.range30 >> 8),
   made = (dPtr->range30 & 0xff) + (data.range30 & 0xff);
   dPtr->range30 = (att << 8) | made;

   att = (dPtr->range40 >> 8) + (data.range40 >> 8),
   made = (dPtr->range40 & 0xff) + (data.range40 & 0xff);
   dPtr->range40 = (att << 8) | made;

   att = (dPtr->range50 >> 8) + (data.range50 >> 8),
   made = (dPtr->range50 & 0xff) + (data.range50 & 0xff);
   dPtr->range50 = (att << 8) | made;

   if (Statistic::data.number > ((Statistic::Data *) ptr)->number)
      ((Statistic::Data *) ptr)->number = Statistic::data.number;
}



/*
   ---------------------------------------------------------------------
   NAME:          StatisticRush

   FUNCTION:      StatisticRush object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticRush object
   ---------------------------------------------------------------------
*/

StatisticRush::StatisticRush ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticRush

   FUNCTION:      StatisticRush object DESTRUCTOR

   INPUT:         -> StatisticRush object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticRush::~StatisticRush ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticPass

   FUNCTION:      StatisticPass object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPass object
   ---------------------------------------------------------------------
*/

StatisticPass::StatisticPass ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPass

   FUNCTION:      StatisticPass object DESTRUCTOR

   INPUT:         -> StatisticPass object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPass::~StatisticPass ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPass object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPass::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   StatisticSum::Load (fp, t, i);
   data.comps = getShort (fp);
   data.ints = getShort (fp);
   data.sacks = getShort (fp);
   data.sackYards = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPass::FillBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_COMPS]) = data;
}

void
StatisticPass::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticPass::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticPass::ReadBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_COMPS]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPass::UpdateBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_COMPS];

   dPtr->comps += data.comps;
   dPtr->ints += data.ints;
   dPtr->sacks += data.sacks;
   dPtr->sackYards += data.sackYards;
}



/*
   ---------------------------------------------------------------------
   NAME:          StatisticPassRec

   FUNCTION:      StatisticPassRec object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPassRec object
   ---------------------------------------------------------------------
*/

StatisticPassRec::StatisticPassRec ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPassRec

   FUNCTION:      StatisticPassRec object DESTRUCTOR

   INPUT:         -> StatisticPassRec object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPassRec::~StatisticPassRec ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticInt

   FUNCTION:      StatisticInt object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticInt object
   ---------------------------------------------------------------------
*/

StatisticInt::StatisticInt ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticInt

   FUNCTION:      StatisticInt object DESTRUCTOR

   INPUT:         -> StatisticInt object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticInt::~StatisticInt ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticPunt

   FUNCTION:      StatisticPunt object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPunt object
   ---------------------------------------------------------------------
*/

StatisticPunt::StatisticPunt ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPunt

   FUNCTION:      StatisticPunt object DESTRUCTOR

   INPUT:         -> StatisticPunt object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPunt::~StatisticPunt ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPunt object

   INPUT:      fp-> FILE to get data from
               t = statistic type
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPunt::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   StatisticSum::Load (fp, t, i);
   data.blocks = getShort (fp);
   data.rets = getShort (fp);
   data.retYards = getShort (fp);
   data.touchBacks = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPunt::FillBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_BLOCKS]) = data;
}

void
StatisticPunt::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticPunt::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticPunt::ReadBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_BLOCKS]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPunt::UpdateBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_BLOCKS];

   dPtr->blocks += data.blocks;
   dPtr->rets += data.rets;
   dPtr->retYards += data.retYards;
   dPtr->touchBacks += data.touchBacks;
}



/*
   ---------------------------------------------------------------------
   NAME:          StatisticPuntRet

   FUNCTION:      StatisticPuntRet object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticPuntRet object
   ---------------------------------------------------------------------
*/

StatisticPuntRet::StatisticPuntRet ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticPuntRet

   FUNCTION:      StatisticPuntRet object DESTRUCTOR

   INPUT:         -> StatisticPuntRet object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticPuntRet::~StatisticPuntRet ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPuntRet object

   INPUT:      fp-> FILE to get data from
               t = statistic type
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPuntRet::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   StatisticSum::Load (fp, t, i);
   data.fairs = getShort (fp);
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPuntRet::FillBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_FAIRS]) = data;
}

void
StatisticPuntRet::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticPuntRet::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticPuntRet::ReadBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_FAIRS]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticPuntRet::UpdateBuff (DYN_U_16 FAR *ptr)
{
   StatisticSum::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_FAIRS];

   dPtr->fairs += data.fairs;
}





/*
   ---------------------------------------------------------------------
   NAME:          StatisticKickRet

   FUNCTION:      StatisticKickRet object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticKickRet object
   ---------------------------------------------------------------------
*/

StatisticKickRet::StatisticKickRet ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticKickRet

   FUNCTION:      StatisticKickRet object DESTRUCTOR

   INPUT:         -> StatisticKickRet object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticKickRet::~StatisticKickRet ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticFumble

   FUNCTION:      StatisticFumble object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticFumble object
   ---------------------------------------------------------------------
*/

StatisticFumble::StatisticFumble ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticFumble

   FUNCTION:      StatisticFumble object DESTRUCTOR

   INPUT:         -> StatisticFumble object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticFumble::~StatisticFumble ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticFumbleRec

   FUNCTION:      StatisticFumbleRec object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticFumbleRec object
   ---------------------------------------------------------------------
*/

StatisticFumbleRec::StatisticFumbleRec ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticFumbleRec

   FUNCTION:      StatisticFumbleRec object DESTRUCTOR

   INPUT:         -> StatisticFumbleRec object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticFumbleRec::~StatisticFumbleRec ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticSack

   FUNCTION:      StatisticSack object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticSack object
   ---------------------------------------------------------------------
*/

StatisticSack::StatisticSack ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticSack

   FUNCTION:      StatisticSack object DESTRUCTOR

   INPUT:         -> StatisticSack object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticSack::~StatisticSack ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticSafety

   FUNCTION:      StatisticSafety object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticSafety object
   ---------------------------------------------------------------------
*/

StatisticSafety::StatisticSafety ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticSafety

   FUNCTION:      StatisticSafety object DESTRUCTOR

   INPUT:         -> StatisticSafety object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticSafety::~StatisticSafety ()
{
}




/*
   ---------------------------------------------------------------------
   NAME:          StatisticTackle

   FUNCTION:      StatisticTackle object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticTackle object
   ---------------------------------------------------------------------
*/

StatisticTackle::StatisticTackle ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticTackle

   FUNCTION:      StatisticTackle object DESTRUCTOR

   INPUT:         -> StatisticTackle object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticTackle::~StatisticTackle ()
{
}



// HOF JWM 9/10/96
StatisticPenalty::StatisticPenalty ()
{
}


StatisticPenalty::~StatisticPenalty ()
{
}



StatisticHOF::StatisticHOF ()
{
}


StatisticHOF::~StatisticHOF ()
{
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPOG object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHOF::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.confIndex  = getShort (fp) ;
   data.pointCount = getShort (fp) ;
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHOF::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_CONFINDEX]) = data;
}

void
StatisticHOF::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticHOF::FillBuff (ptr);
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticHOF::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_CONFINDEX]);
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHOF::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_CONFINDEX];

   dPtr->confIndex  = data.confIndex ;
   dPtr->pointCount += data.pointCount ;
}


StatisticHALLOFAME::StatisticHALLOFAME ()
{
}


StatisticHALLOFAME::~StatisticHALLOFAME ()
{
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       Load

   FUNCTION:   read in the data for the StatisticPOG object

   INPUT:      fp-> FILE to get data from
               t = type of statistic
               i = id of player stat is for

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHALLOFAME::Load (FILE *fp, DYN_16 t, DYN_16 i)
{
   Statistic::Load (fp, t, i);
   data.pointCount  = getShort (fp) ;
   data.YearRetired = getShort (fp) ;
   data.YearElected = getShort (fp) ;
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHALLOFAME::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_POINTCOUNT]) = data;
}

void
StatisticHALLOFAME::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticHALLOFAME::FillBuff (ptr);
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticHALLOFAME::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_POINTCOUNT]);
}

// HOF JWM 9/20/96
/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticHALLOFAME::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_POINTCOUNT];

   dPtr->pointCount  = data.pointCount ;
   dPtr->YearRetired = data.YearRetired ;
   dPtr->YearElected = data.YearElected ;
}



/*
   ---------------------------------------------------------------------
   NAME:          StatisticTeamFirstDowns

   FUNCTION:      StatisticTeamFirstDowns object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticTeamFirstDowns object
   ---------------------------------------------------------------------
*/

StatisticTeamFirstDowns::StatisticTeamFirstDowns ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticTeamFirstDowns

   FUNCTION:      StatisticTeamFirstDowns object DESTRUCTOR

   INPUT:         -> StatisticTeamFirstDowns object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticTeamFirstDowns::~StatisticTeamFirstDowns ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of team stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamFirstDowns::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_TRUSHING]) = data;
}

void
StatisticTeamFirstDowns::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticTeamFirstDowns::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticTeamFirstDowns::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_TRUSHING]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamFirstDowns::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Data  *dPtr = (Data *) &ptr [STATOFFSET_TRUSHING];

   ((Statistic::Data *) ptr)->number += data.rushing + data.passing + data.penalty;
   dPtr->rushing += data.rushing;
   dPtr->passing += data.passing;
   dPtr->penalty += data.penalty;
}

/*
   ---------------------------------------------------------------------
   NAME:          StatisticTeamAttDowns

   FUNCTION:      StatisticTeamAttDowns object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticTeamAttDowns object
   ---------------------------------------------------------------------
*/

StatisticTeamAttDowns::StatisticTeamAttDowns ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticTeamAttDowns

   FUNCTION:      StatisticTeamAttDowns object DESTRUCTOR

   INPUT:         -> StatisticTeamAttDowns object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticTeamAttDowns::~StatisticTeamAttDowns ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of team stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamAttDowns::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_TATT3]) = data;
}

void
StatisticTeamAttDowns::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticTeamAttDowns::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticTeamAttDowns::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_TATT3]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamAttDowns::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_TATT3];

   dPtr->attempt3 += data.attempt3;
   dPtr->made3 += data.made3;
   dPtr->attempt4 += data.attempt4;
   dPtr->made4 += data.made4;
}

/*
   ---------------------------------------------------------------------
   NAME:          StatisticTeamMisc

   FUNCTION:      StatisticTeamMisc object CONSTRUCTOR

   INPUT:         none

   RETURNS:       -> created StatisticTeamMisc object
   ---------------------------------------------------------------------
*/

StatisticTeamMisc::StatisticTeamMisc ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:          ~StatisticTeamMisc

   FUNCTION:      StatisticTeamMisc object DESTRUCTOR

   INPUT:         -> StatisticTeamMisc object

   RETURNS:       none
   ---------------------------------------------------------------------
*/

StatisticTeamMisc::~StatisticTeamMisc ()
{
}

/*
   ---------------------------------------------------------------------
   NAME:       FillBuff

   FUNCTION:   fill the given buffer with all of this stat's pertinent
               data

   INPUT:      ptr-> where to put data
               (optional:
               t = stat type
               i = id of player stat is for)

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamMisc::FillBuff (DYN_U_16 FAR *ptr)
{
   Statistic::FillBuff (ptr);
   *((Data *) &ptr [STATOFFSET_TPENALTIES]) = data;
}

void
StatisticTeamMisc::FillBuff (DYN_U_16 FAR *ptr, DYN_16 t, DYN_16 i)
{
   Statistic::data.type = t;
   Statistic::data.id = i;
   StatisticTeamMisc::FillBuff (ptr);
}

/*
   ---------------------------------------------------------------------
   NAME:       ReadBuff

   FUNCTION:   fill the object with data from ptr
               (the opposite of FillBuff)

   INPUT:      ptr-> where to get data from

   RETURNS:    object's data filled in
   ---------------------------------------------------------------------
*/

void
StatisticTeamMisc::ReadBuff (DYN_U_16 FAR *ptr)
{
   Statistic::ReadBuff (ptr);
   data = *((Data *) &ptr [STATOFFSET_TPENALTIES]);
}

/*
   ---------------------------------------------------------------------
   NAME:       UpdateBuff

   FUNCTION:   this statistical record already exists in the database
               the new data is to ADDED to the existing data

   INPUT:      ptr-> where to get old/put new data

   RETURNS:    statistic objects
   ---------------------------------------------------------------------
*/

void
StatisticTeamMisc::UpdateBuff (DYN_U_16 FAR *ptr)
{
   Statistic::UpdateBuff (ptr);
   Data  *dPtr = (Data *) &ptr [STATOFFSET_TPENALTIES];

   ((Statistic::Data *) ptr)->number = Statistic::data.number;
   dPtr->penalties += data.penalties;
   dPtr->yardsLost += data.yardsLost;
   dPtr->timePossession += data.timePossession;
   dPtr->finalScore = data.finalScore;
   dPtr->fumblesLost += data.fumblesLost;
}







/*
   =====================================================================
                           PUBLIC METHODS
   =====================================================================
*/







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
