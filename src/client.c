/* 
 * File:   main.c
 * Author: Filip Sergot
 *
 * Created on December 22, 2013, 11:48 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <fcntl.h>

#include <unistd.h>
#include <semaphore.h>

#include "msg.h"
#include "help.h"
#include "settings.h"

int main(int argc, char **argv) {
    int shmfd,
        shm_seg_size = sizeof(struct msg);
    struct msg *shm_msg;
    
    char buff[MAX_MSG_LENGTH];
    
    sem_t *semfd; // semaphore file descriptor
    
    // open named semaphore
    semfd = sem_open(SEM_PATH, O_RDWR, S_IRWXU | S_IRWXG, 0);
    if(semfd == SEM_FAILED)
        error("sem_open()");

    // open shared memory
    shmfd = shm_open(SHM_PATH, O_RDWR, S_IRWXU | S_IRWXG);
    if(shmfd < 0)
        error("shm_open()");

    // map shared memory to address space
    shm_msg = (struct msg *) mmap(NULL, shm_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(shm_msg == NULL)
        error("mmap()");
    
    //int command = 0; // bool; if input is a command
    
    pid_t pid;
    pid = getpid();
    
    pid_t fid = fork();
    if(fid > 0) {
        while(getLine("cmd> ", buff, sizeof(buff)) == IN_OK) {
            //command = 0;
            //printf("LOOP\n");
            while(shm_msg->read && shm_msg->read == '_') sleep(1);

            if(!shm_msg->read || shm_msg->read == '!') {
                sem_wait(semfd);
                //printf("ZAMYKAM\n");

                shm_msg->pid = pid;
                shm_msg->type = TYPE_CLIENT_MSG;

                shm_msg->read = '_';
                
                if(first_char(buff) == '/') {
                    //printf("TO JEST KURWA KOMENDA\n");
                    //command = 1;
                    cmd(buff, shm_msg->cmd);
                    //printf("cmd: %s\n", shm_msg->cmd);
                    
                    remove_cmd(buff);
                    //printf("buff: %s\n", buff);
                } else {
                    snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "msg");
                }
                
                snprintf(shm_msg->content, MAX_MSG_LENGTH, "%s", buff);
            }
        }
    } else {
        while(1) {
            if(shm_msg->read == '*') {
                if(strncmp("resp", shm_msg->cmd, 4) == 0 && shm_msg->type == TYPE_SERVER_MSG && shm_msg->pid == pid) {
                    //sem_wait(semfd);
                    printf("response: %s\n", shm_msg->content);
                    shm_msg->read = '!';

                    sem_post(semfd);
                    //printf("PODNOSZE\n");
                } else if(strncmp("msg", shm_msg->cmd, 3) == 0) {
                    if(shm_msg->pid != pid && shm_msg->read != '!') {
                        //sem_post(semfd);
                        //printf("PODNOSZE\n");
                        printf("%s\n", shm_msg->content);
                        shm_msg->read = '!';
                    }
                }
            }
        }
    }
    
    return EXIT_SUCCESS;
}