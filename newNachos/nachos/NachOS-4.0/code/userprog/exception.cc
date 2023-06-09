// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>  // For strlen
#include "openfile.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------
#define MAX_FDS 20
#define MAX_OPEN_FILE_NAME 25
#define MAX_CONTENT 100

#define ONLY_READ 0
#define READ_WRITE 1

#define SOCKET 10
#define OPENFILE 20

int fds[MAX_FDS];
int num_fds = 0;
int i = 0;
OpenFile **fileid = new OpenFile *[20];

struct table
{
	int socket;
	OpenFile *Fileid;
	int typeFile;
	int flag;
};

table list[20];
int posfree = 0;

void countPosFree()
{
	for (int i = 0; i < 20; i++)
	{
		if (list[posfree].socket || list[posfree].Fileid)
		{
			posfree++;
		}
	}
}

void SysPrintChar(char character)
{
	kernel->synchConsoleOut->PutChar(character);
}

void SysPrintString()
{
	int idbuffer = kernel->machine->ReadRegister(4);

	char *content = new char[MAX_OPEN_FILE_NAME];
	bzero(content, MAX_OPEN_FILE_NAME);
	readFromMem(content, MAX_OPEN_FILE_NAME, idbuffer);

	for (int i = 0; i < strlen(content); i++)
	{
		kernel->synchConsoleOut->PutChar(content[i]);
	}
	kernel->synchConsoleOut->PutChar('\n');
}

void SysPrintNum(int num)
{
	char ch[100];			// a character array to store the resulting string
	sprintf(ch, "%d", num); // convert num to char*
	for (int i = 0; i < sizeof(ch); i++)
	{
		kernel->synchConsoleOut->PutChar(ch[i]);
	}
}

void MoveProgramCounter()
{
	/* set previous programm counter (debugging only)
	 * similar to: registers[PrevPCReg] = registers[PCReg];*/
	kernel->machine->WriteRegister(PrevPCReg,
								   kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction
	 * similar to: registers[PCReg] = registers[NextPCReg]*/
	kernel->machine->WriteRegister(PCReg,
								   kernel->machine->ReadRegister(NextPCReg));

	/* set next programm counter for brach execution
	 * similar to: registers[NextPCReg] = pcAfter;*/
	kernel->machine->WriteRegister(
		NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}

int SocketTCP()
{
	if (num_fds >= MAX_FDS)
	{
		kernel->machine->WriteRegister(2, -1);
		return -1;
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		kernel->machine->WriteRegister(2, -1);
		return -1;
	}
	list[posfree].socket = fd;
	list[posfree].flag = SOCKET;

	kernel->machine->WriteRegister(2, posfree);
	countPosFree();

	return fd;
}

int Connect()
{
	int socketid = kernel->machine->ReadRegister(4);
	int addIp = kernel->machine->ReadRegister(5);
	int ServerPort = kernel->machine->ReadRegister(6);

	static char Ip[50];
	bzero(Ip, 50);
	readFromMem(Ip, 50, addIp);

	int iRetval = -1;
	struct sockaddr_in remote = {0};
	remote.sin_addr.s_addr = inet_addr(Ip);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(ServerPort);
	iRetval = connect(list[socketid].socket, (struct sockaddr *)&remote, sizeof(struct sockaddr_in));

	if (iRetval < 0)
	{
		cout << "\nConnection Failed \n";
	}

	kernel->machine->WriteRegister(2, iRetval);
	return iRetval;
}

void Send()
{
	int socketid = kernel->machine->ReadRegister(4);
	int addContent = kernel->machine->ReadRegister(5);
	int sizeContent = kernel->machine->ReadRegister(6);

	static char contentSend[MAX_CONTENT];
	bzero(contentSend, MAX_CONTENT);
	readFromMem(contentSend, MAX_CONTENT, addContent);

	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20; /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if (setsockopt(list[socketid].socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		kernel->machine->WriteRegister(2, -1);
	}
	shortRetval = send(list[socketid].socket, contentSend, sizeContent, 0);
	kernel->machine->WriteRegister(2, shortRetval);
}

void Receive()
{
	int socketid = kernel->machine->ReadRegister(4);
	int addContent = kernel->machine->ReadRegister(5);
	int sizeContent = kernel->machine->ReadRegister(6);

	static char contentSend[MAX_CONTENT];
	bzero(contentSend, MAX_CONTENT);
	readFromMem(contentSend, MAX_CONTENT, addContent);

	int shortRetval = -1;
	struct timeval tv;
	tv.tv_sec = 20; /* 20 Secs Timeout */
	tv.tv_usec = 0;
	if (setsockopt(list[socketid].socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		kernel->machine->WriteRegister(2, -1);
	}
	shortRetval = recv(list[socketid].socket, contentSend, sizeContent, 0);
	/////////////////////////////////////////////

	kernel->machine->WriteRegister(2, shortRetval);
}

void CreateFile()
{
	DEBUG(dbgSys, "Create a file.");
	static char fileName[MAX_OPEN_FILE_NAME];
	bzero(fileName, MAX_OPEN_FILE_NAME);
	int virAddr = kernel->machine->ReadRegister(4);
	readFromMem(fileName, MAX_OPEN_FILE_NAME, virAddr);

	if (kernel->fileSystem->Create(fileName))
		kernel->machine->WriteRegister(2, 0);
	else
		kernel->machine->WriteRegister(2, -1);
}

void Open()
{
	int AddrFileName = kernel->machine->ReadRegister(4);
	int typeFile = kernel->machine->ReadRegister(5);

	static char fileName[MAX_OPEN_FILE_NAME];
	bzero(fileName, MAX_OPEN_FILE_NAME);
	readFromMem(fileName, MAX_OPEN_FILE_NAME, AddrFileName);

	list[posfree].Fileid = kernel->fileSystem->Open(fileName);
	list[posfree].typeFile = typeFile;
	list[posfree].flag = OPENFILE;
	if (list[posfree].Fileid && posfree < 20)
	{
		kernel->machine->WriteRegister(2, posfree);
		countPosFree();
	}
	else
	{
		cout << "open error: cannot open file" << endl;
		kernel->machine->WriteRegister(2, -1);
	}
}

void Close()
{
	int AddrFileName = kernel->machine->ReadRegister(4);
	if (list[AddrFileName].flag == OPENFILE)
	{
		if (list[AddrFileName].Fileid != 0)
		{
			delete list[AddrFileName].Fileid;
			list[AddrFileName].Fileid = NULL;
		}
		else
		{
			cout << "close file error" << endl;
		}
	}
	else
	{
		cout << "it's not file, can't close !!!"
			 << "\n";
	}
}

void Read()
{

	int idbuffer = kernel->machine->ReadRegister(4);
	int charcount = kernel->machine->ReadRegister(5);
	int ID = kernel->machine->ReadRegister(6);
	if (list[ID].flag == OPENFILE)
	{
		char *buffer = new char[MAX_OPEN_FILE_NAME];
		bzero(buffer, MAX_OPEN_FILE_NAME);
		readFromMem(buffer, MAX_OPEN_FILE_NAME, idbuffer);

		if (list[ID].Fileid != 0 && list[ID].typeFile == READ_WRITE)
		{
			int result = list[ID].Fileid->Read(buffer, charcount);

			kernel->machine->WriteRegister(2, result);
			writeToMem(buffer, MAX_OPEN_FILE_NAME, idbuffer);
		}
		else
		{
			kernel->machine->WriteRegister(2, -1);
		}
	}
	else
	{
		static char contentSend[MAX_CONTENT];
		bzero(contentSend, MAX_CONTENT);
		readFromMem(contentSend, MAX_CONTENT, idbuffer);

		int shortRetval = -1;
		struct timeval tv;
		tv.tv_sec = 20; /* 20 Secs Timeout */
		tv.tv_usec = 0;
		if (setsockopt(list[ID].socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
		{
			kernel->machine->WriteRegister(2, -1);
		}
		shortRetval = send(list[ID].socket, contentSend, charcount, 0);
		kernel->machine->WriteRegister(2, shortRetval);
	}
}

void write()
{
	int idbuffer = kernel->machine->ReadRegister(4);
	int charcount = kernel->machine->ReadRegister(5);
	int ID = kernel->machine->ReadRegister(6);

	if (list[ID].flag == OPENFILE)
	{
		char *buffer = new char[MAX_OPEN_FILE_NAME];
		bzero(buffer, MAX_OPEN_FILE_NAME);
		readFromMem(buffer, MAX_OPEN_FILE_NAME, idbuffer);

		if (list[ID].Fileid != 0 && list[ID].typeFile == READ_WRITE)
		{
			int result = list[ID].Fileid->Write(buffer, charcount);
			kernel->machine->WriteRegister(2, result);
		}
		else
		{
			kernel->machine->WriteRegister(2, -1);
		}
	}
	else
	{
		char *contentSend = new char[MAX_CONTENT];
		bzero(contentSend, MAX_CONTENT);
		readFromMem(contentSend, MAX_CONTENT, idbuffer);

		int shortRetval = -1;
		struct timeval tv;
		tv.tv_sec = 20; /* 20 Secs Timeout */
		tv.tv_usec = 0;
		if (setsockopt(list[ID].socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0)
		{
			kernel->machine->WriteRegister(2, -1);
		}
		shortRetval = recv(list[ID].socket, contentSend, charcount, 0);

		writeToMem(contentSend, MAX_CONTENT, idbuffer);
		kernel->machine->WriteRegister(2, shortRetval);
	}
}

void seek()
{
	int offset = kernel->machine->ReadRegister(4);
	int openfileId = kernel->machine->ReadRegister(5);
	if (list[openfileId].Fileid != 0 && list[openfileId].typeFile == READ_WRITE)
	{

		int result = list[openfileId].Fileid->getCurOffset();
		if (offset == result)
		{
			kernel->machine->WriteRegister(2, result);
		}
		else
		{
			kernel->machine->WriteRegister(2, -1);
		}
	}
}

void Remove()
{
	DEBUG(dbgSys, "Create a file.");
	static char fileName[MAX_OPEN_FILE_NAME];
	bzero(fileName, MAX_OPEN_FILE_NAME);
	int virAddr = kernel->machine->ReadRegister(4);
	if (readFromMem(fileName, MAX_OPEN_FILE_NAME, virAddr) && kernel->fileSystem->Remove(fileName))
		kernel->machine->WriteRegister(2, 0);
	else
		kernel->machine->WriteRegister(2, -1);
}

void Exec()
{
	int virtAddr = kernel->machine->ReadRegister(4);
	char *name = new char[50];
	bzero(name, 50);
	readFromMem(name, 50, virtAddr);

	if (name == NULL)
	{
		DEBUG(dbgSys, "\n Not enough memory in System");
		ASSERT(false);
		kernel->machine->WriteRegister(2, -1);
		return;
	}

	OpenFile *file = kernel->fileSystem->Open(name);
	if (file == NULL)
	{
		DEBUG(dbgSys, "\nExec:: Can't open this file.");
		kernel->machine->WriteRegister(2, -1);
	}
	delete file;

	// Return child process id
	int pid = kernel->pTab->ExecUpdate(name);
	kernel->machine->WriteRegister(2, pid);

	return;
}

void Join()
{
	int id = kernel->machine->ReadRegister(4);
	kernel->machine->WriteRegister(2, kernel->pTab->JoinUpdate(id));
}

void Exit()
{
	int id = kernel->machine->ReadRegister(4);
	kernel->machine->WriteRegister(2, kernel->pTab->ExitUpdate(id));
}

void CreateSemaphore()
{
	int virtAddr = kernel->machine->ReadRegister(4);
	int semVal = kernel->machine->ReadRegister(5);
	char *name = new char[50];
	bzero(name, 50);
	readFromMem(name, 50, virtAddr);
	// cout << name;
	if (name == NULL)
	{
		DEBUG(dbgSys, "\n Not enough memory in System");
		ASSERT(false);
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
	}
	kernel->machine->WriteRegister(2, SysCreateSemaphore(name, semVal));
	delete[] name;
}

void Wait()
{
	int virtAddr = kernel->machine->ReadRegister(4);

	char *name = new char[50];
	bzero(name, 50);
	readFromMem(name, 50, virtAddr);
	if (name == NULL)
	{
		DEBUG(dbgSys, "\n Not enough memory in System");
		ASSERT(false);
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
	}

	kernel->machine->WriteRegister(2, SysWait(name));
	delete[] name;
}

void Signal()
{
	int virtAddr = kernel->machine->ReadRegister(4);

	char *name = new char[50];
	bzero(name, 50);
	readFromMem(name, 50, virtAddr);
	if (name == NULL)
	{
		DEBUG(dbgSys, "\n Not enough memory in System");
		ASSERT(false);
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
	}

	kernel->machine->WriteRegister(2, SysSignal(name));
	delete[] name;
}

void PrintChar()
{
	int virtAddr = kernel->machine->ReadRegister(4);
}

void Argc()
{
	kernel->machine->WriteRegister(2, kernel->currentThread->getArgc);
	cout << kernel->currentThread->getArgc;
}

int storeMem(char** argv, int argc) {
    if (argv == NULL || argc == 0)
        return 0;
    
    int vir_stack = kernel->machine->ReadRegister(StackReg);
    
    // Allocate memory for the array of pointers
    int pointersSize = argc * sizeof(char*);
    char** argvCopy = new char*[argc];

    // Write the array of pointers to memory
    kernel->machine->WriteMem(vir_stack, pointersSize, reinterpret_cast<int>(argvCopy));

    // Write each string to memory and update the pointers
    int stringStartAddr = vir_stack + pointersSize;
    for (int i = 0; i < argc; i++) {
        const char* str = argv[i];
        int strLength = strlen(str) + 1;  // Include null terminator

        // Allocate memory for the string and copy it to memory
        kernel->machine->WriteMem(vir_stack + i * sizeof(char*), sizeof(char*), stringStartAddr);
        kernel->machine->WriteMem(stringStartAddr, strLength, reinterpret_cast<int>(str));

        // Update the pointer in argvCopy
        argvCopy[i] = reinterpret_cast<char*>(stringStartAddr);

        // Move the stringStartAddr to the next string
        stringStartAddr += strLength;
    }

    // Cleanup: delete temporary arrays
    delete[] argvCopy;

    return vir_stack;
}

void Argv()
{
	char **argv = kernel->currentThread->getArgv;
	int stack = storeMem(kernel->currentThread->getArgv, kernel->currentThread->getArgc);
	kernel->machine->WriteRegister(2, stack);
}


void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{

		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;
		}

		case SC_Create:
		{
			CreateFile();
			break;
		}

		case SC_Open:
		{
			Open();
			break;
		}

		case SC_Close:
		{
			Close();
			break;
		}

		case SC_Remove:
		{
			Remove();
			break;
		}

		case SC_SocketTCP:
		{
			SocketTCP();
			break;
		}

		case SC_Connect:
		{
			Connect();
			break;
		}

		case SC_Send:
		{
			Send();
			break;
		}

		case SC_Receive:
		{
			Receive();
			break;
		}

		case SC_Write:
		{
			write();
			break;
		}

		case SC_PrintString:
		{
			SysPrintString();
			break;
		}

		case SC_Read:
		{
			Read();
			break;
		}

		case SC_Test:
		{
			int argc = kernel->machine->ReadRegister(4);
			int virtAddrArgv = kernel->machine->ReadRegister(5);
			// Allocate memory for argv
			char **argv = new char *[argc + 1];
			for (int i = 0; i <= argc; i++)
			{
				argv[i] = new char[20];
			}

			// Read argv from user memory
			for (int i = 0; i < argc; i++)
			{
				int virtAddrArgv_i;
				if (!kernel->machine->ReadMem(virtAddrArgv + i * sizeof(int), sizeof(int), &virtAddrArgv_i))
				{
					DEBUG(dbgSys, "\nExecV:: Error reading argv from user memory.");
					for (int j = 0; j <= argc; j++)
					{
						delete[] argv[j];
					}
					delete[] argv;
					kernel->machine->WriteRegister(2, -1);
					return;
				}
				if (!readFromMem(argv[i], 20, virtAddrArgv_i))
				{
					DEBUG(dbgSys, "\nExecV:: Error reading argv from user memory.");
					for (int j = 0; j <= argc; j++)
					{
						delete[] argv[j];
					}
					delete[] argv;
					kernel->machine->WriteRegister(2, -1);
					return;
				}
			}
			argv[argc] = NULL; // argv list must end with NULL

			OpenFile *file = kernel->fileSystem->Open(argv[0]);
			if (file == NULL)
			{
				DEBUG(dbgSys, "\nExec:: Can't open this file.");
				kernel->machine->WriteRegister(2, -1);
			}
			delete file;

			int pid = kernel->pTab->ExecVUpdate(argc, argv, virtAddrArgv);

			kernel->machine->WriteRegister(2, pid);

			break;
		}

		case SC_getArgv:
		{
			Argv();
			break;
		}

		case SC_getArgc:
		{
			Argc();
			break;
		}

		case SC_Exec:
		{
			Exec();
			break;
		}

		case SC_Join:
		{
			Join();
			break;
		}

		case SC_Exit:
		{
			Exit();
			break;
		}
		case SC_CreateSemaphore:
		{
			CreateSemaphore();
			break;
		}
		case SC_Wait:
		{
			Wait();
			break;
		}
		case SC_Signal:
		{
			Signal();
			break;
		}

		case SC_PrintChar:
		{
			char character = (char)kernel->machine->ReadRegister(4);
			SysPrintChar(character);
			break;
		}

		default:
		{
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		}
		break;
	default:
	{
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	}
	MoveProgramCounter();
	// ASSERTNOTREACHED();
}
