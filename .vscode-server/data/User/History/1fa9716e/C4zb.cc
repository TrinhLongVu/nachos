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
#define MAX_CONTENT 1000

int fds[MAX_FDS]; // array of file descriptor tables
int num_fds = 0;  // current number of file descriptors in the array

void SysPrintChar(char character)
{
	kernel->synchConsoleOut->PutChar(character);
}

void SysPrintString(char *character)
{
	for (int i = 0; i < sizeof(character); i++)
	{
		kernel->synchConsoleOut->PutChar(character[i]);
	}
}

void SysPrintNum(int num)
{
	char ch[100];			// a character array to store the resulting string
	sprintf(ch, "%d", num); // convert num to char*
	SysPrintString(ch);
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

int SocketTCP()
{
	if (num_fds >= MAX_FDS)
	{
		kernel->machine->WriteRegister(2, -1);
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		kernel->machine->WriteRegister(2, -1);
	}
	fds[num_fds] = fd;
	kernel->machine->WriteRegister(2, num_fds);
	num_fds++;
	return fd;
}

int Connect()
{
	int socketid = kernel->machine->ReadRegister(4);
	int addIp = kernel->machine->ReadRegister(5);
	int ServerPort = kernel->machine->ReadRegister(6);

	DEBUG(dbgSys, "Create a file.");
	static char Ip[50];
	bzero(Ip, 50);
	readFromMem(Ip, 50, addIp);

	cout << endl
		 << fds[socketid] << endl
		 << Ip << endl
		 << ServerPort;

	int iRetval = -1;
	struct sockaddr_in remote = {0};
	remote.sin_addr.s_addr = inet_addr(Ip); // Local Host
	remote.sin_family = AF_INET;
	remote.sin_port = htons(ServerPort);
	iRetval = connect(fds[socketid], (struct sockaddr *)&remote, sizeof(struct sockaddr_in));

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
	if (setsockopt(fds[socketid], SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		//	printf("Time Out\n");
		kernel->machine->WriteRegister(2, -1);
	}
	shortRetval = send(fds[socketid], contentSend, sizeContent, 0);
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
	if (setsockopt(fds[socketid], SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv)) < 0)
	{
		// printf("Time Out\n");
		kernel->machine->WriteRegister(2, -1);
	}
	shortRetval = recv(fds[socketid], contentSend, sizeContent, 0);
	printf("Response %s\n", contentSend);
	kernel->machine->WriteRegister(2, shortRetval);
}

int i = 0;
OpenFile **fileid = new OpenFile *[20];

void Open()
{
	int AddrFileName = kernel->machine->ReadRegister(4);
	int type = kernel->machine->ReadRegister(5);

	static char fileName[MAX_OPEN_FILE_NAME];
	bzero(fileName, MAX_OPEN_FILE_NAME);
	readFromMem(fileName, MAX_OPEN_FILE_NAME, AddrFileName);

	fileid[i] = kernel->fileSystem->Open(fileName);
	if (fileid[i] != 0)
	{
		cout << "open success" << endl;
		kernel->machine->WriteRegister(2, i);
		i++;
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

	if (fileid[AddrFileName] != 0)
	{
		delete fileid[AddrFileName];
		fileid[AddrFileName] = NULL;
	}
	else
	{
		cout << "close file error" << endl;
	}
}

void write() 
{
	int idbuffer = kernel->machine->ReadRegister(4);
	int addContent = kernel->machine->ReadRegister(5);
	int openfileId = kernel->machine->ReadRegister(6);


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
		}

		case SC_Connect:
		{
			Connect();
		}

		case SC_Send:
		{
			Send();
		}

		case SC_Receive:
		{
			Receive();
		}

		case SC_Test:
		{

			break;
		}

		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

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
