/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

// -----------------------------------------------------------------------
//
//	 TeamSel.h
//		Select a new team header.
//
//
//		CAM	16 JUL 96	First Incarnation
//
// -----------------------------------------------------------------------


#ifndef _TEAMSEL_H_
#define _TEAMSEL_H_

 // -- Return screen procdure
typedef void (*FP_RETURNPROC)( void );

 // -- Prototypes
void initTeamSel	( FP_RETURNPROC fpRetFunc, DYN_16 *pTeamId );

#endif

// TeamSel.h
