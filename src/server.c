/* 
 * File:   main.c
 * Author: Filip Sergot
 *
 * Created on December 22, 2013, 11:29 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <semaphore.h>

#include <fcntl.h>

#include <unistd.h>

#include <time.h>

#include <string.h>

#include "msg.h"
#include "help.h"
#include "settings.h"

// clears memory
static void clear();

int main(int argc, char **argv) {
    int shmfd, // shared memory file descriptor
        shm_seg_size = sizeof(struct msg); // max shm space size
    
    struct msg *shm_msg; // message "msg.h"
    
    sem_t *semfd; // semaphore file descriptor
    
    
    // open named semaphore
    semfd = sem_open(SEM_PATH, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 1);
    if(semfd == SEM_FAILED)
        error("sem_open()");
    
    // weird?
    sem_post(semfd);
    
    // open named shared memory
    shmfd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
    if(shmfd < 0)
        error("shm_open()");
    
    // set size of shared memory
    ftruncate(shmfd, shm_seg_size);
    
    // map shared memory to address space
    shm_msg = (struct msg *) mmap(NULL, shm_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(shm_msg == NULL)
            error("mmap()");
        
    //int i;
    int command;
    while(1) {//while(scanf("%d", &i) == 1) {
        command = 0;
        if(shm_msg->read && shm_msg->read == '_') { // msg from client
            printf("read: %s from %d\n", shm_msg->content, shm_msg->pid);
            printf("cmd: |%s|\n", shm_msg->cmd);
            
            shm_msg->read = '*';
            
            if(strncmp("msg", shm_msg->cmd, 3) != 0) {
                shm_msg->type = TYPE_SERVER_MSG;
                command = 1;
                
                //printf("DOSTALEM KOMENDE\n");
                if(strncmp(shm_msg->cmd, "join", 4) == 0) {
                    //printf("TO JE JOIN!\n");
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "dolaczyles do kanalu");
                }
                
                snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "resp");
            } else {
                shm_msg->type = TYPE_CLIENT_MSG;
                snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "msg");
            }
            
            if(command == 0) {
                sem_post(semfd);
                //printf("PODNOSZE\n");
            }
        }
    }
    
    clear();
    
    return EXIT_SUCCESS;
}

static void clear() {
    // remove shared memory file
    if(shm_unlink(SHM_PATH) != 0)
        error("shm_unlink()");
    
    // remove named semaphore
    if(sem_unlink(SEM_PATH) != 0)
        error("sem_unlink()");
    
    // TODO
    
}