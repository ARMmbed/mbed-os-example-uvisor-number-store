# Threaded RPC with uVisor Example

This is a simple example to show how to use several uVisor APIs to build a box that securely stores a number.
This number can only be written by one box, but read by all boxes.

- Create and run secure boxes.
- Secure static and dynamic memory inside secure boxes.
- Run threads inside secure boxes.
- Issue RPC requests to other secure boxes and wait for their completion.
- Receive and process incoming RPC requests from other secure boxes.
- Get box id and namespace of the RPC caller.

This demo contains three secure boxes:

1. The secure number vault. This box stores one number that can only be written to by client A, but read by everyone.
2. Client A, which attempts to write (and succeeds) and read the secure number.
3. Client B, which attempts to write (but fails) and read the secure number.

The insecure box 0 also attempts to write (but fails) and read the secure number.

## Building

The example currently only works on K64F with the GCC_ARM toolchain.

### Release

For a release build, please enter:

```bash
$ mbed compile -m K64F -t GCC_ARM
```

You will find the resulting binary in `.build/K64F/GCC_ARM/mbed-os-example-uvisor.bin`. You can drag and drop it onto your board USB drive.

### Debug

When a debugger is connected, you can observe debug output from uVisor. Please note that these messages are sent through semihosting, which halts the program execution if a debugger is not connected. For more information please read the [Debugging uVisor on mbed OS](https://github.com/ARMmbed/uvisor/blob/master/docs/api/DEBUGGING.md) guide. To build a debug version of the program:

```bash
$ mbed compile -m K64F -t GCC_ARM -o "debug-info"
```
