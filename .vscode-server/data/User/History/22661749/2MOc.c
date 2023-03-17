#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{   
    Test(argv[2]);
    Halt();
    return 0;
}