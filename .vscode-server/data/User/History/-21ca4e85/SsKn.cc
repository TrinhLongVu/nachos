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
char *User2System(int virtAddr, int limit)
{
	int i; // chi so index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void move_program_counter()
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

void handle_SC_RandomNum()
{
	int result;
	result = SysRandomNum();
	kernel->machine->WriteRegister(2, result);
	move_program_counter();
	return;
}

void
ExceptionHandler(ExceptionType which) {
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which) {
		case SyscallException:
			switch (type) {
				case SC_Halt:
					DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

					SysHalt();

					ASSERTNOTREACHED();
					break;

				case SC_Create: {
					DEBUG(dbgSys, "Create a file.");
					static char fileName[MAX_OPEN_FILE_NAME];
					bzero(fileName, MAX_OPEN_FILE_NAME);
					int virAddr = kernel->machine->ReadRegister(4);
					if (readFromMem(fileName, MAX_OPEN_FILE_NAME, virAddr) && kernel->fileSystem->Create(fileName))
						kernel->machine->WriteRegister(2, 0);
					else
						kernel->machine->WriteRegister(2, -1);
					break;
				}

				case SC_Open: {
					DEBUG(dbgSys, "Open an existing file.");
					static char fileName[MAX_OPEN_FILE_NAME];
					bzero(fileName, MAX_OPEN_FILE_NAME);
					int virAddr = kernel->machine->ReadRegister(4);
					if (readFromMem(fileName, MAX_OPEN_FILE_NAME, virAddr)) {
						FILE* file = fopen(fileName, "r+");
						if (file) {
							kernel->machine->WriteRegister(2, (int)file);
							break;
						}
					}
					cout << "Fail to open file " << fileName << "." << endl;
					kernel->machine->WriteRegister(2, -1);
					break;
				}

				case SC_Close: {
					DEBUG(dbgSys, "Close file.");
					int fileId = kernel->machine->ReadRegister(4);
					if (fclose((FILE*)fileId) == 0)
						kernel->machine->WriteRegister(2, 0);
					else
						kernel->machine->WriteRegister(2, -1);
					break;
				}

				case SC_Write: {
					DEBUG(dbgSys, "Write to file.");
					int address = kernel->machine->ReadRegister(4);
					int size = kernel->machine->ReadRegister(5);
					OpenFileId f_id = kernel->machine->ReadRegister(6);
					char* data = new char[size];
					bzero(data, size);
					if (readFromMem(data, size, address) && fwrite(data, 1, size, (FILE*)f_id) > 0) {

						DEBUG(dbgSys, "Write successfully.");
						kernel->machine->WriteRegister(2, 0);
					}
					else {
						DEBUG(dbgSys, "Write error.");
						kernel->machine->WriteRegister(2, -1);
					}
					delete[] data;
					break;
				}
				case SC_ConsoleReadLine: {
					DEBUG(dbgSys, "Read a line from console into a char array.");
					int virAddr = kernel->machine->ReadRegister(4);
					int maxSize = kernel->machine->ReadRegister(5);
					char* temp = new char[maxSize] {};
					cin.getline(temp, maxSize, '\n');
					if (writeToMem(temp, strlen(temp), virAddr))
						kernel->machine->WriteRegister(2, cin.gcount());
					else
						kernel->machine->WriteRegister(2, -1);
					delete[] temp;
					break;
				}
				case SC_ConsoleWrite: {
					DEBUG(dbgSys, "Write char array to console.");
					int virAddr = kernel->machine->ReadRegister(4);
					int size = kernel->machine->ReadRegister(5);
					char* temp = new char[size] {};
					if (readFromMem(temp, size, virAddr)) {
						cout.write(temp, size);
						kernel->machine->WriteRegister(2, 0);
					}
					else
						kernel->machine->WriteRegister(2, -1);
					delete[] temp;
					break;
				}
				case SC_ConsoleRead: {
					DEBUG(dbgSys, "Read input to char array.\n");
					int buffer = kernel->machine->ReadRegister(4);
					int size = kernel->machine->ReadRegister(5);
					char* temp = new char[size];
					bzero(temp, size);
					int count = fread(temp, 1, size, stdin);
					cin.get();
					if (writeToMem(temp, size, buffer)) {
						kernel->machine->WriteRegister(2, count);
					}
					else
						kernel->machine->WriteRegister(2, -1);

					delete[] temp;
					break;
				}
				case SC_ConsoleWriteLine: {
					DEBUG(dbgSys, "Write char array on a line.");
					int vAddr = kernel->machine->ReadRegister(4);
					int size = kernel->machine->ReadRegister(5);
					char* temp = new char[size];
					if (readFromMem(temp, size, vAddr)) {
						int count = fwrite(temp, 1, size, stdout);
						cout << endl;
						kernel->machine->WriteRegister(2, count+1);
					}
					else
						kernel->machine->WriteRegister(2, -1);
					delete[] temp;
					break;
				}
				case SC_Add:
					DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

					/* Process SysAdd Systemcall*/
					int result;
					result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
						/* int op2 */(int)kernel->machine->ReadRegister(5));
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

				default:
					cerr << "Unexpected system call " << type << "\n";
					break;
			}
			break;
		default:
			cerr << "Unexpected user mode exception" << (int)which << "\n";
			break;
	}

	// advance program counter register
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(NextPCReg));
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
	// ASSERTNOTREACHED();
}
