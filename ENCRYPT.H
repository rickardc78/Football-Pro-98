/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   encrypt.h

   the Encryption class

   a transformation array is generated so that every value of 8 bits,
   is run through 3 times, generating a final value:

   [256 byte array of 'indices' (each value appearing only once)],
   so if the array were filled uncreatively with 0 thru 0xff,
   and we were trying to encode the value 0,
   look up array [array [array [0]]], or 0, then 0 and finally 0

   if the array were filled somewhat randomly the values may be
   16, 138, 27, or EVERY occurance of 0 would be replaced by 27.
   However, since the array is filled randomly, 1 and 2 would not
   be replaced by 28 and 29, but two other random numbers.

   Also, if the two numbers used to create the data array are known
   (i.e. saved in the file somewhere), the process is reversed, that
   is, 27 would be found in the array, yielding the index 138, which
   would be searched for, yielding the index 16, which would be searched
   for yielding the index 0, or the original value

   GJW:	2-24-1994	original creation
   GJW:	9-24-1994	ported to football
	MMQ	14-Feb-97	Added ENCRYPT_ACCESS to class definition.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _ENCRYPT_H
#define  _ENCRYPT_H

#define ENCRYPT_ACCESS			PROCACCESS

class ENCRYPT_ACCESS Encryption
{
private:
   // member functions

   // data members


protected:
   // member functions

   // data members
   enum { SIZE = 256 };
   DYN_U_8     array [SIZE];

public:
//    member functions
               Encryption ();
               ~Encryption ();

      void     GetInit (DYN_U_8 &s, DYN_U_8 &i);
      void     Init (const DYN_U_8 s = 0, const DYN_U_8 i = 1);
      void     Encrypt (void FAR *ptr, DYN_16 size);
      void     Decrypt (void FAR *ptr, DYN_16 size);

//    data members
};

#endif   // _ENCRYPT_H
