/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.


   dyndefs.h

   Dynamix global typedefs for easy access across comilers

   GJW: 6-2-1992, original creation
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

#ifndef  _DYNDEFS_H
#define  _DYNDEFS_H

#ifndef _WINDOWS_
// GJW: 2-15-1995, far is invalid in 32 bit, so make it a define
#ifdef   __FLAT__
#define NEAR
#define FAR
#define HUGE
#else
#define NEAR   near
#define FAR    far
#define HUGE   huge
787878
#endif
#endif	// #ifndef _WINDOWS_

// storage identifiers
typedef char               DYN_8;
typedef short              DYN_16;
typedef long               DYN_32;
typedef unsigned char      DYN_U_8;
typedef unsigned short     DYN_U_16;
typedef unsigned long      DYN_U_32;

// function helpers:
// first block is the return type
// second are the parameters
// C = char, F = far, S = short, P = pointer modifier (i.e. SP = *short),
// V = void

typedef DYN_8  *(*CP_SS_FUNC) (DYN_16, DYN_16);
typedef DYN_8  (*C_V_FUNC) (void);
typedef DYN_16 (*S_CPS_FUNC) (DYN_8 *, DYN_16);
typedef DYN_16 (*S_USPS_FUNC) (DYN_U_16 *, DYN_16);
typedef DYN_16 (*S_VPVPS_FUNC) (void *, void *, DYN_16);
typedef DYN_16 (*S_VFPVFPS_FUNC) (void FAR *, void FAR *, DYN_16);
typedef void   (*V_S_FUNC) (DYN_16);
typedef void   (*V_SCP_FUNC) (DYN_16, DYN_8 *);
typedef void   (*V_SCPCP_FUNC) (DYN_16, DYN_8 *, DYN_8 *);
typedef void   (*V_SSCPCP_FUNC) (DYN_16, DYN_16, DYN_8 *, DYN_8 *);
typedef void   (*V_V_FUNC) (void);
typedef void   (*V_VP_FUNC) (void *);
typedef void   (*V_VFP_FUNC) (void FAR *);
typedef void   (*V_CS_FUNC) (const DYN_16);

typedef void   NEAR (*V_V_NFUNC) (void);
typedef DYN_16 NEAR (*S_USPS_NFUNC) (DYN_U_16 *, DYN_16);

#ifdef  TRUE
#undef  TRUE 
#undef  FALSE 
#endif
#define TRUE  1
#define FALSE 0

#define BOTTOM_HALF_SCREEN_ONLY 0x0001

#endif   // _DYNDEFS_H
