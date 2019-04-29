#include <check.h>

#include "bolos_replacement.h"


/*
  each entry of this 
*/
const char TLV_to_RS_signatures[][2][140] = {
    {
        "3044022047efd2bfe73ac36090e91fa78ebb84cafc40ffb7778fa61222260afffedff08302200943567580955e204ee41ed1ba3cfec6f902cfa39c22b3b06aa1de0963e4b863", 
        "47efd2bfe73ac36090e91fa78ebb84cafc40ffb7778fa61222260afffedff0830943567580955e204ee41ed1ba3cfec6f902cfa39c22b3b06aa1de0963e4b863a5"
    }
};

TCase *add_signature_tests(TCase *tc);
