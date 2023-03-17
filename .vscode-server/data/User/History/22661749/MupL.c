#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();

    Connect(fd1, "192.168.27.0", 123);

    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);



    Halt();
    return 0;
}