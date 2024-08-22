#ifndef STATES_H
#define STATES_H

typedef enum {
    NEW,
    READY,
    BLOCKED,
    EXEC,
    EXIT
} stateProcess;

typedef enum {
    FIFO,
    RR, // Round Robin
    VRR // Virtual Round Robin
} planification;

#endif /* STATES_H */
