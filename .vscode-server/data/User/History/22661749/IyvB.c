#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    char buffer[100];
    Test(buffer);
    Halt();
    return 0;
}