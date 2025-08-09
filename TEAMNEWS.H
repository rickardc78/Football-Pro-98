/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   teamnews.h

   GJW: 12-08-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _TEAMNEWS_H
#define  _TEAMNEWS_H

#include    "genlist.h"
#include    "tradnews.h"

// root team news class
class TNBase
{
   friend class TeamNews;

private:
//    member functions

//    data members


protected:
//    member functions

//    data members

public:
//    member functions
               TNBase ();
               TNBase (DYN_16 tId);
      virtual  ~TNBase ();

//    data members
};

// Trade Info
class TNTrade : public TNBase
{
      friend class TeamNews;

private:
//    member functions

//    data members
      GenList<Trade>       tradeList;
      TradeNews            *tn;
      GenList<RosPlayer>   *pList;

public:
               TNTrade (DYN_16 tId);
               ~TNTrade ();

};

// Last Game
class TNGame : public TNBase
{
      friend class TeamNews;

private:
//    member functions
      DYN_16   Size () { return buffer [0] != 0; }

//    data members
      // team_1 xxx, team_2 xxx (OT)
      enum { MAX_SCHED_STRING_LEN = 64 };
      DYN_8    buffer [MAX_SCHED_STRING_LEN];

public:
               TNGame (DYN_16 tId);
               ~TNGame ();
};

// Career Ending Injury
class TNCEI : public TNBase
{
      friend class TeamNews;

private:
//    member functions

//    data members
      GenList<RosPlayer>   pList;

public:
      TNCEI (const DYN_16 tId, FILE *fp);
      ~TNCEI ();
};

// Retired players
class TNRetired : public TNBase
{
      friend class TeamNews;

private:
//    member functions

//    data members
      GenList<RosPlayer>   pList;

public:
      TNRetired (const DYN_16 tId, FILE *fp);
      ~TNRetired ();
};

struct CEIDiskData
{
   DYN_8    tId;
   DYN_16   pId;
};

// CEI/Retired players id saver
class TNPSaver : public TNBase
{
      friend class TeamNews;

private:
//    member functions

//    data members
      GenList<CEIDiskData> idList;

public:
            TNPSaver ();
            ~TNPSaver ();

      void  Add (const DYN_16 tId, const DYN_16 pId);
      void  Save (FILE *fp);
};


// the high-level organizer
class TeamNews
{

      enum NewsItem
      {
            TNI_TRADES,
            TNI_GAME,
            TNI_CEI,
            TNI_RETIRED,
            TNI_MAX,
            TNI_PSAVER = TNI_TRADES,	// must be equal to the first enum!
      };

private:
//    member functions

//    data members


protected:
//    member functions
      TNBase               *GetTNItem (const NewsItem ni);
      Trade                *GetTrade (TNTrade *tntPtr, const DYN_16 index);
      DYN_16               GetGame (TNGame *tngPtr, DYN_8 *ptr, const DYN_16 index);
      RosPlayer            *GetCEI (const DYN_16 index);
      RosPlayer            *GetRetired (const DYN_16 index);

//    data members
      GenList<TNBase>      news;

public:
//    member functions
                           TeamNews (DYN_8 *, DYN_8 *, DYN_16);
                           ~TeamNews ();

      static void          Flush (const DYN_8 *pName, const DYN_8 *fName);
      GenList<RosPlayer>   *GetPList ();
      GenList<Trade>       *GetTradeList ();
      Trade                *GetTrade (const DYN_16 index);

      DYN_8                *FillString (DYN_8 *fillPtr, DYN_16 index);

      DYN_16               Size ();

      void                 AddNews (const DYN_16 tId, const DYN_16 pId);
      void                 AddRetire (const DYN_16 tId, const DYN_16 pId);
      void                 Save (const DYN_8 *pName, const DYN_8 *fName);

//    data members
};

#endif      // _TEAMNEWS_H
