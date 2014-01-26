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
#include "user.h"

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
    
    pid_t pid;
    pid = getpid();
    
    user *usr = new_user(pid, "",""); // this user
    strncpy(usr->channel, "all", 3);;
    
    pid_t fid = fork();

    // input handling
    if(fid > 0) {
        // register client
        sem_wait(semfd);

        shm_msg->read = '_';
        shm_msg->pid = pid;
        shm_msg->type = TYPE_CLIENT_MSG;
        strncpy(shm_msg->cmd, "reg", MAX_CMD_LENGTH);

        sem_post(semfd);

        // get input line by line
        while(getLine("", buff, sizeof(buff)) == IN_OK) {
            while(shm_msg->read && shm_msg->read == '_') sleep(1);

            if(!shm_msg->read || shm_msg->read == '!') {
                sem_wait(semfd);

                shm_msg->pid = pid;
                shm_msg->type = TYPE_CLIENT_MSG;

                shm_msg->read = '_';
                
                if(first_char(buff) == '/') {
                    cmd(buff, shm_msg->cmd);
                    remove_cmd(buff);
                } else {
                    snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "msg");
                }
                
                snprintf(shm_msg->content, MAX_MSG_LENGTH, "%s", buff);
            }
        }
    // reponse handling
    } else {
        while(1) {
            fflush(stdout); fflush(stdin);
            if(shm_msg->read == '*') {
                if(strncmp("resp", shm_msg->cmd, 4) == 0 && shm_msg->pid == pid && shm_msg->type == TYPE_SERVER_MSG) {
                    printf("\n");
                    printf("response: %s\n", shm_msg->content);
                    
                    shm_msg->read = '!';

                    char CMD[MAX_MSG_LENGTH];
                    cmd(shm_msg->content, CMD);
                    remove_cmd(shm_msg->content);

                    if(strncmp(CMD, "join", 4) == 0) {
                        strncpy(usr->channel, shm_msg->content, MAX_CHAN_LEN);
                    } else if(strncmp(CMD, "name", 4) == 0) {
                        strncpy(usr->name, shm_msg->content, MAX_NAME_LEN);
                    }
                    
                    sem_post(semfd);
                } else if(strncmp("pm", shm_msg->cmd, 2) == 0 && shm_msg->pid == pid && shm_msg->type == TYPE_CLIENT_MSG) {
                    printf("[PM -> %d] %s: %s\n", shm_msg->pid, shm_msg->from, shm_msg->content);
                    shm_msg->read = '!';
                } else if(strncmp("msg", shm_msg->cmd, 3) == 0) {
                    if((strncmp(shm_msg->channel, usr->channel, MAX_CHAN_LEN) == 0) && shm_msg->pid != pid && shm_msg->read != '!') {
                        printf("[#%s] <%s> ", shm_msg->channel, shm_msg->from);
                        printf("%s\n", shm_msg->content);
                        shm_msg->read = '!';
                    }
                }
            }
        }
    }
    
    return EXIT_SUCCESS;
}
