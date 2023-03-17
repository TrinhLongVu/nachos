#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{   
    Test(argv[1]);
    printf(123);
    Halt();
    return 0;
}