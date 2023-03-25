#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();

    Connect(fd1, "192.168.27.0", 5000);

    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);
    // Send(fd1, "hello", 15);
    // Receive(fd1, "hi", 16);

    // OpenFileId id = Open("test.txt", 1);
    // Test("1",7,5);

    // Halt();
    return 0;
}