#include "../userprog/syscall.h"

int main() {
    Remove("test.txt");
    
    Halt();
    return 0;
}