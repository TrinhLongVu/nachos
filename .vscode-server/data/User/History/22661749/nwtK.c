#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    int fd2 = SocketTCP();
    int fd3 = SocketTCP();
    Test(fd1);
    Halt();
    return 0;
}