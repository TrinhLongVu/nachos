// #include "synch.h"
#ifndef PCB_H
#define PCB_H

class PCB{
private:
    Semaphore *joinsem;
    Semaphore *exitsem;
    Semaphore *multex;

    int exitcode;
    int numwait;

    Thread *thread;
    char filename[128];
public:
    int parentID;
    int processID;
    PCB();
    PCB(int id);
    ~PCB();

    int Exec(char *filename, int pid);
    int ExecV(int argc, char** argv, int id);

    int GetID();
    int GetNumWait();

    void JoinWait();
    void ExitWait();
    void JoinRelease();
    void ExitRelease();

    void IncNumWait();
    void DecNumWait();

    void SetExitCode(int ec);
    int GetExitCode();

    void SetFileName(char *fn);
    char* GetFileName();
};

#endif