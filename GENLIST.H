/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   genlist.h

   the general List template class

   GJW: 1-12-1993, made template from Zortech tools
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _GENLIST
#define  _GENLIST

#include    "dlist.h"

template <class T>
class GenList : public zDList
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
   GenList ()                             { }
   GenList (DYN_16 n)                     { zDList::init (n, 0, 0); }
   GenList (DYN_16 n, DYN_16 p = 0)       { zDList::init (n, 0, p); }
   GenList (DYN_16 n, T *a, DYN_16 p = 0) { zDList::init (n, a, p); }

   DYN_16   linkin (T *t)           { return zDList::linkin (t); }

   DYN_16   fwd ()                  { return zDList::fwd (); }
   DYN_16   bkwd ()                 { return zDList::bkwd (); }
   DYN_16   operator+= (DYN_16 n)   { DYN_16 r; while (n--) r = fwd (); return r; }
   DYN_16   operator-= (DYN_16 n)   { DYN_16 r; while (n--) r = bkwd (); return r; }
   DYN_16   operator++ ()           { return fwd (); }
   DYN_16   operator-- ()           { return bkwd (); }

   T        *update (T *t)          { return (T *) zDList::update (t); }

   T        *get ()                 { return (T *) zDList::get (); }
   T        *linkout ()             { return (T *) zDList::linkout (); }
   T        *operator* ()           { return (T *) linkout (); }
   T        *operator() ()          { return (T *) get (); }

   DYN_16   start ()                { return zDList::start (); }
   DYN_16   end ()                  { return zDList::end (); }
   DYN_16   size () const           { return zDList::size (); }
   DYN_16   error ()                { return zDList::error (); }

//    data members
};

template <class T>
class GenCursor : public zDLCursor
{
private:
//    member functions

//    data members


protected:
//    member functions

//    data members


public:
//    member functions
   GenCursor (GenList<T> *l, DYN_16 s = 0)   { zDLCursor::init (l, s); }

   DYN_16   insync ()               { return zDLCursor::insync (); }

   DYN_16   fwd ()                  { return zDLCursor::fwd (); }
   DYN_16   bkwd ()                 { return zDLCursor::bkwd (); }
   DYN_16   operator+= (DYN_16 n)   { DYN_16 r; while (n--) r = fwd (); return r; }
   DYN_16   operator-= (DYN_16 n)   { DYN_16 r; while (n--) r = bkwd (); return r; }
   DYN_16   operator++ ()           { return fwd (); }
   DYN_16   operator-- ()           { return bkwd (); }

   T        *get ()                 { return (T *) zDLCursor::get (); }
   T        *update (T *t)          { return (T *) zDLCursor::update (t); }
   T        *operator() ()          { return (T *) get (); }

   DYN_16   start ()                { return zDLCursor::start (); }
   DYN_16   end ()                  { return zDLCursor::end (); }
   DYN_16   error ()                { return zDLCursor::error (); }

//    data members
};
#endif   // _GENLIST

