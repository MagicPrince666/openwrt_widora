#ifndef _MD5_H__
#define _MD5_H__

#include <inttypes.h>
/* MD5.H - header file for MD5C.C */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
   rights reserved.

   License to copy and use this software is granted provided that it
   is identified as the "RSA Data Security, Inc. MD5 Message-Digest
   Algorithm" in all material mentioning or referencing this software
   or this function.

   License is also granted to make and use derivative works provided
   that such works are identified as "derived from the RSA Data
   Security, Inc. MD5 Message-Digest Algorithm" in all material
   mentioning or referencing the derived work.

   RSA Data Security, Inc. makes no representations concerning either
   the merchantability of this software or the suitability of this
   software for any particular purpose. It is provided "as is"
   without express or implied warranty of any kind.

   These notices must be retained in any copies of any part of this
   documentation and/or software.

   2007-09-15 Last modified by cheungmine.
 */

/* POINTER defines a generic pointer type */  
typedef unsigned char * POINTER;  
  
/* UINT2 defines a two byte word */  
//typedef unsigned short int UINT2;   
  
/* UINT4 defines a four byte word */  
typedef uint32_t UINT4;  
  
  
/* MD5 context. */  
typedef struct {  
    UINT4 state[4];                                   /* state (ABCD) */  
    UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */  
    unsigned char buffer[64];                         /* input buffer */  
} MY_MD5_CTX;  
  
void MD5Init (MY_MD5_CTX *context);  
void MD5Update (MY_MD5_CTX *context, unsigned char *input, unsigned int inputLen);  
void MD5UpdaterString(MY_MD5_CTX *context,const char *string);  
int MD5FileUpdateFile (MY_MD5_CTX *context,char *filename);  
void MD5Final (unsigned char digest[16], MY_MD5_CTX *context);  
void MDString (char *string,unsigned char digest[16]);  
int MD5File (char *filename,unsigned char digest[16]);  

#endif    /* _MD5_H__ */

