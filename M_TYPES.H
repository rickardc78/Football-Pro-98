/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef _M_TYPES_H_
#define _M_TYPES_H_

// GJW: 3-31-1994
// true if linking bug occurs
#define  BOR_BUG  0


//---------------------------------------------------------------------------

//	$Workfile:   m_types.h  $
//	$Revision:   1.0  $
// $Version: 1.6.0 $
//	$Date:   02 Oct 1995 14:37:04  $
// $Log:   G:\fball96\code\glenw\fb_menu\dyn_inc\m_types.h_v  $
//	
//	   Rev 1.0   02 Oct 1995 14:37:04   GLENW
//	Initial revision.
//	
//	   Rev 1.0   02 Oct 1995 14:09:50   GLENW
//	Initial revision.
//	
//	   Rev 1.12   02 Dec 1993 14:58:48   NANCYH
//	Release version #1.6.0 for ml Library
//	
//	   Rev 1.6   19 Oct 1993 15:44:20   RICKO
//	If defined TRUE/FALSE/YES/NO/ON/OFF undefine berore defining
//	
//	   Rev 1.5   09 Aug 1993 12:20:26   CAEDMONI
//	Moved all remaining type definitions from m_base.h to m_types.h.
//	
//	   Rev 1.4   20 Jul 1993 11:03:10   TIMG
//	Added ON/OFF
//	
//	   Rev 1.3   19 Jul 1993 20:54:06   TIMG
//	Added TRUE/FALSE YES/NO
//	
//	   Rev 1.2   04 Feb 1993 13:36:32   TIMG
//	Converted min/max defines to constants and corrected values.
//	
//	   Rev 1.1   04 Feb 1993 13:12:44   TIMG
//	Added min/max for types.
//	
//	   Rev 1.0   10 Dec 1992 15:02:28   TIMG
//	Initial revision.
//	

//---------------------------------------------------------------------------

// Basic types are:  typename[size][_modifier]
//                   Where size is the size of the item in bits.
//                   For fixed point items, the size represents the
//                   number of bits for the fraction, the actual size
//                   of the variable is always Int16.

//  Function Modifiers:
//    							f     	Fast
//    							vf    	Very fast.

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Types and classes.
//---------------------------------------------------------------------------

#ifdef  TRUE
# undef  TRUE 
# undef  FALSE 
#endif
#define TRUE  1
#define FALSE 0

#ifdef  YES
# undef  YES
# undef  NO
#endif
#define YES   TRUE
#define NO    FALSE

#ifdef  ON
# undef  ON
# undef  OFF
#endif
#define ON    TRUE
#define OFF   FALSE

typedef unsigned char   Bool;

typedef  char  			int8,   Int8,   INT8,   byte,   Byte,   BYTE;
typedef  short 			int16,  Int16,  INT16,	word,   Word,   WORD;
typedef  long  			int32,  Int32,  INT32,  dword,  DWord,  DWORD;

typedef struct
{
   Int32    lo, hi;
} Int64;


typedef  unsigned char  	uint8,  UInt8,  UINT8,  ubyte,  UByte,  UBYTE;
typedef  unsigned short 	uint16, UInt16, UINT16, uword,  UWord,  UWORD;
typedef  unsigned long  	uint32, UInt32, UINT32, udword, UDWord, UDWORD;

typedef UInt16	Angle;

typedef struct
{
	Int16		quot, rem;
} div_t16;

typedef struct
{
	Int32		quot, rem;
} div_t32;


//---------------------------------------------------------------------------
// Fixed point types.

typedef Int16 Fix8;					// 8.8	=   +- 127.256
typedef Int16 Fix10;					// 6.10	=    +- 31.1024
typedef Int16 Fix14;					// 2.14	=     +- 1.16284
typedef Int32 Fix16;					// 16.16 = +- 32767.65535


const Int16		trig_shift = 14;

const int		M_MIN_INT =		1 << ((sizeof(int) << 3) - 1);
const int		M_MAX_INT =		~M_MIN_INT;

const Int8		M_MAX_INT8 =	0x7f;
const Int8		M_MIN_INT8 =	0x80;
const Int16		M_MAX_INT16 =	0x7fff;
const Int16		M_MIN_INT16	=	0x8000;
const Int32		M_MAX_INT32	=	0x7fffffff;
const Int32		M_MIN_INT32	=	0x80000000;

const UInt8		M_MAX_UINT8	=	0xff;
const UInt16	M_MAX_UINT16 =	0xffff;
const UInt32	M_MAX_UINT32 =	0xffffffff;

#endif
