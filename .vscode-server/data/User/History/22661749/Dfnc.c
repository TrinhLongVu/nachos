
#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{   
    Test(argv[1]);
    Halt();
    return 0;
}