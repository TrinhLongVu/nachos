#include "syscall.h"

int main()
{
    SpaceId id1, id2;
    id1 = Exec("cat");
    id2 = Exec("copy");
    Join(id1);
    Join(id2);
}
