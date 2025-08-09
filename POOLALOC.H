/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   poolaloc.h

   the PoolAlloc class

   allocate a big block of mem, then dish it out in pieces
   (convert all of the memory to a linked-list of allocation units)
   e.g. allocate 4 units, of 128 bytes each,
   4 * 128 is allocated, then the first 4 bytes of each unit, are set
   to point to each other.
   +--------------+  +--------------+  +--------------+  +--------------+
   |              |->|              |->|              |->|              |->0
   |              |  |              |  |              |  |              |
   |              |  |              |  |              |  |              |
   +--------------+  +--------------+  +--------------+  +--------------+

   calls to New (), return the head pointer, and a new head pointer is
   obtained by following the list
   calls to Delete (), install the incomming memory as the head, and
   set it's list pointer to the old head

   GJW: 9-13-1993, original creation
   GJW: 9-13-1994, ported over to medium model for football
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _POOLALOC_H
#define  _POOLALOC_H

#include <iostream.h>

class PoolAlloc
{
private:
//    member functions

//    data members

protected:
//    member functions
                     // the default (no argument) constructor is NOT
                     // accessible by the public
                     // a user is expected to provide a memory amount
                     // when instantiating this type of object
                     PoolAlloc ();

      void           Link ();

#if   DEBUG_MEM
      void           CheckList (const DYN_8 *str);
#endif

      virtual void   Alloc ();

//    data members

      DYN_8          *block,
                     *head;
	DYN_U_16       classSize,
                     numNodes,
                     numAlloc;

public:
      union MemBlock
      {
         union  MemBlock   *nextBlock;
         DYN_8             dataBlock [1];
      };

//    member functions
//      friend ostream&   operator<< (ostream &s, const PoolAlloc &t);

                     PoolAlloc (const DYN_U_16 cSize, const DYN_U_16 nNodes);
      virtual        ~PoolAlloc ();

      virtual void   *New ();
      virtual void   Delete (const void *pPtr);
      virtual DYN_16 ReAlloc (const DYN_U_16 nNodes);
// JDW - added next two lines to simplify debugging
	  friend void FAR *RosPlayer::operator new (size_t size);
	  friend void RosPlayer::operator delete (void FAR *t, size_t size);

//    data members (hopefully none!)
};
#endif   // _POOLALOC_H
