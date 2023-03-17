#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    int fd2 = SocketTCP();
    if (fd1 != 0) {
       Test(fd1);
    }
    if (fd2 != 0) {
       Test(fd2);
    }
    //Halt();
    return 0;
}