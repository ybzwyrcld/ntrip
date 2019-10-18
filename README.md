# Ntrip

Simple NTRIP caster/client/server example programs, support the NTRIP 1.0/2.0 protocol;


## Quick Start

This assumes you are running Ubuntu 16.04

Clone and run the build:

```bash
$ git clone https://github.com/hanoi404/ntrip && cd ntrip
$ make all
```

First step, run the `ntrip_caster`:
```bash
$ ./ntrip_caster
```

Second step, run the `ntrip_server`:
```bash
$ ./ntrip_server
```

Third step， run the `ntrip_client`:
```bash
$ ./ntrip_client
```

After the above steps are completed, you can see that the example data sent by ntripserver flows to ntripclient through ntripcaster.

## CMake

Config and build:

```bash
$ mkdir build && cd build
$ cmake .. -DNTRIP_BUILD_EXAMPLES=ON
$ make
```

Output programs in `examples` directory:

```bash
examples/ntrip_caster
examples/ntrip_client
examples/ntrip_server
```


