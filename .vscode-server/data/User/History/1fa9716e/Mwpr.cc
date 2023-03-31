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
		if (list[posfree].socket && list[posfree].Fileid)
		{
			posfree++;
		}
	}
}

void SysPrintChar(char character)
{
	kernel->synchConsoleOut->PutChar(character);
}

void SysPrintString(char *content)
{
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
	posfree++;

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
	remote.sin_addr.s_addr = inet_addr("127.0.0.1");
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
	SysPrintString(contentSend);
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
		cout << "open success" << endl;
		kernel->machine->WriteRegister(2, i);
		posfree++;
	}
	else
	{
		cout << "open error: cannot file" << endl;
		kernel->machine->WriteRegister(2, -1);
	}
}

void Close()
{
	int AddrFileName = kernel->machine->ReadRegister(4);

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
		static char contentSend[MAX_CONTENT];
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
		/////////////////////////////////////////////
		SysPrintString(contentSend);
		kernel->machine->WriteRegister(2, shortRetval);
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

		case SC_Test:
		{
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
