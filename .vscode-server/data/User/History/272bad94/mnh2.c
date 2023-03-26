#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    char *buffer[100];

    Connect(fd1, "127.0.0.1", 12345);

    Send(fd1, "hello anh em", 15);


    Receive(fd1, buffer, 16);

    PrintString(buffer);
    
    
    Halt();
    return 0;
}