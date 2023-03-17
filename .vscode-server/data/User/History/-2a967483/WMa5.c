#include "syscall.h"

int main() {
   int id1 = Open("test1.txt", 1);
   int id2 = Open("test2.txt", 1);

   Close(id1);
   Halt();
}