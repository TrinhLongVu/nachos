#include "syscall.h"
#include "copyright.h"
#define maxlen 32
int main()
{
    int len;
    char filename[maxlen + 1];
    Create("test.txt");
    Halt();
}