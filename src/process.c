/*********************************************************************/
/*                                                                   */
/*  This Program Written by Alica Okano.                             */
/*  Released to the Public Domain as discussed here:                 */
/*  http://creativecommons.org/publicdomain/zero/1.0/                */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  process.c - process management                                   */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "process.h"
#include "helper.h"

#ifndef __32BIT__
#include "support.h"
#endif

/* initial process; beginning of process control block chain */
static PDOS_PROCESS *initProc = NULL;
/* PCB of process running right now */
PDOS_PROCESS *curPCB = NULL;

/* initPCB - Initialize process control block.
 * Sets the magic, flags, etc.
 */
void initPCB(PDOS_PROCESS *pcb, unsigned long pid, char *prog, char *envPtr)
{
    char *tmp;

    /* Skip any drive or directory in the program name */
    for (;;)
    {
        tmp = strchr(prog,':');
        if (tmp == NULL)
            tmp = strchr(prog,'\\');
        if (tmp == NULL)
            tmp = strchr(prog,'/');
        if (tmp == NULL)
            break;
        prog = tmp+1;
        continue;
    }

    /* Clear the PCB before using it.
     * This will set all fields to 0 except those we set explicitly below.
     */
    memset(pcb, 0, PDOS_PROCESS_SIZE);

    /* Set the PCB magic. This helps identify PCBs in memory */
    pcb->magic[0] = 'P';
    pcb->magic[1] = 'C';
    pcb->magic[2] = 'B';
    pcb->magic[3] = 0;

    /* Set the exe name */
    strncpy(pcb->exeName, prog, PDOS_PROCESS_EXENAMESZ-1);
    upper_str(pcb->exeName);

    /* Set the PID */
    pcb->pid = pid;

    /* Set the parent */
    pcb->parent = curPCB;

    /* Set the environment block */
    pcb->envBlock = envPtr;
}

/* Process has terminated, remove it from chain */
void removeFromProcessChain(PDOS_PROCESS *pcb)
{
    PDOS_PROCESS *cur;
    PDOS_PROCESS *prev = pcb->prev;
    PDOS_PROCESS *next = pcb->next;

    /* We don't support removing the init process, it can't terminate. */
    if (prev == NULL)
        return;

    /* Patch this PCB out of the chain. */
    prev->next = next;
    if (next != NULL)
        next->prev = prev;
    pcb->next = NULL;
    pcb->prev = NULL;

    /* Walk through chain, any of our surviving children are reparented
       to the init process. */
    cur = initProc;
    while (cur != NULL)
    {
        if (cur->parent == pcb)
        {
            cur->parent = initProc;
        }
        cur = cur->next;
    }
}

/* Add some new process to process chain */
void addToProcessChain(PDOS_PROCESS *pcb)
{
    PDOS_PROCESS *last = NULL;

    /* This is first process ever, it starts the chain. */
    if (initProc == NULL)
    {
        initProc = pcb;
        return;
    }

    /* Find last process in chain, this process goes in end. */
    last = initProc;
    while (last->next != NULL)
    {
        last = last->next;
    }

    /* We have last process, stick new process after it. */
    last->next = pcb;
    pcb->prev = last;
}

/* Find process with given PID */
PDOS_PROCESS *findProc(unsigned long pid)
{
    PDOS_PROCESS *cur = initProc;

    if (pid == 0)
        return cur;
    while (cur != NULL)
    {
        if (cur->pid == pid)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/* Is there a process control block just before this pointer? */
int isProcessPtr(void *ptr)
{
    unsigned long abs;
    PDOS_PROCESS *p;
    if (ptr == NULL)
        return 0;
#if !defined(__32BIT__) && !defined(__SMALLERC__) && !defined(__SZ4__)
    abs = ADDR2ABS(ptr);
    abs -= PDOS_PROCESS_SIZE;
    ptr = FP_NORM(ABS2ADDR(abs));
    p = (PDOS_PROCESS*)ptr;
#else
    p = (PDOS_PROCESS*)(((char *)ptr) - PDOS_PROCESS_SIZE);
#endif
    return p->magic[0] == 'P' &&
           p->magic[1] == 'C' &&
           p->magic[2] == 'B' &&
           p->magic[3] == 0;
}
