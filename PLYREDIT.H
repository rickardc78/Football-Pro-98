/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   plyredit.h

   the PlayerEdit class

   GJW: 10-6-1994, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _PLYREDIT_H
#define  _PLYREDIT_H

//#include <dir.h>
#include "league.h"

class PlayerEdit
{
private:
   // member functions

   // data members


protected:
   // member functions

   // data members
   DYN_8       lPath [MAXPATH + 1],
               lName [8 + 1],
               pbFlag,
               ratPath [MAXPATH + 1],
               ratName [8 + 1];

public:
   // member functions
                     PlayerEdit ();
                     ~PlayerEdit ();

      void           Load ();
      void           Save (League *lPtr, const DYN_8 pbf);

      DYN_16         GetPBFlag () const;
      const DYN_8    *GetRatPath () const;
      const DYN_8    *GetRatName () const;

   // data members
};

inline DYN_16
PlayerEdit::GetPBFlag () const
{
   return pbFlag;
}

inline const DYN_8 *
PlayerEdit::GetRatPath () const
{
   return ratPath;
}

inline const DYN_8 *
PlayerEdit::GetRatName () const
{
   return ratName;
}

#endif      // _PLYREDIT_H
