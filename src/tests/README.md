# Tests for Skycoin Ledger app  

This folder provides files for testing Skycoin Ledger BOLOS app.

Unfortunately, now the main problem is that we do not know any working emulator which we can use to test functions which require Ledger SDK.

So, at least for now, we test functions which do not require Ledger SDK directly (e.g. compress_public_key or generate_address). For this, we use data from the cipher-testdata.  

## How to install

For testing, we are using Check (a unit testing framework for C). To install it, follow the [instructions](https://libcheck.github.io/check/web/install.html).

If you are using Ubuntu/Debian, the easiest way to install it:
```

sudo apt-get install check

```
