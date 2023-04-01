/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

bool readFromMem(char* buffer, int size, int virAddr) {
    int idx = 0;
    // memcpy(buffer, kernel->machine->mainMemory + virAddr, size);
    int data = 0;
    while (size >= 4) {
        if (!kernel->machine->ReadMem(virAddr + idx, 4, &data))
            return false;
        memcpy(buffer + idx, &data, 4);
        size -= 4;
        idx += 4;
    }
    while (size >= 2) {
        if (!kernel->machine->ReadMem(virAddr + idx, 2, &data))
            return false;
        memcpy(buffer + idx, &data, 2);
        size -= 2;
        idx += 2;
    }
    while (size >= 1) {
        if (!kernel->machine->ReadMem(virAddr + idx, 1, &data))
            return false;
        memcpy(buffer + idx, &data, 1);
        size -= 1;
        idx += 1;
    }
    return true;
}

bool writeToMem(char* buffer, int size, int virAddr) {
    int count = 0;
    // memcpy(kernel->machine->mainMemory + virAddr, buffer, size);
    int data = 0;
    while (size >= 4) {
        memcpy(&data, buffer + count, 4);
        if (!kernel->machine->WriteMem(virAddr + count, 4, data))
            return false;
        size -= 4;
        count += 4;
    }
    while (size >= 2) {
        memcpy(&data, buffer + count, 2);
        if (!kernel->machine->WriteMem(virAddr + count, 2, data))
            return false;
        size -= 2;
        count += 2;
    }
    while (size >= 1) {
        memcpy(&data, buffer + count, 1);
        if (!kernel->machine->WriteMem(virAddr + count, 1, data))
            return false;
        size -= 1;
        count += 1;
    }
    delete buffer;
    return true;
}
#endif /* ! __USERPROG_KSYSCALL_H__ */
