#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    if (fd1 != -1) {
       Test("12");
    }
    Halt();
    return 0;
}