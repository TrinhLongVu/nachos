#include "../userprog/syscall.h"

int main(int argc, char *argv[])
{
    // int id1, id2;
    // id1 = Exec("cat");
    // id2 = Exec("copy");
    // Join(id1);
    // Join(id2);

    Test();
    PrintString(argv[1]);
    return 0;
}