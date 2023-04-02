#include "../userprog/syscall.h"

int main(){
    int fd1 = SocketTCP();
    OpenFileId id1;
    OpenFileId id2;   
    char *buffer[100];
    char* buffer2[100];
    id2 = Open("test4.txt" ,1);
    id1 =  Open("test.txt",1);
    Read(buffer,100 ,id1);

    Connect(fd1, "127.0.0.1", 12345);
    Read(buffer, 100, fd1);
    Write(buffer2, 100, fd1);
    Write(buffer2,100, id2);

    Close(id2);
    Close(id1);
   


    Halt();
    return 0;
}