#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    int fd1 = SocketTCP();
    
    int x = Send(fd1, "hello", 15);
    int y = Receive(fd1, "hi", 16);
    Test(x);
    Halt();
    return 0;
}