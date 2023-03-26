#include "../userprog/syscall.h"

int main() {
    //Create("test.txt");

    OpenFileId id = Open("test.txt", 1);
    Write("1111akfaflj", 7, id);

    Halt();
    return 0;
}