#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    int fd2 = SocketTCP();
    int fd3 = SocketTCP();
    if (fd1 != -1)
    {
        Test(fd1);
    }
    if (fd2 != -1)
    {
        Test(fd2);
    }
    Test(fd3);
    Halt();
    return 0;
}