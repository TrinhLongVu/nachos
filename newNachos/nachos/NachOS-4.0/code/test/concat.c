#include "../userprog/syscall.h"

int main(){
    OpenFileId id1; 
    OpenFileId id2; 
    OpenFileId id3;
    char* buffer1;
    char* buffer2; 
  
    id1 = Open("test.txt",1);
    id2 = Open("test1.txt",1);
    id3 = Open("test3.txt" ,1 );

    Read(buffer1 ,10 ,id1);
    Write(buffer1,10, id3);
   
    Read(buffer2, 10, id2);
    
    Write(buffer2, 10 ,id3);
    Close(id1);
    Close(id2);
    Close(id3);


    Halt();
    return 0;
}