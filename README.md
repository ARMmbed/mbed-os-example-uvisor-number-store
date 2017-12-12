# Threaded RPC with uVisor example

[![Build Status](https://travis-ci.org/ARMmbed/mbed-os-example-uvisor-number-store.svg?branch=master)](https://travis-ci.org/ARMmbed/mbed-os-example-uvisor-number-store)

This is a simple example to show how to use several uVisor APIs to build a box that securely stores a number.
This number can only be written by one box, but read by all boxes.

* Create and run secure boxes.
* Secure static and dynamic memory inside secure boxes.
* Run threads inside secure boxes.
* Issue RPC requests to other secure boxes and wait for their completion.
* Receive and process incoming RPC requests from other secure boxes.
* Get box id and namespace of the RPC caller.

This demo contains three secure boxes:

1. The secure number vault. This box stores one number that can only be written to by Client A, but read by everyone.
1. Client A, which attempts to write (and succeeds) and read the secure number.
1. Client B, which attempts to write (but fails) and read the secure number.

As usual, all the code/data that is not protected by a secure box ends up in the public box (also known as box 0), which is visible by all other boxes and, hence, insecure. The public box also attempts to write (but fails) and read the secure number.

Supported devices:

| Target            | Toolchain | Baud rate |
|-------------------|-----------|-----------|
| `K64F`            | `GCC_ARM` | 9600      |
| `DISCO_F429ZI`    | `GCC_ARM` | 9600      |
| `EFM32GG_STK3700` | `GCC_ARM` | 9600      |

Latest release: [mbed-os-5.4.x](https://github.com/ARMmbed/mbed-os-example-uvisor/releases/tag/latest). Tested with [mbed-cli v1.0.0](https://github.com/ARMmbed/mbed-cli/releases/tag/1.0.0).

## Quickstart

For a release build, please enter:

```bash
$ mbed compile -m K64F -t GCC_ARM -c
```

You will find the resulting binary in `BUILD/K64F/GCC_ARM/mbed-os-example-uvisor-number-store.bin`. You can drag and drop it onto your board USB drive.

Press the reset button. The `box_number_store` secure box will use 3 LEDs on your target to signal the execution of the `get_caller_id`, `get_number` and `set_number` APIs. You can observe the example output on the serial port:

```bash
$ screen /dev/tty.usbmodem1422 9600
```

You will see an output similar to the following one:

```
***** uVisor secure number store example *****
vault   : Only client_a can write into the vault
vault   : All clients can read the vault
client_b: Attempt to write  0xFFFFFED4 (denied)
client_a: Attempt to read : 0x00000000 (granted)
client_a: Attempt to write  0xFFFFFE0C (granted)
public  : Attempt to read : 0xFFFFFE0C (granted)
public  : Attempt to write  0x00000019 (denied)
client_b: Attempt to read : 0xFFFFFE0C (granted)
client_a: Attempt to read : 0xFFFFFE0C (granted)
...
```

> **Note**: If your target does not have 3 different LEDs or LED colours, you will see the same LED blink multiple times. The example use the general mbed OS naming structure `LED1`, `LED2`, `LED3`.

### Debug

When a debugger is connected, you can observe debug output from uVisor. Please note that these messages are sent through semihosting, which halts the program execution if a debugger is not connected. For more information please read the [Debugging uVisor on mbed OS](https://github.com/ARMmbed/uvisor/blob/master/docs/lib/DEBUGGING.md) guide. To build a debug version of this example, please enter:

```bash
$ mbed compile -m K64F -t GCC_ARM --profile mbed-os/tools/profiles/debug.json -c
```

## Known issues

- Use of exporters for multiple IDEs is not supported at the moment.
- uVisor will halt the system on unregistered interrupts arrival. Do not attempt to push any SW buttons not listed in this document.

