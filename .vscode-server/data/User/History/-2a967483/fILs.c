#include "syscall.h"

int main() {
   OpenFileId id1 = Open("test1.txt", 1);
   OpenFileId id2 = Open("test2.txt", 1);

   Close(id1);

   
   Halt();
}