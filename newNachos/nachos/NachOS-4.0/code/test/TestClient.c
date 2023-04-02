#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    char *buffer[100];

    Connect(fd1, "127.0.0.1", 12345);
    Read("hello", 15, fd1);
    Write(buffer, 15, fd1);
    PrintString(buffer);
    

    Halt();
    return 0;
}