#include "../userprog/syscall.h"
#include "../userprog/synchconsole.h"

int main(int argc, char *argv[])
{   
    Test(argv[1]);
    Halt();
    return 0;
}