# Skycoin BOLOS app for Ledger Nano

## Overview
Ledger Nano integration for Skycoin

[![Build Status](https://travis-ci.com/skycoin/ledger-nano.svg)](https://travis-ci.com/skycoin/ledger-nano)

This repository provides a BOLOS app for Skycoin integration with Ledger Nano S 

## Install tools

To setup toolchain, SDK and Python Loader follow the [instructions](https://ledger.readthedocs.io/en/latest/userspace/getting_started.html)

## Build instructions:

### Get list of targets(help)

```
make 
make help
```

### Build(compile) the app

```
make all
```

### Build and load the app to the ledger

```
make load
```

### Remove app from the ledger

```
make delete
```

### Clean the build files

```
make clean
```
