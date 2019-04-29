#ifndef NEW_CX
#define NEW_CX value
#include <stddef.h>
enum cx_md_e {
    /** NONE Digest */
    CX_NONE,
    /** RIPEMD160 Digest */
    CX_RIPEMD160, // 20 bytes
    /** SHA224 Digest */
    CX_SHA224, // 28 bytes
    /** SHA256 Digest */
    CX_SHA256, // 32 bytes
    /** SHA384 Digest */
    CX_SHA384, // 48 bytes
    /** SHA512 Digest */
    CX_SHA512, // 64 bytes
    /** KECCAK (pre-SHA3) Digest */
    CX_KECCAK, // 28,32,48,64 bytes
    /** SHA3 Digest */
    CX_SHA3, // 28,32,48,64 bytes
    /** SHA3-XOF  Digest */
    CX_SHA3_XOF, // any bytes
    /** */
    CX_GROESTL,
    /** */
    CX_BLAKE2B,
};

typedef enum cx_md_e cx_md_t;

struct cx_hash_header_s {
    /** Message digest identifier, See cx_md_e. */
    cx_md_t algo;
    /** Number of block already processed */
    unsigned int counter;
};

typedef struct cx_hash_header_s cx_hash_t;

#ifndef PLENGTH
#define PLENGTH(len) /**/
#endif

#define WIDE // const // don't !!
#define WIDE_AS_INT unsigned long int
#define REENTRANT(x) x //

void cx_hash(cx_hash_t *hash PLENGTH(scc__cx_scc_struct_size_hash__hash),
                    int mode, const unsigned char WIDE *in PLENGTH(len),
                    unsigned int len, unsigned char *out PLENGTH(out_len)) {

}
//void cx_hash(cx_hash_t *hash PLENGTH(scc__cx_scc_struct_size_hash__hash),
//                    int mode, const unsigned char WIDE *in PLENGTH(len),
//                    unsigned int len) {

//}
struct cx_sha256_s {
    /** @copydoc cx_ripemd160_s::header */
    struct cx_hash_header_s header;
    /** @internal @copydoc cx_ripemd160_s::blen */
    int blen;
    /** @internal @copydoc cx_ripemd160_s::block */
    unsigned char block[64];
    /** @copydoc cx_ripemd160_s::acc */
    unsigned char acc[8 * 4];
};

/** Convenience type. See #cx_sha256_s. */
typedef struct cx_sha256_s cx_sha256_t;
typedef struct cx_sha256_s cx_ripemd160_t;
typedef int cx_ecfp_public_key_t;

typedef int cx_ecfp_private_key_t;

#define CX_LAST 0
#define CX_CURVE_256K1 0
#define CX_RND_RFC6979 0
#define CX_ECCINFO_PARITY_ODD 0
#define CX_ECCINFO_xGTn 0
#endif /* ifndef NEW_CX */

