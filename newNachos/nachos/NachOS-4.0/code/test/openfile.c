#include "syscall.h"

int main() {
   OpenFileId id1;
   OpenFileId id2;
   id1 = Open("test.txt", 1);
   id2 = Open("test1.txt", 1);

   Halt();
}