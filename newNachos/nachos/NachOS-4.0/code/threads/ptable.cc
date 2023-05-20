#include "synch.h"
#include "ptable.h"

PTable::PTable(int size)
{
    int i;
    psize = size;
    for (i = 0; i < size; i++)
    {
        pcb[i] = NULL;
    }
    bmsem = new Semaphore("bmsem", 1);
    pcb[0] = new PCB(0);
    pcb[0]->parentID = -1;
}

PTable::~PTable()
{
    int i;
    for (i = 0; i < psize; i++)
    {
        if (!pcb[i])
            delete pcb[i];
    }
    delete bmsem;
}

int PTable::ExecUpdate(char *name)
{
    bmsem->P();
    if (name == NULL)
    {
        DEBUG(dbgSys, "\nPTable::Exec : Can't not execute name is NULL.\n");
        bmsem->V();
        return -1;
    }

    if (strcmp(name, "scheduler") == 0 || strcmp(name, kernel->currentThread->getName()) == 0)
    {
        DEBUG(dbgSys, "\nPTable::Exec : Can't not execute itself.\n");
        bmsem->V();
        return -1;
    }

    int index = this->GetFreeSlot();

    if (index < 0)
    {
        DEBUG(dbgSys, "\nPTable::Exec :There is no free slot.\n");
        bmsem->V();
        return -1;
    }

    pcb[index] = new PCB(index);
    pcb[index]->SetFileName(name);
    pcb[index]->parentID = kernel->currentThread->processID;
    int pid = pcb[index]->Exec(name, index);
    bmsem->V();
    return pid;
}

int PTable::ExecVUpdate(int argc, char **argv, int addr)
{
    bmsem->P();

    char* name = argv[0];
    if (name == NULL)
    {
        DEBUG(dbgSys, "\nPTable::Exec : Can't not execute name is NULL.\n");
        bmsem->V();
        return -1;
    }
    if (strcmp(name, "scheduler") == 0 || strcmp(name, kernel->currentThread->getName()) == 0)
    {
        DEBUG(dbgSys, "\nPTable::Exec : Can't not execute itself.\n");
        bmsem->V();
        return -1;
    }
    int index = this->GetFreeSlot();
    if (index < 0)
    {
        DEBUG(dbgSys, "\nPTable::Exec :There is no free slot.\n");
        bmsem->V();
        return -1;
    }
    pcb[index] = new PCB(index);    
    pcb[index]->SetFileName(name);
    pcb[index]->parentID = kernel->currentThread->processID;   
    int pid = pcb[index]->ExecV(argc, argv, index, addr);
    bmsem->V();
    return pid;
}

int PTable::ExitUpdate(int exitcode)
{
    int id = kernel->currentThread->processID;
    if (id == 0)
    {
        kernel->currentThread->FreeSpace();
        kernel->interrupt->Halt();
        return 0;
    }

    if (!IsExist(id))
    {
        DEBUG(dbgSys, "Process " << id << " is not exist.");
        return -1;
    }
    pcb[id]->SetExitCode(exitcode);
    pcb[pcb[id]->parentID]->DecNumWait();
    pcb[id]->JoinRelease();
    pcb[id]->ExitWait();

    Remove(id);
    return exitcode;
}

int PTable::JoinUpdate(int id)
{
    if (id < 0 || id >= psize || pcb[id] == NULL ||
        pcb[id]->parentID != kernel->currentThread->processID)
    {
        DEBUG(dbgSys, "\nPTable::Join : Can't not join.\n");
        return -1;
    }
    pcb[pcb[id]->parentID]->IncNumWait();
    pcb[id]->JoinWait();

    int exit_code = pcb[id]->GetExitCode();
    
    pcb[id]->ExitRelease();
    return exit_code;
}

int PTable::GetFreeSlot()
{
    int i;
    for (i = 0; i < psize; i++)
    {
        if (!pcb[i])
            return i;
    }
    return -1;
}

bool PTable::IsExist(int pid) { return pcb[pid] != NULL; }

void PTable::Remove(int pid)
{
    if (pcb[pid] != NULL)
    {
        delete pcb[pid];
        pcb[pid] = NULL;
    }
}

char *PTable::GetFileName(int id) { return pcb[id]->GetFileName(); }
