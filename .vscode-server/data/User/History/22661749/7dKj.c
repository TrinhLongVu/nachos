#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    char * buffer = new char[200];
    Test(buffer);
    Halt();
    return 0;
}