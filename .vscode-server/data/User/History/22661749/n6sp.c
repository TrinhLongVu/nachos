#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    int fd2 = SocketTCP();
    int fd3 = SocketTCP();

    int x = Send(fd1, "hello", 15);

    Test(x);
    Halt();
    return 0;
}