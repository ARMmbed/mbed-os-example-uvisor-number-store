# Threaded RPC with uVisor example

This is a simple example to show how to use several uVisor APIs to build a box that securely stores a number.
This number can only be written by one box, but read by all boxes.

* Create and run secure boxes.
* Secure static and dynamic memory inside secure boxes.
* Run threads inside secure boxes.
* Issue RPC requests to other secure boxes and wait for their completion.
* Receive and process incoming RPC requests from other secure boxes.
* Get box id and namespace of the RPC caller.

This demo contains three secure boxes:

1. The secure number vault. This box stores one number that can only be written to by client A, but read by everyone.
1. Client A, which attempts to write (and succeeds) and read the secure number.
1. Client B, which attempts to write (but fails) and read the secure number.

The insecure box 0 also attempts to write (but fails) and read the secure number.

Supported devices:

| Target              | Toolchain | Baud rate    |
|---------------------|-----------|--------------|
| `ARM_MPS2_ARMv8MML` | `GCC_ARM` | See `stdout` |

Latest release: [uvisor-v8m-epr](https://github.com/ARMmbed/mbed-os-example-uvisor-number-store/releases/tag/uvisor-v8m-epr). Tested with [mbed-cli v1.0.0](https://github.com/ARMmbed/mbed-cli/releases/tag/1.0.0).

## Caveats

* This is a ARMv8-M specific example, and only works on an ARMv8-M FastModel. For the officially supported example, please use the [master branch](https://github.com/ARMmbed/mbed-os-example-uvisor-number-store).
* The ARMv8-M target requires GCC 5.4.1 20160919 or higher.
* The ARMv8-M target used via our FastModel binary cannot show LED output. Refer to `stdout` instead.

For a complete list of ARMv8-M-specific known issue, check out the [uvisor-tests-v8m README](https://github.com/ARMmbed/uvisor-tests-v8m/tree/dev/v8m#caveats).

## Quickstart

First, in order to use the ARMv8-M target you need to setup our Docker'ized FastModel. This allows you to use the FastModel as a self-contained binary on both Linux and macOS. First, clone the FastModel repository:

```bash
$ mkdir -p ~/code
$ cd ~/code
$ git clone git@github.com:ARMmbed/uvisor-fastmodel-testing
$ cd uvisor-fastmodel-testing
```

If you have git-lfs installed, the clone operation will have fetched the Docker image, otherwise you need to visit [this link](https://github.com/ARMmbed/uvisor-fastmodel-testing/blob/master/mbed-v8m.tar.xz) and download it manually:

[https://github.com/ARMmbed/uvisor-fastmodel-testing/blob/master/mbed-v8m.tar.xz](https://github.com/ARMmbed/uvisor-fastmodel-testing/blob/master/mbed-v8m.tar.xz)

Once the Docker image has been downloaded, load it via Docker:

```bash
$ docker load -i mbed-v8m.tar.xz
```

You can verify that it has been loaded correctly by running `docker images`:

```bash
REPOSITORY   TAG      IMAGE ID       CREATED      SIZE
mbed-v8m     latest   402206ca9a0b   2 days ago   108.7 MB
```

Run our provided script to create links to the Docker'ized fastmodel binaries and then add them to your path:

```bash
$ docker/tools/link_binaries.sh
$ export PATH=$PATH:$HOME/code/uvisor-fastmodel-testing/docker/bin
```

You can verify that the path is set correctly by running:

```bash
$ which simulate
~/uvisor-fastmodel-testing/docker/bin/simulate
```

Finally, you can clone this repository, select the `dev/v8m` branch, and build the example app:

```bash
$ cd ~/code
$ git clone git@github.com:ARMmbed/mbed-os-example-uvisor-number-store
$ cd mbed-os-example-uvisor-number-store
$ git checkout dev/v8m
$ mbed deploy

# For a release build:
$ mbed compile -m ARM_MPS2_ARMv8MML -t GCC_ARM -c

# For a debug build:
$ mbed compile -m ARM_MPS2_ARMv8MML -t GCC_ARM -c --profile mbed-os/tools/profiles/debug.json
```

You will find the resulting ELF file in `BUILD/ARM_MPS2_ARMv8MML/GCC_ARM/mbed-os-example-uvisor-number-store.elf`.

To execute the example, simply run:

```bash
$ simulate FVP_MPS2_Cortex-M33 BUILD/ARM_MPS2_ARMv8MML/GCC_ARM/mbed-os-example-uvisor-number-store.elf
```

You can observe the serial output in `stdout` from the `simulate` command. You will see an output similar to the following one:

```
Model selected: FVP_MPS2_Cortex-M33.
Window could not be created! SDL Error: No available video device
Simulation is started

**** uVisor secure number store example *****
Trusted client a has box id 2
2: Wrote '0xfffffed4'
1: Read '0xfffffed4'
1: Permission denied. This client cannot write the secure number '0xfffffe0c'
0: Read '0xfffffed4'
0: Permission denied. This client cannot write the secure number '0x00000019'
2: Read '0xfffffed4'
2: Wrote '0xfffffda8'
2: Read '0xfffffda8'
1: Permission denied. This client cannot write the secure number '0xfffffc18'
2: Wrote '0xfffffc7c'
...
```
