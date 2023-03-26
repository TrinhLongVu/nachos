#include "../userprog/syscall.h"

int main() {
    //Create("test.txt");

    Open("test.txt", 1);

    Halt();
    return 0;
}