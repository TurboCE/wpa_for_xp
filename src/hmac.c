//
//  hmac.c
//  OAuthConsumer
//
//  Created by Jonathan Wight on 4/8/8.
//  Copyright 2008 Jonathan Wight. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

/*
 * Implementation of HMAC-SHA1. Adapted from example at http://tools.ietf.org/html/rfc2104
 
 */

#include "md5.h"
#include "sha1.h"

#include <stdlib.h>
#include <string.h>

// SHA_* (We use the sha1-git implementation because is much faster)
#define SHA_CTX                                         SHA1_CTX
#define SHA1_Init(ctx)                                  sha1_init(ctx)
#define SHA1_Update(ctx,buffer,len)                     sha1_update(ctx,buffer,len)
#define SHA1_Final(digest,ctx)                          sha1_final(ctx,digest)

#define MD5_Init(tctx)                                  md5_init(tctx)
#define MD5_Update(tctx, key, key_len)                  md5_update(tctx,key,key_len)
#define MD5_Final(tk, tctx)                             md5_final(tctx,tk)

void hmac_sha1(const unsigned char *inText, size_t inTextLength, unsigned char* inKey, size_t inKeyLength, unsigned char *outDigest)
{
  const size_t B = 64;
  const size_t L = 20;
  
  SHA1_CTX theSHA1Context;
  unsigned char k_ipad[B + 1]; /* inner padding - key XORd with ipad */
  unsigned char k_opad[B + 1]; /* outer padding - key XORd with opad */
  
  /* if key is longer than 64 bytes reset it to key=SHA1 (key) */
  if (inKeyLength > B)
    {
      SHA1_Init(&theSHA1Context);
      SHA1_Update(&theSHA1Context, inKey, inKeyLength);
      SHA1_Final(inKey, &theSHA1Context);
      inKeyLength = L;
    }
  
  /* start out by storing key in pads */
  memset(k_ipad, 0, sizeof k_ipad);
  memset(k_opad, 0, sizeof k_opad);
  memcpy(k_ipad, inKey, inKeyLength);
  memcpy(k_opad, inKey, inKeyLength);
  
  /* XOR key with ipad and opad values */
  int i;
  for (i = 0; i < B; i++)
    {
      k_ipad[i] ^= 0x36;
      k_opad[i] ^= 0x5c;
    }
  
  /*
   * perform inner SHA1
   */
  SHA1_Init(&theSHA1Context);                 /* init context for 1st pass */
  SHA1_Update(&theSHA1Context, k_ipad, B);     /* start with inner pad */
  SHA1_Update(&theSHA1Context, (unsigned char *)inText, inTextLength); /* then text of datagram */
  SHA1_Final((unsigned char *)outDigest, &theSHA1Context);                /* finish up 1st pass */
  
  /*
   * perform outer SHA1
   */
  SHA1_Init(&theSHA1Context);                   /* init context for 2nd
						* pass */
  SHA1_Update(&theSHA1Context, k_opad, B);     /* start with outer pad */
  SHA1_Update(&theSHA1Context, outDigest, L);     /* then results of 1st
						  * hash */
  SHA1_Final(outDigest, &theSHA1Context);          /* finish up 2nd pass */
  
}

/*
 * hmac.cFor the sake of illustration we provide the following
 *sample code for the implementation of HMAC-MD5 as well
 *as some corresponding test vectors (the code is based
 *on MD5 code as described in [MD5]).
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Copyright 2000  The FreeRADIUS server project
 */

/*
** Function: hmac_md5
*/

// reference : http://www.opensource.apple.com/source/freeradius/freeradius-11/freeradius/src/lib/hmac.c
void hmac_md5(const unsigned char *text, int text_len, const unsigned char *key, int key_len,unsigned char *digest)
{
  MD5_CTX context;
  unsigned char k_ipad[65];    /* inner padding -
				* key XORd with ipad
				*/
  unsigned char k_opad[65];    /* outer padding -
				* key XORd with opad
				*/
  unsigned char tk[16];
  int i;
  /* if key is longer than 64 bytes reset it to key=MD5(key) */
  if (key_len > 64) {

    MD5_CTX      tctx;

    MD5_Init(&tctx);
    MD5_Update(&tctx, key, key_len);
    MD5_Final(tk, &tctx);

    key = tk;
    key_len = 16;
  }

  /*
   * the HMAC_MD5 transform looks like:
   *
   * MD5(K XOR opad, MD5(K XOR ipad, text))
   *
   * where K is an n byte key
   * ipad is the byte 0x36 repeated 64 times

   * opad is the byte 0x5c repeated 64 times
   * and text is the data being protected
   */

  /* start out by storing key in pads */
  memset( k_ipad, 0, sizeof(k_ipad));
  memset( k_opad, 0, sizeof(k_opad));
  memcpy( k_ipad, key, key_len);
  memcpy( k_opad, key, key_len);

  /* XOR key with ipad and opad values */
  for (i = 0; i < 64; i++) {
    k_ipad[i] ^= 0x36;
    k_opad[i] ^= 0x5c;
  }
  /*
   * perform inner MD5
   */
  MD5_Init(&context);                   /* init context for 1st
					* pass */
  MD5_Update(&context, k_ipad, 64);      /* start with inner pad */
  MD5_Update(&context, text, text_len); /* then text of datagram */
  MD5_Final(digest, &context);          /* finish up 1st pass */
  /*
   * perform outer MD5
   */
  MD5_Init(&context);                   /* init context for 2nd
					* pass */
  MD5_Update(&context, k_opad, 64);     /* start with outer pad */
  MD5_Update(&context, digest, 16);     /* then results of 1st
					* hash */
  MD5_Final(digest, &context);          
}
