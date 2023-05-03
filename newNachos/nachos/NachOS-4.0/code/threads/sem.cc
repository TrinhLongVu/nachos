#include "sem.h"
#include "synch.h"

Sem::Sem(char* na, int i) {
    strcpy(this->name, na);
    sem = new Semaphore(this->name, i);
}

Sem::~Sem() {
    if (sem) delete sem;
}

void Sem::wait() {sem->P(); } //downsem

void Sem::signal() { sem->V();} //upsem

char* Sem::GetName() { return this->name; }