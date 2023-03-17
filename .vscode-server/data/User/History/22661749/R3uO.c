#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    Test(argv[4]);
    Halt();
    return 0;
}