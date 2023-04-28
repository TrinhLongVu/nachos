#ifndef SEM_H
#define SEM_H

class Semaphore;

class Sem{
private:
    char name[50];
    Semaphore *sem;
public:
    Sem(char* na, int i);
    ~Sem();

    void wait();

    void signal();

    char* GetName();
};

#endif