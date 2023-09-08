# implement
write syscall
1. Implement the int Create(char *name) system call. The createfile system call will use the Nachos Filesystem Object Instance to create a zero-length file. Remember, the filename exists in user space. This means the buffer that the user space pointer points to must be translated from user memory space to system memory space. The createfile system call returns 0 for successful completion, -1 for an error.
2. Implement the OpenFileID Open(char *name, int type) and int Close(OpenFileID id) system calls. The user program can open two types of “files”, files that can be read only and files that can be read and write. Each process will allocate a fixed size file descriptor table. For now, set this size to be 20 file descriptors. The first two file descriptors, 0 and 1, will be reserved for console input and console output respectively. The open file system call will be responsible for translating the user space buffers when necessary and allocating the appropriate kernel constructs. You will use the filesystem objects provided to you in the filesystem directory. (NOTE: We are using the FILESYSTEM_STUB code) The calls will use the Nachos Filesystem Object Instance to open and close files.
The Open system call returns the file descriptor id (OpenFileID == an integer number), or –1 if the call fails. Open can fail for everal reasons, such as trying to open a file or mailbox that does not exist or if there is not enough room in the file descriptor table. The type parameter will be set to 0 for a standard file and 1 for a read only file. If the type parameter is set to any other value, the system call should fail. The close system call will take a file descriptor as the parameter. The system call will return –1 on failure and 0 on success
3. Implement the int Read(char *buffer, int charcount, OpenFileID id) and int Write(char *buffer, int charcount, OpenFileID id) system calls. These system calls respectively read and write to a file descriptor ID. Remember, you must translate the character buffers appropriately and you must differentiate between console IO (OpenFileID 0 and 1) and File (any other valid OpenFileID).
In case the Console is a read and write console, using the SynchConsoleInput class and the SynchConsoleOutput class for reading and writing, you must ensure that the correct data is returned to the user. Reading and writing to the console will return the correct number of characters read or written, not the charcount. In case of read or write failure, return -1
4. Implement the int Seek(int pos, OpenFileID id) system call. Seek will move the file cursor to a specified location. The parameter pos will be the absolute character position within a file. If pos is a –1, the position will be moved to the end of file. The system call will return the actual file position upon success, -1 if the call fails. Seeks on console I/O will fail
Install system call int Remove(char *name). The Remove system call will use the Nachos FileSystem Object to delete the file. Note: need to check if the file is open or not before deleting.
5. Implement system call for Network operations
6. Implement system call int SocketTCP(). You build an array of file descriptor tables with the size of 20 file descriptors.
The system function call "SocketTCP" will return the file descriptor id (int is an integer), or -1 if there is an error.
7. Implement system call int Connect(int socketid, char *ip, int port). Connect to server according to IP and port information. Returns 0 if the connection is successful, and -1 if it fails
8. Implement system call int Send(int socketid, char *buffer, int len), int Receive(int socketid, char *buffer, int len). Send and receive data from socket.
- If successful, returns the number of bytes sent or received
- If connection is closed, return 0
- If it fails, return -1
9. Implement system call int Close(int socketid). Close socket with 0 for success, -1 for error
Advanced
As part 1 and part 2, there are 2 file descriptor table arrays. In this requirement, 
You are only allowed to use a single array for file and network management
Only use Read/Write system call for file system and network (it means that you don’t use Send/Receive  system call at all).
