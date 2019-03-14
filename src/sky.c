#include "sky.h"

void get_bip44_path(uint8_t *dataBuffer, unsigned int bip44_path[]) {
    for (uint32_t i = 0; i < BIP44_PATH_LEN; i++) {
        bip44_path[i] = U4BE(dataBuffer, 0);
        dataBuffer += 4;
    }
}

int encode_base_58(const unsigned char *pbegin, int len, char *result) {
    const unsigned char *pend = pbegin + len;
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    unsigned char b58[ADDRESS_BASE58_LEN];
    for (int i = 0; i < 35; i++) {
        b58[i] = 0;
    }
    int size = (pend - pbegin) * 138 / 100 + 1;
    // Found all remainders
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (int j = size - 1; (carry != 0 || i < length) && (j != -1); j--, i++) {
            carry += 256 * b58[j];
            b58[j] = carry % 58;
            carry /= 58;
        }

        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    int j = size - length;
    while (j != size && b58[j] == 0)
        j++;
    // Translate the result into a string.
    int i = 0;
    while (i < zeroes)
        result[i++] = '1';
    while (j != size)
        result[i++] = BASE_58_ALPHABET[b58[j++]];
    result[i] = '\0';
}

void to_address(const unsigned char *public_key_compressed, char *result) {
    /*
     * Address format 1+20=21 bytes:
     *  Version byte + 20 byte RIPMD160(SHA256(SHA256(compressed public key)))
     *
     * Base 58 address format 20+1+4 bytes:
     *	20 bytes RIPMD(...) + Version byte + 4 checksum bytes ( 4 bytes of SHA256 of the previous 21 bytes)
     *
     */
    static cx_sha256_t address_hash;
    static cx_ripemd160_t address_rip;

    unsigned char address_hash_result_0[SHA256_HASH_LEN];
    unsigned char address_hash_result_1[SHA256_HASH_LEN];

    // do a sha256 hash of the public key twice.
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, public_key_compressed, COMPRESSED_PK_LEN, address_hash_result_0);
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, address_hash_result_0, SHA256_HASH_LEN, address_hash_result_1);

    // do a RIPMD160 to sha256(sha256(public key))
    unsigned char address_ripmd_hash[RIPMD_HASH_LEN];
    cx_ripemd160_init(&address_rip);
    cx_hash(&address_rip.header, CX_LAST, address_hash_result_1, SHA256_HASH_LEN, address_ripmd_hash);

    // add version to address
    os_memmove(result, address_ripmd_hash, RIPMD_HASH_LEN);
    result[20] = ADDRESS_VERSION;


    // add checksum to address
    unsigned char checksum[SHA256_HASH_LEN];
    cx_sha256_init(&address_hash);
    cx_hash(&address_hash.header, CX_LAST, result, 21, checksum);
    os_memmove(result + 21, checksum, CHECKSUM_LEN);

    //encode address to base58
    encode_base_58(result, ADDRESS_LEN, result);
}

void derive_keypair(unsigned int bip44_path[], cx_ecfp_private_key_t *private_key, cx_ecfp_public_key_t *public_key) {
    unsigned char private_key_data[32];
    cx_ecfp_private_key_t pk;

    os_perso_derive_node_bip32(CX_CURVE_256K1, bip44_path, BIP44_PATH_LEN, private_key_data, NULL);
    cx_ecfp_init_private_key(CX_CURVE_256K1, private_key_data, 32, &pk);

    if (public_key) {
        // generate the public key.
        cx_ecfp_init_public_key(CX_CURVE_256K1, NULL, 0, public_key);
        cx_ecfp_generate_pair(CX_CURVE_256K1, public_key, &pk, 1);
    }

    if (private_key) {
        *private_key = pk;
    }

    os_memset(private_key_data, 0, sizeof(private_key_data));
    os_memset(&pk, 0, sizeof(pk));
}

void compress_public_key(const unsigned char *public_key, unsigned char *dst) {
    // check parity and add appropriate prefix
    dst[0] = ((public_key[64] & 1) ? 0x03 : 0x02);
    os_memmove(dst + 1, public_key + 1, 32);
}

void generate_address(const unsigned char *public_key, unsigned char *dst) {
    // convert public key from uncompressed to compressed
    unsigned char public_key_compressed[COMPRESSED_PK_LEN];
    compress_public_key(public_key, public_key_compressed);

    to_address(public_key_compressed, address_base58);
}


void convert_signature_from_TLV_to_RS(const unsigned char *tlv_signature, unsigned char *dst) {
    /**
     * Ledger SKD(e.g. function `cx_ecdsa_sign`) return a signature in the TLV format,
     * but Skycoin works with simple (R|S + recovery byte) format
     * This function convert TLV to skycoin format 
     * 
     * TLV specification: 
     *   Short:
     *      type | length | x02 identifier of integer | R length | R value (32-33 bytes) | x02 identifier of integer | S length | S value (32-33 bytes)
     *   Details:
     *      1-byte type 0x30 "Compound object" (the tuple of (R,S) values)
     *      1-byte length of the compound object
     *      The signature's R value, consisting of:
     *      1-byte type 0x02 "Integer"
     *      1-byte length of the integer
     *      variable-length R value's bytes
     *      The signature's S value, consisting of:
     *      1-byte type 0x02 "Integer"
     *      1-byte length of the integer
     *      variable-length S value's bytes
     *      The sighash type byte
     * 
     *  @param [in] tlv_signature
     *    The signature which returns ledger SDK (70-72 bytes length).
     * 
     *  @param [out] dst
     *    The resulting signature which is appropriate with Skycoin cipher API
     * */

    int r_size = tlv_signature[3];
    int s_size = tlv_signature[3 + r_size + 2];

    int r_offset = r_size - 32;
    int s_offset = s_size - 32;

    os_memmove(dst, tlv_signature + 4 + r_offset, 32); // skip first bytes and store the `R` part
    os_memmove(dst + 32, tlv_signature + 4 + 32 + 2 + r_offset + s_offset,
               32); // skip unused bytes and store the `S` part
}


void sign(cx_ecfp_private_key_t *private_key, const unsigned char *hash, unsigned char *signature) {
    /**
     *  First we need to sign message and get R ans S, for our signature
     *  As one pair, R and S, can produce different public keys, we need to add
     *  recovery id at the end of our signature.
     *
     *  To find recovery bit you need to check parity of Y when
     *  computing k.G. We can get it using info from cx_ecdsa_sign
     */

    /** create the signature of a hash */
    unsigned char tle_sign_tx[70];

    unsigned int info = 0;
    unsigned int recovery_id = 0;
    int sign_size;

    sign_size = cx_ecdsa_sign((void *) private_key, CX_RND_RFC6979 | CX_LAST, CX_SHA256, hash, 32, tle_sign_tx, &info);

    if (info & CX_ECCINFO_PARITY_ODD)
        recovery_id++;
    
    if (info & CX_ECCINFO_xGTn)
        recovery_id += 2;

    convert_signature_from_TLV_to_RS(tle_sign_tx, signature);

    signature[64] = recovery_id;
}