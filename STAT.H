/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
   Copyright 1991-1997 Sierra On-Line.  All Rights Reserved.

   This code is copyrighted and intended as an aid in writing utilities
   for the Front Page Sports Football products.  All rights reserved.

   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
   PARTICULAR PURPOSE.
컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 */

/*  stat.h

    Definitions used for file status functions
*/

#if !defined(__STAT_H)
#define __STAT_H

// GJW 8-24-1995, Yikes! A hack to include borland in watcom
#if defined(__MT__)
#    define _MT
#endif

#if defined(__OS2__)
#    define _RTLENTRY  __stdcall
#    define _USERENTRY __stdcall
#elif defined(__WIN32__)
#    define _RTLENTRY  __cdecl
#    define _USERENTRY __cdecl
#else
#    define _RTLENTRY  __cdecl
#    define _USERENTRY __cdecl
#endif

#if defined(__PAS__)
#    define _RTLENTRYF __pascal
#else
#    define _RTLENTRYF _RTLENTRY
#endif

#if defined(__FLAT__)
#   define _FAR
#   if defined(_BUILDRTLDLL)
#       define _EXPCLASS  __export
#       define _EXPDATA   __export
#       define _EXPFUNC   __export
#   elif defined(_RTLDLL) && !defined(__OS2__)
#       define _EXPCLASS  __import
#       define _EXPDATA   __import
#       define _EXPFUNC   __import
#   else
#       define _EXPCLASS
#       define _EXPDATA
#       define _EXPFUNC
#   endif
#else
#   if defined(__DLL__)
#       if defined(_BUILDRTLDLL)
#           define _EXPCLASS __export
#       elif defined(_RTLDLL) || defined(_CLASSDLL)
#           define _EXPCLASS __export
#       else
#           define _EXPCLASS __far
#       endif
#       define _FAR __far
#   elif defined(_RTLDLL) || defined(_CLASSDLL)
#       define _EXPCLASS __export
#       define _FAR __far
#   else
#       if   defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)
#           if defined(_RTLFARVTABLE)
#               define _EXPCLASS  __huge
#           else
#               define _EXPCLASS  __near
#           endif
#       elif defined(__COMPACT__) || defined(__LARGE__)
#           if defined(_RTLFARVTABLE)
#               define _EXPCLASS  __huge
#           else
#               define _EXPCLASS  __far
#           endif
#       else
#           define _EXPCLASS  __huge
#       endif
#       define _FAR
#   endif
#   if defined(_BUILDRTLDLL)
#       define _EXPFUNC __export
#   else
#       if defined(_RTLDLL) || defined(_CLASSDLL)
#           define _EXPFUNC __far
#       else
#           define _EXPFUNC
#       endif
#   endif
#   define _EXPDATA
#endif


#if defined(__FLAT__)
#define _EXPFUNC32  _EXPFUNC
#define _EXPFUNC16
#else
#define _EXPFUNC32
#define _EXPFUNC16  _EXPFUNC
#endif


#define _CType      _RTLENTRYF
#define _CLASSTYPE  _EXPCLASS
#define _FARFUNC    _EXPFUNC
#define _FARCALL    _EXPFUNC __far

#if defined(__FLAT__) && !defined(_MSC_VER)
#  define _M_IX86 300
#endif

#if !defined(__FLAT__) && defined(__MSC)
#   define __emit db
#   ifdef __SMALL__
#       define _M_I86SM
#   endif
#   ifdef __COMPACT__
#       define _M_I86CM
#   endif
#   ifdef __MEDIUM__
#       define _M_I86MM
#   endif
#   ifdef __LARGE__
#       define _M_I86LM
#   endif
#   ifndef _Windows
#       define _DOS
#   else
#       define _WINDOWS
#   endif
#endif

#if defined(__cplusplus)
#   define _PTRDEF(name) typedef name _FAR * P##name;
#   define _REFDEF(name) typedef name _FAR & R##name;
#   define _REFPTRDEF(name) typedef name _FAR * _FAR & RP##name;
#   define _PTRCONSTDEF(name) typedef const name _FAR * PC##name;
#   define _REFCONSTDEF(name) typedef const name _FAR & RC##name;

/*********** Obsolete definitions for OWL 1.0 *************/
#   define _CLASSDEF(name) class _EXPCLASS name; \
           _PTRDEF(name) \
           _REFDEF(name) \
           _REFPTRDEF(name) \
           _PTRCONSTDEF(name) \
           _REFCONSTDEF(name)
#endif

#ifndef  _TIME_T
#define  _TIME_T
typedef long time_t;
#endif

#if defined(__FLAT__)
typedef short dev_t;
typedef short ino_t;
typedef short mode_t;
typedef short nlink_t;
typedef int   uid_t;
typedef int   gid_t;
typedef long  off_t;

#endif  /* __FLAT__ */

/* Traditional names for bits in st_mode.
 */
#define S_IFMT  0xF000  /* file type mask */
#define S_IFDIR 0x4000  /* directory */
#define S_IFIFO 0x1000  /* FIFO special */
#define S_IFCHR 0x2000  /* character special */
#define S_IFBLK 0x3000  /* block special */
#define S_IFREG 0x8000  /* or just 0x0000, regular */
#define S_IREAD 0x0100  /* owner may read */
#define S_IWRITE 0x0080 /* owner may write */
#define S_IEXEC 0x0040  /* owner may execute <directory search> */

#if defined(__FLAT__)
/* POSIX file type test macros.  The parameter is an st_mode value.
 */
#define S_ISDIR(m)  ((m) & S_IFDIR)
#define S_ISCHR(m)  ((m) & S_IFCHR)
#define S_ISBLK(m)  ((m) & S_IFBLK)
#define S_ISREG(m)  ((m) & S_IFREG)
#define S_ISFIFO(m) ((m) & S_IFIFO)

/* POSIX names for bits in st_mode.
 */
#define S_IRWXU  0x01c0 /* RWE permissions mask for owner */
#define S_IRUSR  0x0100 /* owner may read */
#define S_IWUSR  0x0080 /* owner may write */
#define S_IXUSR  0x0040 /* owner may execute <directory search> */

#if !defined(_RC_INVOKED)
//#pragma pack(1)
#endif

struct  stat
{
    dev_t   st_dev;
    ino_t   st_ino;
    mode_t  st_mode;
    nlink_t st_nlink;
    uid_t   st_uid;
    gid_t   st_gid;
    dev_t   st_rdev;
    off_t   st_size;
    time_t  st_atime;
    time_t  st_mtime;
    time_t  st_ctime;
};

#if !defined(_RC_INVOKED)
//#pragma pack()
#endif

#else
struct  stat
{
    short st_dev;
    short st_ino;
    short st_mode;
    short st_nlink;
    int   st_uid;
    int   st_gid;
    short st_rdev;
    long  st_size;
    long  st_atime;
    long  st_mtime;
    long  st_ctime;
};
#endif  /* __FLAT__ */

#ifdef __cplusplus
extern "C" {
#endif
int  _RTLENTRY _EXPFUNC fstat(int __handle, struct stat _FAR *__statbuf);
int  _RTLENTRY _EXPFUNC stat(const char _FAR *__path, struct stat _FAR *__statbuf);

#ifdef __MSC
#define _fstat(h,b) fstat(h,(struct stat *)b)
#define _stat(p,b)   stat(p,(struct stat *)b)
struct  _stat
{
    short st_dev;
    short st_ino;
    short st_mode;
    short st_nlink;
    int   st_uid;
    int   st_gid;
    short st_rdev;
    long  st_size;
    long  st_atime;
    long  st_mtime;
    long  st_ctime;
};
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __STAT_H */

