#include "../userprog/syscall.h"

int main(){
    OpenFileId id1; 
    OpenFileId id2;
    char* buffer; 
    id1 =  Open("test.txt",1);
   
    Read(buffer,10,id1);
    PrintString(buffer);
    id2 = Open("test1.txt",1);
    Write(buffer, 10, id2);
     //PrintString(buffer);
    Close(id1);
    Close(id2);
    Halt();
    return 0;
}