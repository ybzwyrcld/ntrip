# Ntrip Lib

Simple NTRIP caster/client/server example programs, support the NTRIP 1.0/2.0 protocol;


## Quick Start

This assumes you are running Ubuntu 16.04


Clone and run the build:
```bash
$ git clone https://github.com/hanoi404/ntriplib && cd ntriplib/ntriplib
$ make all
```

First step, run the ntripcaster:
```bash
$ ./ntripcaster
```

Second step, run the ntripserver:
```bash
$ ./ntripserver
```

Third step， run the ntripclient:
```bash
$ ./ntripclient
```

After the above steps are completed, you can see that the example data sent by ntripserver flows to ntripclient through ntripcaster.

