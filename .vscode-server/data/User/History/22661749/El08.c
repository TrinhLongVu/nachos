#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{   
    Test(argv[6]);
    Halt();
    return 0;
}