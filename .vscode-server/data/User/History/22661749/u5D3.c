#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    int fd2 = SocketTCP();
    if (fd1 != -1)
    {
        Test(fd1);
    }
    if (fd2 != -1)
    {
        Test(fd2);
    }
    Halt();
    return 0;
}