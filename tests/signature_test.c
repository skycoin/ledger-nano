#include "signature_test.h"	
#include "stdlib.h"

#include "../src/skycoin-api/skycoin_crypto.c"

START_TEST(test_convert_signature_from_TLV_to_RS) {
	for (size_t i = 0; i < 1; i++)
	{
		char* tlv = TLV_to_RS_signatures[i][0];
		char* rs = TLV_to_RS_signatures[i][1];

		// printf("tlv: %s\n", tlv);
		// printf("rs: %s\n", rs);
	}
	
	// translate_pin_code("483926571", pin_code);
	// ck_assert_str_eq(pin_code, "123");

	// strcpy(pin_code, "672");
	// translate_pin_code("638591247", pin_code);
	// ck_assert_str_eq(pin_code, "123");
}
END_TEST


Suite *signature_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Signature");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_convert_signature_from_TLV_to_RS);
    suite_add_tcase(s, tc_core);

    return s;
}

int main() {
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = signature_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? 0 : 1;
}