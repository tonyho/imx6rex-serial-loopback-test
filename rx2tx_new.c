/*
 * uart-loopback.c - userspace uart loopback test
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <stddef.h>

#include <sys/time.h>

#include "serial-loopback-test.h"
//Read timeout 500s
#define TIMEOUT 500

void print_usage(char *prg) {
    fprintf(stderr, "\nUsage: %s baud /dev/ttyUSB0 [dump]\n\n", prg);
    exit(1);
}

int main(int argc, char **argv) {
    fd_set readfs;    /* file descriptor set */
    struct timeval Timeout;
    int fd,ret;
    char *tty;
    char j[2];
    char *btr = NULL;
    int opt,count=127;
    //char i;

    unsigned long counter = 0;

    speed_t serspeed_t;

    if (argc < 3) {
        print_usage(argv[0]);
    }

    //btr = argv[1];
    //printf("uart port baud used: %s\n",btr);

    char *serspeed;
    // Get the baudrate
    serspeed = argv[1];
    serspeed_t = string_to_baud(serspeed);
    fprintf(stdout, "Got speed %s (%d/0x%x);\n", serspeed, serspeed_t, serspeed_t);

    //tty = argv[optind];
    tty = argv[2];

    int dump_data = 0;
    if(argc == 4) {
        if(strcmp("dump", argv[3])) {
            printf("Disable data dump\n");
        } else {
            printf("Enabled data dump\n");
            dump_data = 1;
        }
    }

    printf("count=%d\n",count);

    printf("uart port used: %s\n",tty);

#if 0
    if ((fd = open (tty, O_RDWR | O_NOCTTY)) < 0) {
        perror(tty);
        exit(1);
    }
#endif

    // Open and Initialise port
    fd = open( tty, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if ( fd < 0 ) { perror(tty); return 1; }
    initport( fd, serspeed_t );

    //tcflush(fd, TCIFLUSH);

    //for (i=33; i < count+33; i++) {
    for (;;) {
        FD_SET(fd, &readfs);  /* set testing source */

        /* set timeout value within input loop */
        Timeout.tv_usec = 0;  /* milliseconds */
        Timeout.tv_sec  = TIMEOUT;  /* seconds */
        ret = select(fd+1, &readfs, NULL, NULL, &Timeout);
        if (ret==0){
            printf("read timeout error!\n");
            exit(1);
        } else {
            //printf("-");
            ret=read(fd, &j, 1);
        }

        if (ret!=1) {
            printf("read error!\n");
            exit(1);
        } else {
            //printf("read OK\n");
            counter += ret;
        }

        if(dump_data == 1) {
            printf("%c ", j[0]);
        } else {
            //printf(".");
        }
        //Send back
        //ret=write(fd,&i, 1);
        int ret1=write(fd,&j, ret);
        if (ret1 != ret) {
            printf("write error!\n");
            exit(1);
        }

#if 0
        if ( i!=j[0] ) {
            printf("read data error: wrote 0x%x read 0x%x\n",i,j[0]);
            exit(1);
        }
#endif
        if(counter % 200 == 0) {
            printf("read count:%lu\n", counter);
        }
        //printf("%c",j[0]);

    }

    printf("\n");
    close(fd);
    return 0;
}
