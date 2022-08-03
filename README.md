# Compiling and strip

```
$ arm-linux-gnueabihf-gcc serial-loopback-test.c -o serial-loopback-test
$ arm-linux-gnueabihf-strip serial-loopback-test
```

# How to use

Loopback you UART TX and RX in hardware first.

Run the following example command to test:

```
$ ./serial-loopback-test /dev/ttymxc1 115200 "slfjslfjslfjslfjslfjslfjslfjslfjsldjfs"
```

The send file testing is failed durning my testing, so use string method instead.


# Result checking

If checking passed, 0 is returned, otherwise, 1 is returned.

```
echo $?
```

The loop count is set to 200 for more handy script testing


# Example result

## Failed result

```
# time /serial-loopback-test /dev/ttyS3 115200 'fslfjsldjfsldfjslfjsldfjsfjksfjs'
stdalt not opened; Alternative file descriptor: 3
Opening port /dev/ttyS3;
Got speed 115200 (4098/0x1002);
Got file/string 'fslfjsldjfsldfjslfjsldfjsfjksfjs'; interpreting as string (32).

+++START+++
CTRL+C to exit
Timeout Or Error

real	0m10.004s
user	0m9.930s
sys	0m0.000s
# time /serial-loopback-test /dev/ttyS3 115200 'fslfjsldjfsldfjslfjsldfjsfjksfjs'
stdalt not opened; Alternative file descriptor: 3
Opening port /dev/ttyS3;
Got speed 115200 (4098/0x1002);
Got file/string 'fslfjsldjfsldfjslfjsldfjsfjksfjs'; interpreting as string (32).

+++START+++
CTRL+C to exit
Timeout Or Error

real	0m10.006s
user	0m9.920s
sys	0m0.000s

# echo $?
1
```

## OK result

```
# time /serial-loopback-test /dev/ttyS3 115200 'fslfjsldjfsldfjslfjsldfjsfjksfjs'
stdalt not opened; Alternative file descriptor: 3
Opening port /dev/ttyS3;
Got speed 115200 (4098/0x1002);
Got file/string 'fslfjsldjfsldfjslfjsldfjsfjksfjs'; interpreting as string (32).

+++START+++
CTRL+C to exit

+++DONE+++
CYCLES PASS: 200 FAILED: 0
Wrote: 6400 bytes; Read: 6400 bytes; Total: 12800 bytes. 
Test time: 0 s 814655 us. 
Measured: write 7856.09 Bps, read 7856.09 Bps, total 15712.17 Bps.

real	0m0.820s
user	0m0.610s
sys	0m0.160s

# echo $?
0
```
