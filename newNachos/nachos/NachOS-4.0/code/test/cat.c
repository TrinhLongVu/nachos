#include "../userprog/syscall.h"


int main()
{
    char *buffer;
    OpenFileId id = Open("test.txt", 1);
    Read(buffer,10,id);
    //Test(buffer);
    //PrintString(buffer);
    Close(id);
    Halt();

    return 0;
}