#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{   
    Test(argv[5]);
    Halt();
    return 0;
}