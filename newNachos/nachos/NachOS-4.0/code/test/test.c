#include "../userprog/syscall.h"

int main()
{
    char* argv1[] = {"createfile", "hello"};
    char* argv2[] = {"cat", "hi"};
    SpaceId t1, t2;
    t1 = Test(2, argv1);
    // t2 = Test(2, argv2);
    // t2 = Exec("cat");
    
    Join(t1);
    // Join(t2);
    Exit(0);
}