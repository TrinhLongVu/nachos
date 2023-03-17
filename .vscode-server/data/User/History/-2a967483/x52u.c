#include "syscall.h"

int main() {
   Open("test1.txt", 1);
   Open("test2.txt", 1);
   Halt();
}