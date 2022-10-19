/*
    writeread.c - based on writeread.cpp
    [SOLVED] Serial Programming, Write-Read Issue - http://www.linuxquestions.org/questions/programming-9/serial-programming-write-read-issue-822980/

    build with: gcc -o writeread -Wall -g writeread.c
*/

// Ref:
// Timeout in 3 seconds, ref: https://copyprogramming.com/howto/how-can-i-implement-timeout-for-read-when-reading-from-a-serial-port-c-c
// Base loopback code from: https://github.com/FEDEVEL/imx6rex-serial-loopback-test
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <stdlib.h>
#include <sys/time.h>

#include "serial-loopback-test.h"

#define REPEAT 500

#include <setjmp.h>
void DumpHex(const void* data, size_t size);
static jmp_buf env_alarm;
#define TIMEOUT 20 //10seconds
static void sig_alarm(int signo)
{
    longjmp(env_alarm, 1);
}

int serport_fd;

void usage(char **argv)
{
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "%s port baudrate file/string\n", argv[0]);
    fprintf(stdout, "Examples:\n");
    fprintf(stdout, "%s /dev/ttyUSB0 115200 /path/to/somefile.txt\n", argv[0]);
    fprintf(stdout, "%s /dev/ttyUSB0 115200 \"some text test\"\n", argv[0]);
}

//CTRL+C handler
int execute;
void trap(int signal) {execute = 0;}

int main( int argc, char **argv )
{

    if( argc != 4 ) {
        usage(argv);
        return 1;
    }

    char *serport;
    char *serspeed;
    speed_t serspeed_t;
    char *serfstr, *recievedBytes;
    int serf_fd; // if < 0, then serfstr is a string
    int bytesToSend;
    int sentBytes, sentBytesTotal = 0;
    char byteToSend[2];
    int readChars;
    int recdBytes, totlBytes;

    char sResp[11];

    struct timeval timeStart, timeEnd, timeDelta;
    float deltasec;

    int write_failed;
    int cyclesPass = 0, cyclesFailed = 0;

    /* Re: connecting alternative output stream to terminal -
    * http://coding.derkeiler.com/Archive/C_CPP/comp.lang.c/2009-01/msg01616.html
    * send read output to file descriptor 3 if open,
    * else just send to stdout
    */
    FILE *stdalt;
    if(dup2(3, 3) == -1) {
        fprintf(stdout, "stdalt not opened; ");
        stdalt = fopen("/dev/tty", "w");
    } else {
        fprintf(stdout, "stdalt opened; ");
        stdalt = fdopen(3, "w");
    }
    fprintf(stdout, "Alternative file descriptor: %d\n", fileno(stdalt));

    // Get the PORT name
    serport = argv[1];
    fprintf(stdout, "Opening port %s;\n", serport);

    // Get the baudrate
    serspeed = argv[2];
    serspeed_t = string_to_baud(serspeed);
    fprintf(stdout, "Got speed %s (%d/0x%x);\n", serspeed, serspeed_t, serspeed_t);

    //Get file or command;
    serfstr = argv[3];
    serf_fd = open( serfstr, O_RDONLY );
    fprintf(stdout, "Got file/string '%s'; ", serfstr);
    if (serf_fd < 0) {
        bytesToSend = strlen(serfstr);
        fprintf(stdout, "interpreting as string (%d).\n", bytesToSend);
    } else {
        struct stat st;
        stat(serfstr, &st);
        bytesToSend = st.st_size;
        fprintf(stdout, "opened as file (%d).\n", bytesToSend);
    }


    // Open and Initialise port
    serport_fd = open( serport, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if ( serport_fd < 0 ) { perror(serport); return 1; }
    initport( serport_fd, serspeed_t );

    sentBytes = 0; recdBytes = 0;
    byteToSend[0]='x'; byteToSend[1]='\0';
    gettimeofday( &timeStart, NULL );

    //CTRL+C handler
    signal(SIGINT,&trap);
    execute = 1;

    fprintf(stdout, "\n+++START+++\n");

    fprintf(stdout, "CTRL+C to exit\n");


    //alloc number of bytes to send
    recievedBytes = (char*) malloc(sizeof(char) *  bytesToSend);

   if (signal(SIGALRM, sig_alarm) == SIG_ERR) {
       exit(0);
   }
   if (setjmp(env_alarm) != 0) {
      //close(fd);
      printf("Timeout Or Error\n");
      exit(1);
   }
   alarm(TIMEOUT);

    // write / read loop - interleaved (i.e. will always write
    // one byte at a time, before 'emptying' the read buffer )
    int ii = 0;
    for ( ; ii<REPEAT; ii++) {
        while ( sentBytes < bytesToSend ) {
            // read next byte from input...
            if (serf_fd < 0) { //interpreting as string
                byteToSend[0] = serfstr[sentBytes];
            } else { //opened as file
                read( serf_fd, &byteToSend[0], 1 );
            }

            write_failed = TRUE;
            int write_failed_count = 0;
            do {
                 //CTRL+C handler
                if(execute == 0)
                    goto show_results;

                if ( !writeport( serport_fd, byteToSend ) ) {
                    write_failed = TRUE;
                    fprintf(stdout, "!WARNING: Write failed.\n");
                    cyclesFailed ++;
                    write_failed_count ++;
                    if(write_failed_count > 3)
                        break;
                } else
                    write_failed = FALSE;
            } while (write_failed == TRUE);

            while ( wait_flag == TRUE );

            // read was interrupted, try to read again
            while ( ((readChars = readport( serport_fd, sResp, 1)) == -1) && (errno == EINTR) )
                fprintf(stdout, "!WARNING: Read was interrupted, read again.\n");

            if(strcmp(sResp, byteToSend) != 0) {
                fprintf(stdout, "!!!%d of %d, ERROR BYTE: Written: %s Read: %s\n", sentBytes, bytesToSend, byteToSend, sResp);
                //exit(1);
            }
            recdBytes += readChars;
            recievedBytes[sentBytes] = *sResp;

            wait_flag = TRUE; // was ==
            //~ usleep(50000);
            sentBytes++;
        }
        if ( strcmp(serfstr, recievedBytes) != 0) {
            fprintf(stdout, "!!! ERROR STRING: Written: %s Read: %s\n", serfstr, recievedBytes);
            DumpHex(recievedBytes, readChars);
            readChars = readport( serport_fd, sResp, 1);

            cyclesFailed++;
        } else {
            cyclesPass++;
        }

        // CTRL+C handler
        if(execute == 0) {
            goto show_results;
        }

        sentBytesTotal += sentBytes;
        sentBytes = 0;
    }
    //cancel pending alarm
    alarm(0);
show_results:

    //CTRL+C handler
    signal(SIGINT,SIG_DFL);

    gettimeofday( &timeEnd, NULL );

    free(recievedBytes);
    // Close the open port
    close( serport_fd );
    if (!(serf_fd < 0)) close( serf_fd );

    fprintf(stdout, "\n+++DONE+++\n");

    totlBytes = sentBytesTotal + recdBytes;
    timeval_subtract(&timeDelta, &timeEnd, &timeStart);
    deltasec = timeDelta.tv_sec+timeDelta.tv_usec*1e-6;

    fprintf(stdout, "CYCLES PASS: %d FAILED: %d\n", cyclesPass, cyclesFailed);
    fprintf(stdout, "Wrote: %d bytes; Read: %d bytes; Total: %d bytes. \n", sentBytesTotal, recdBytes, totlBytes);
    fprintf(stdout, "Test time: %ld s %ld us. \n", timeDelta.tv_sec, timeDelta.tv_usec);
//    fprintf(stdout, "Start: %ld s %ld us; End: %ld s %ld us; Delta: %ld s %ld us. \n", timeStart.tv_sec, timeStart.tv_usec, timeEnd.tv_sec, timeEnd.tv_usec, timeDelta.tv_sec, timeDelta.tv_usec);
//    fprintf(stdout, "%s baud for 8N1 is %d Bps (bytes/sec).\n", serspeed, atoi(serspeed)/10);
    fprintf(stdout, "Measured: write %.02f Bps, read %.02f Bps, total %.02f Bps.\n", sentBytesTotal/deltasec, recdBytes/deltasec, totlBytes/deltasec);

    //Any error would be set to error
    if(cyclesFailed != 0) {
        return 1;
    }

    return 0;
}


void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ");
			if ((i+1) % 16 == 0) {
				printf("|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}
