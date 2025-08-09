/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   gameplan.h

   the GamePlan class

   GJW: 3-20-1992, original creation
   GJW: 7-20-1993, moved data to FAR memory
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _GAMEPLAN_H
#define  _GAMEPLAN_H

//#include    <dir.h>
#define GAMEPLANSIZE ((MAXPATH + 1) + (MAXFILE + MAXEXT + 1))

class GamePlan {
private:
//    member functions

//    data members


protected:
//    member functions
      DYN_8    FAR *path,
               FAR *fileName;

//    data members


public:
//    member functions
               GamePlan ();
               ~GamePlan ();

      static DYN_8   *GetDefPlan ();
      static DYN_8   *GetDefDefPlan ();

      DYN_8          FAR *GetPath () const;
      void           SetPath (const DYN_8 *ptr);
      DYN_8          FAR *GetFName () const;
      void           SetFName (const DYN_8 *ptr);

//    data members
};

#endif      // _GAMEPLAN_H
