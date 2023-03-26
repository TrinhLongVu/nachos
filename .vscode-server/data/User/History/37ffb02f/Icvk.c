#include "../userprog/syscall.h"

int main() {
    //Create("test.txt");

    Open("text.txt", 1);

    Halt();
    return 0;
}