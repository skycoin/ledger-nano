#include "sky.h"

/** if true, show a screen with the transaction type. */
#define SHOW_TX_TYPE true

/** if true, show a screen with the transaction length. */
#define SHOW_TX_LEN false

/** if true, show a screen with the transaction version. */
#define SHOW_VERSION false

/** if true, show the tx-type exclusive data, such as coin claims for a Claim Tx */
#define SHOW_EXCLUSIVE_DATA false

/** if true, show number of attributes. */
#define SHOW_NUM_ATTRIBUTES false

/** if true, show number of tx-in coin references. */
#define SHOW_NUM_COIN_REFERENCES false

/** if true, show number of output transactions. */
#define SHOW_NUM_TX_OUTS false

/** if true, show tx-out values in hex as well as decimal. */
#define SHOW_VALUE_HEX false

/** if true, show script hash screen as well as address screen */
#define SHOW_SCRIPT_HASH false

/**
 * each CoinReference has two fields:
 *  UInt256 PrevHash = 32 bytes.
 *  ushort PrevIndex = 2 bytes.
 */
#define COIN_REFERENCES_LEN (32 + 2)

/** length of tx.output.value */
#define VALUE_LEN 8

/** length of tx.output.asset_id */
#define ASSET_ID_LEN 32

/** length of tx.output.script_hash */
#define SCRIPT_HASH_LEN 20

/** length of the checksum used to convert a tx.output.script_hash into an Address. */
#define SCRIPT_HASH_CHECKSUM_LEN 4

/** length of a tx.output Address, after Base58 encoding. */
#define ADDRESS_BASE58_LEN 35

/** length of a tx.output Address before encoding, which is the length of <address_version>+<script_hash>+<checksum> */
#define ADDRESS_LEN (1 + SCRIPT_HASH_LEN + SCRIPT_HASH_CHECKSUM_LEN)

/** the current version of the address field */
#define ADDRESS_VERSION 0

/** the length of a SHA256 hash */
#define SHA256_HASH_LEN 32

/** the position of the decimal point, 8 characters in from the right side */
#define DECIMAL_PLACE_OFFSET 8

/** Label when a public key has not been set yet */
static const char NO_PUBLIC_KEY_0[] = "No Public Key";
static const char NO_PUBLIC_KEY_1[] = "Requested Yet";

// concatenate the ADDRESS_VERSION and the address.
unsigned char address[ADDRESS_LEN];

/** array of capital letter hex values */
static const char HEX_CAP[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

/** array of base58 aplhabet letters */
static const char BASE_58_ALPHABET[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q',
		'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z' };

/** array of base10 aplhabet letters */
static const char BASE_10_ALPHABET[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

/** MAX_TX_TEXT_WIDTH in blanks, used for clearing a line of text */
static const char TXT_BLANK[] = "                 ";

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length);

/** encodes in_length bytes from in into base-10, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_10(const void *in, const unsigned int in_length, char *out, const unsigned int out_length) {
	return encode_base_x(BASE_10_ALPHABET, sizeof(BASE_10_ALPHABET), in, in_length, out, out_length);
}

/** encodes in_length bytes from in into base-58, writes the converted bytes to out, stopping when it converts out_length bytes.  */
static unsigned int encode_base_58(const void *in, const unsigned int in_len, char *out, const unsigned int out_len) {
	return encode_base_x(BASE_58_ALPHABET, sizeof(BASE_58_ALPHABET), in, in_len, out, out_len);
}

/** encodes in_length bytes from in into the given base, using the given alphabet. writes the converted bytes to out, stopping when it converts out_length bytes. */
static unsigned int encode_base_x(const char * alphabet, const unsigned int alphabet_len, const void * in, const unsigned int in_length, char * out,
		const unsigned int out_length) {
	char tmp[64];
	char buffer[128];
	unsigned char buffer_ix;
	unsigned char startAt;
	unsigned char zeroCount = 0;
	if (in_length > sizeof(tmp)) {
		hashTainted = 1;
		THROW(0x6D11);
	}
	os_memmove(tmp, in, in_length);
	while ((zeroCount < in_length) && (tmp[zeroCount] == 0)) {
		++zeroCount;
	}
	buffer_ix = 2 * in_length;
	if (buffer_ix > sizeof(buffer)) {
		hashTainted = 1;
		THROW(0x6D12);
	}

	startAt = zeroCount;
	while (startAt < in_length) {
		unsigned short remainder = 0;
		unsigned char divLoop;
		for (divLoop = startAt; divLoop < in_length; divLoop++) {
			unsigned short digit256 = (unsigned short) (tmp[divLoop] & 0xff);
			unsigned short tmpDiv = remainder * 256 + digit256;
			tmp[divLoop] = (unsigned char) (tmpDiv / alphabet_len);
			remainder = (tmpDiv % alphabet_len);
		}
		if (tmp[startAt] == 0) {
			++startAt;
		}
		buffer[--buffer_ix] = *(alphabet + remainder);
	}
	while ((buffer_ix < (2 * in_length)) && (buffer[buffer_ix] == *(alphabet + 0))) {
		++buffer_ix;
	}
	while (zeroCount-- > 0) {
		buffer[--buffer_ix] = *(alphabet + 0);
	}
	const unsigned int true_out_length = (2 * in_length) - buffer_ix;
	if (true_out_length > out_length) {
		THROW(0x6D14);
	}
	os_memmove(out, (buffer + buffer_ix), true_out_length);
	return true_out_length;
}

void public_key_hash_hash160(unsigned char * in, unsigned short inlen, unsigned char *out) {
	union {
		cx_sha256_t shasha;
		cx_ripemd160_t riprip;
	} u;
	unsigned char buffer[32];

	cx_sha256_init(&u.shasha);
	cx_hash(&u.shasha.header, CX_LAST, in, inlen, buffer);
	cx_ripemd160_init(&u.riprip);
	cx_hash(&u.riprip.header, CX_LAST, buffer, 32, out);
}


// Converts compressed public key to address
static void to_address(char * dest, unsigned int dest_len, const unsigned char * public_key_compressed) {
	/*
	 * Address format 1+20=21 bytes:
	 *  Version byte + 20 byte RIPMD160(SHA256(SHA256(compressed public key)))
	 *
	 * Base 58 address format 20+1+4 bytes:
	 *	20 bytes RIPMD(...) + Version byte + 4 checksum bytes
	 */
	static cx_sha256_t address_hash;
	static cx_ripemd160_t address_rip;

	unsigned char address_hash_result_0[SHA256_HASH_LEN];
	unsigned char address_hash_result_1[SHA256_HASH_LEN];

	// do a sha256 hash of the public key twice.
	cx_sha256_init(&address_hash);	
	cx_hash(&address_hash.header, CX_LAST, public_key_compressed, 33, address_hash_result_0);
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address_hash_result_0, SHA256_HASH_LEN, address_hash_result_1);

	// do a RIPMD160 to sha256(sha256(public key))
	unsigned char address_ripmd_hash[SCRIPT_HASH_LEN];
	cx_ripemd160_init(&address_rip);
	cx_hash(&address_rip.header, CX_LAST, address_hash_result_1, SHA256_HASH_LEN, address_ripmd_hash);

	// Get address
	os_memmove(address, address_ripmd_hash, SCRIPT_HASH_LEN);
	address[20] = ADDRESS_VERSION;

	unsigned char checksum[SHA256_HASH_LEN];
	cx_sha256_init(&address_hash);
	cx_hash(&address_hash.header, CX_LAST, address, 21, checksum);
	os_memmove(address + 21, checksum, 4);

	// encode the version + address + checksum in base58
	//	encode_base_58(address, ADDRESS_LEN, dest, dest_len);
}

void display_address(const unsigned char * public_key) {
	// os_memmove(current_public_key[0], TXT_BLANK, sizeof(TXT_BLANK));
	// os_memmove(current_public_key[1], TXT_BLANK, sizeof(TXT_BLANK));
	// os_memmove(current_public_key[2], TXT_BLANK, sizeof(TXT_BLANK));

	unsigned char public_key_compressed[33];
	public_key_compressed[0] = ((public_key[64] & 1) ? 0x03 : 0x02);
	os_memmove(public_key_compressed + 1, public_key, 32);

	char address_base58[ADDRESS_BASE58_LEN];
	unsigned int address_base58_len_0 = ADDRESS_BASE58_LEN/3;
	unsigned int address_base58_len_1 = ADDRESS_BASE58_LEN/3;
	unsigned int address_base58_len_2 = ADDRESS_BASE58_LEN - 2*address_base58_len_0;

	to_address(address_base58, ADDRESS_BASE58_LEN, public_key_compressed);

//	os_memmove(current_public_key[0], address_base58_0, address_base58_len_0);
//	os_memmove(current_public_key[1], address_base58_1, address_base58_len_1);
//	os_memmove(current_public_key[2], address_base58_2, address_base58_len_2);
}
