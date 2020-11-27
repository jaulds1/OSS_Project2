#include "../COMMON/char_ioctl.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

//
// This header is required for the ioctl() call
#include <sys/ioctl.h>

#define MAX_READ_SIZE 32
#define MAX_WRITE_SIZE 32


int ioctl_set_data(int fd, char * data)
{
    int i;
    char c;

    printf("[+] %s called\n", __FUNCTION__ );

    ioctl(fd, IOCTL_WRITE_TO_KERNEL, data );

    printf("[+]    Data written: %s\n", data );

    return 0;

}

int ioctl_read_data(int fd, char * data)
{
    int i;
    char c;

    printf("[+] %s called\n", __FUNCTION__ );

    ioctl(fd, IOCTL_READ_FROM_KERNEL, data );

    printf("[+]    Data read: %s\n", data );

    return 0;

}

int main( int argc, char ** argv )
{
    int fd_a = -1;
    int fd_b = -1;
    char* dev_a = "a";
    char* dev_b = "b";
    int ret = -1;
    char set_data[32];
    char read_data[32];
    char read_from_b[MAX_READ_SIZE];
    char read_from_a[MAX_READ_SIZE];
    char write_to_a[MAX_WRITE_SIZE];

    memset(read_from_a, 0, 1024);
    memset(read_from_b, 0, 1024);

    memset(read_data, 0, 32 );

    strcpy( set_data, "Hello world!\n");

    char devname_a[32];
    strcpy(devname_a, "/dev/");
    strcat(devname_a, DEVICE_NAME_A );

    printf("it got before a\n");
    fd_a = open(devname_a, O_WRONLY);

    if (fd_a < 0)
    {
        printf("Can't open device file: %s\n", DEVICE_NAME_A);
        return -1;
    }
    strcpy(write_to_a, "Testing123\n");
    write(fd_a, write_to_a, MAX_WRITE_SIZE);

    ret = ioctl_set_data(fd_a, set_data);
    ret = ioctl_read_data(fd_a, read_data);
    read(fd_a, read_from_a, 16);
    printf("I read: %s\n", read_from_a);
    //
    // You will also use the read() and write() system calls
    //


    close(fd_a);

    return 0;
}
