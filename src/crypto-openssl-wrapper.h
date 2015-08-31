#ifndef _GCRYPT_OPENSSL_WRAPPER_H
#define _GCRYPT_OPENSSL_WRAPPER_H
/*
 *
 * gcrypt-openssl-wrapper.h
 *
 * Copyright (C) 2012 Carlos Alberto Lopez Perez <clopez@igalia.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
#include "rc4.h"
#include "aes.h"
#include "md5.h"
#include "sha1.h"
#include "hmac.h"

// RC4_*
#define RC4_KEY                                         rc4_key_t
#define RC4_set_key(h, l, k)                            rc4_set_key(k,l,h)

                                                        // we need to release the handle to avoid memory leaks.
                                                        // And in the actual code there are not repeat calls to RC4() without calling
                                                        // RC4_set_key() first, so we can encapsulate the call to gcry_cipher_close() inside RC4()
                                                        // This should be changed if you call RC4() without calling RC4_set_key before
#define RC4(h, l, s, d)                                 rc4_crypt(s,d,l,h)

// SHA_* (We use the sha1-git implementation because is much faster)
#define SHA_CTX                                         SHA1_CTX
#define SHA1_Init(ctx)                                  sha1_init(ctx)
#define SHA1_Update(ctx,buffer,len)                     sha1_update(ctx,buffer,len)
#define SHA1_Final(digest,ctx)                          sha1_final(ctx,digest)

// AES_*
/* Same
#define AES_KEY                                         
#define AES_encrypt(text, enc_out, ctx)                 
#define AES_set_encrypt_key(key, len, ctx)              
*/

// EVP_*
#define _MD_MD5    1
#define _MD_SHA1   2
#define EVP_md5()                                       _MD_MD5
#define EVP_sha1()                                      _MD_SHA1

// HMAC_*

#define HMAC(algo, key, klen, data, dlen, res, rlen)			\
  do {									\
    if (algo == _MD_MD5)						\
      hmac_md5(data,dlen,key,klen,res);					\
    else								\
      hmac_sha1(data,dlen,key,klen,res);				\
  } while(0)

/*
#define HMAC_CTX                                        gcry_md_hd_t
#define HMAC_CTX_cleanup(ctx)                           gcry_md_close(*ctx)
#define HMAC_CTX_init(ctx)                              ; // noop
#define HMAC_Init_ex(ctx, key, len, md, engine)         HMAC_Init(ctx, key, len, md)
#define HMAC_Init(ctx, key, len, md)                    do  { \
                                                            if ((len==0)||(key==NULL)||(md==0)) { \
                                                                gcry_md_reset(*ctx); \
                                                            } else { \
                                                                gcry_md_open(ctx, md, GCRY_MD_FLAG_HMAC); \
                                                                gcry_md_setkey(*ctx, key, len); \
                                                            } \
                                                        }  while (0)
#define HMAC_Update(ctx, data, len)                     gcry_md_write(*ctx, data, len)
#define HMAC_Final(ctx, md, len)                        do  { \
                                                            memcpy(   md,  \
                                                                gcry_md_read(*ctx, 0), \
                                                                gcry_md_get_algo_dlen(gcry_md_get_algo(*ctx)) \
                                                            ); \
                                                        } while (0)
*/

// http://tumblr.spantz.org/post/214737529/the-use-of-do-while-0-in-c-macros
#endif // _GCRYPT_OPENSSL_WRAPPER_H
