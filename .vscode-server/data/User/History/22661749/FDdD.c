#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    if (fd1 != -1) {
       Test(fd1);
    }
    Halt();
    return 0;
}