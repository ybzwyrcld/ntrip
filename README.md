# Ntrip

Simple NTRIP caster/client/server example programs, support the NTRIP 1.0/2.0 protocol;



## Quick Start

This assumes you are running Ubuntu 16.04

Clone and run the build:

```bash
$ git clone https://github.com/hanoi404/ntrip && cd ntrip
$ make all
```

First step, run the `ntrip_caster_exam`:
```bash
$ ./ntrip_caster_exam
```

Second step, run the `ntrip_server_exam`:
```bash
$ ./ntrip_server_exam
```

Third step， run the `ntrip_client_exam`:
```bash
$ ./ntrip_client_exam
```

After the above steps are completed, you can see that the example data sent by **NtripServer** flows to **NtripClient** through **NtripCaster**.



## CMake

### Linux

Configure and compile:

```bash
$ mkdir build && cd build
$ cmake .. -DNTRIP_BUILD_EXAMPLES=ON
$ make
```

Output executable file:

```bash
build/examples/ntrip_caster_exam
build/examples/ntrip_client_exam
build/examples/ntrip_client_to_ntrip_server_exam
build/examples/ntrip_server_exam
```

### Windows

#### VS2019

Configure and compile:

```bash
$ mkdir build && cd build
$ cmake .. -G "Visual Studio 16" -DNTRIP_BUILD_EXAMPLES=ON
$ cmake --build . --config Release
```

Or open `build/ntrip.sln` with **VS2019** after the configuration is complete.

Output executable file:

```bash
build/examples/Release/ntrip_caster_exam.exe
build/examples/Release/ntrip_client_exam.exe
build/examples/Release/ntrip_client_to_ntrip_server_exam.exe
build/examples/Release/ntrip_server_exam.exe
```

#### MinGW

Configure and compile:

```bash
$ mkdir build && cd build
$ cmake -G "Unix Makefiles" .. -DNTRIP_BUILD_EXAMPLES=ON
$ make
```

Output executable file:

```bash
build/examples/ntrip_caster_exam
build/examples/ntrip_client_exam
build/examples/ntrip_client_to_ntrip_server_exam
build/examples/ntrip_server_exam
```



for using **NtripCaster**, Add configuration option `-DNTRIP_BUILD_CASTER=ON`.

