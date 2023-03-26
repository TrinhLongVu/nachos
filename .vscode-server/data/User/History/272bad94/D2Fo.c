#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();

    Connect(fd1, "127.0.0.1", 12345);

    Send(fd1, "hello anh em", 15);

    const char * bu

    Receive(fd1, "hi", 16);
    
    Halt();
    return 0;
}