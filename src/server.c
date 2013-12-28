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
#include "user.h"

// clears memory
static void clear();

typedef struct ch {
    char name[MAX_CHAN_LEN];
    struct ch *next;
} chann;

int add_channel(char *name, chann *first_channel) {
    int i = 1;
    
    chann *new_chan = malloc(sizeof(chann));
    strncpy(new_chan->name, name, MAX_CHAN_LEN);
    new_chan->next = NULL;
    
    chann *ch = first_channel;
    while(ch->next != NULL) {
        i++;
        ch = ch->next;
    }

    ch->next = new_chan;
    
    return i;
}

int chan_exists(char *name, chann *first_channel) {
    chann *u = first_channel;
    
    while(u != NULL) {
        if(strncmp(u->name, name, MAX_CHAN_LEN) == 0)
            return 1;
        
        u = u->next;
    }
    
    return 0;
}

int main(int argc, char **argv) {
    int shmfd, // shared memory file descriptor
        shm_seg_size = sizeof(struct msg); // max shm space size
    
    struct msg *shm_msg; // message "msg.h"
    
    sem_t *semfd; // semaphore file descriptor
    
    user *first_user = NULL;
    chann *first_channel = NULL;
    
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
        
    int command;
    while(1) {
        fflush(stdin); fflush(stdout);
        command = 0;
        if(shm_msg->read && shm_msg->read == '_') { // msg from client
            user *u = find_user(shm_msg->pid, first_user);
            if(u == NULL) {
                u = new_user(shm_msg->pid, "", shm_msg->channel);
                if(first_user == NULL)
                    first_user = u;
                else
                    add_user(u, first_user);
                
                snprintf(u->name, MAX_NAME_LEN, "%d", u->pid);
            }
            
            printf("read: %s from %s in %s|\n", shm_msg->content, u->name, u->channel);
            printf("cmd: |%s|\n", shm_msg->cmd);
            
            shm_msg->read = '*';
            
            if(strncmp("msg", shm_msg->cmd, 3) != 0) {
                shm_msg->type = TYPE_SERVER_MSG;
                command = 1;
                
                if(strncmp(shm_msg->cmd, "join", 4) == 0) {
                    char chan[MAX_CHAN_LEN];
                    strncpy(chan, shm_msg->content, MAX_CHAN_LEN);
                    strncpy(u->channel, chan, MAX_CHAN_LEN);
                    if(!chan_exists(chan, first_channel)) {
                        if(first_channel == NULL) {
                            chann *new_chan = malloc(sizeof(chann));
                            strncpy(new_chan->name, chan, MAX_CHAN_LEN);
                            new_chan->next = NULL;

                            first_channel = new_chan;
                        } else {
                            add_channel(chan, first_channel);
                        }
                    }
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "/join %s", chan);
                } else if(strncmp(shm_msg->cmd, "name", 4) == 0) {
                    char name[MAX_NAME_LEN];
                    strncpy(name, shm_msg->content, MAX_NAME_LEN);
                    strncpy(u->name, name, MAX_NAME_LEN);
                    
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "/name %s", name);
                } /*else if(strncmp(shm_msg->cmd, "pm", 2) == 0) {
                    char lol[MAX_NAME_LEN];
                    
                    char b[MAX_NAME_LEN];
                    strncpy(b, shm_msg->content, MAX_NAME_LEN);
                    printf("[????] %s, %s\n", b, lol);
                    first_word(b, lol);
                    
                    get_msg(shm_msg->content, b);
                                        
                    char name[MAX_NAME_LEN];
                    strncpy(name, get_user_name(shm_msg->pid, first_user), MAX_NAME_LEN);
                    shm_msg->pid = get_user_pid(lol, first_user);
                    
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "/pm %s %s", name, b);
                } */ 
                else if(strncmp(shm_msg->cmd, "info", 4) == 0) {
                    strncpy(shm_msg->content, "type /stats to get server statistics", MAX_MSG_LENGTH);
                } else if(strncmp(shm_msg->cmd, "chans", 5) == 0) {
                    chann *ch = first_channel;
                    while(ch != NULL) {
                        printf("%s\n", ch->name);
                        
                        ch = ch->next;
                    }
                } else if(strncmp(shm_msg->cmd, "users", 5) == 0) {
                    user *u = first_user;
                    
                    while(u != NULL) {
                        printf("%s\n", u->name);
                        
                        u = u->next;
                    }
                } else if(strncmp(shm_msg->cmd, "users", 4) == 0) {
                    strncpy(shm_msg->content, "type /stats to get server statistics", MAX_MSG_LENGTH);
                }
                
                snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "resp");
            } else {
                strncpy(shm_msg->channel, u->channel, MAX_CHAN_LEN);
                shm_msg->type = TYPE_CLIENT_MSG;
                snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "msg");
                
                strncpy(shm_msg->channel, get_user_channel(shm_msg->pid, first_user), MAX_CHAN_LEN);
                printf("CHAN: %s\n\n", shm_msg->channel);
                
                char conc[MAX_MSG_LENGTH];
                snprintf(conc, MAX_MSG_LENGTH, "<%s> %s", u->name, shm_msg->content);
                strncpy(shm_msg->content, conc, MAX_MSG_LENGTH);
            }
            
            if(command == 0) {
                sem_post(semfd);
            }
            print_users(first_user);
            printf("--------\n");
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