#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    if (fd1 != -1) {
       Test();
    }
    Halt();
    return 0;
}