gcc rx2tx.c -o rx2tx
gcc rx2tx.c -o rx2tx
./rx2tx 9600  /dev/ttyUSB2 dump
arm-linux-gnueabihf-gcc serial-loopback-test.c -o s.arm
scphimx s.arm root@192.168.1.163:/
scphimx s.arm root@192.168.1.163:/
