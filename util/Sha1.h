// WebWatch Util - by DEATH, 2004
//
// $Workfile: Sha1.h $ - Secure Hashing Algorithm 1 implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#ifndef SHA1_H
#define SHA1_H

#define SHA1_SIGNATURE_LENGTH 20

typedef struct SHA1_CTX {
    unsigned long state[5];
    unsigned long count[2];
    unsigned char buffer[64];
} SHA1_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, const void *data, unsigned int len);
void SHA1Final(unsigned char digest[SHA1_SIGNATURE_LENGTH], SHA1_CTX* context);

#ifdef __cplusplus
}
#endif

#endif
