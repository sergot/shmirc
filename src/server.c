/* 
 * File:   main.c
 * Author: Filip Sergot
 *
 * Created on December 22, 2013, 11:29 PM
 */

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

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
#include "file.h"

// clears memory
static void clear(int);

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

stats *st = NULL;

int main(int argc, char **argv) {
    // signal handling
    if (signal(SIGINT, clear) == SIG_ERR)
        error("signal()");

    if (signal(SIGABRT, clear) == SIG_ERR)
        error("signal()");

    int shmfd, // shared memory file descriptor
        shm_seg_size = sizeof(struct msg); // max shm space size
    
    struct msg *shm_msg; // message "msg.h"
    
    sem_t *semfd; // semaphore file descriptor
    
    user *first_user = NULL;
    chann *first_channel = NULL;

    char *resp = NULL;
    
    // open named semaphore
    semfd = sem_open(SEM_PATH, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, 1);
    if(semfd == SEM_FAILED)
        error("sem_open()");
    
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

    // read stats
    st = calloc(1, sizeof(stats));
    read_stats("stats", st);

    // add default channel
    chann *new_chan = malloc(sizeof(chann));
    strncpy(new_chan->name, "all", MAX_CHAN_LEN);
    new_chan->next = NULL;
    first_channel = new_chan;

    int command;
    while(1) {
        fflush(stdin); fflush(stdout);

        command = 0;
        if(shm_msg->read && shm_msg->read == '_') { // msg from client
            user *u = find_user(shm_msg->pid, first_user);
            if(u == NULL) {
                u = new_user(shm_msg->pid, "", "all");
                st->users++;
                if(first_user == NULL)
                    first_user = u;
                else
                    add_user(u, first_user);
                
                snprintf(u->name, MAX_NAME_LEN, "%d", u->pid);
            }
            
            printf("\n");
            //printf("read: %s from %s in %s|\n", shm_msg->content, u->name, u->channel);
            //printf("cmd: |%s|\n", shm_msg->cmd);

            strncpy(shm_msg->from, u->name, MAX_NAME_LEN);
            
            shm_msg->read = '*';
            if(strncmp("reg", shm_msg->cmd, 3) == 0) {
                shm_msg->read = '!';
            } else if(strncmp("msg", shm_msg->cmd, 3) != 0) {
                shm_msg->type = TYPE_SERVER_MSG;
                command = 1;
                
                if(strncmp(shm_msg->cmd, "join", 4) == 0) {
                    char chan[MAX_CHAN_LEN];
                    strncpy(chan, shm_msg->content, MAX_CHAN_LEN);
                    strncpy(u->channel, chan, MAX_CHAN_LEN);
                    if(!chan_exists(chan, first_channel)) {
                        st->channels++;
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
                } else if(strncmp(shm_msg->cmd, "pm", 2) == 0) {
                    char *name_snd = calloc(MAX_NAME_LEN + 1, sizeof(char));
                    
                    char msg_recv[MAX_MSG_LENGTH];
                    strncpy(msg_recv, shm_msg->content, MAX_MSG_LENGTH);
                    first_word(msg_recv, name_snd);

                    get_msg(shm_msg->content, msg_recv);
                                        
                    char name_recv[MAX_NAME_LEN];
                    strncpy(name_recv, get_user_name(shm_msg->pid, first_user), MAX_NAME_LEN);
                    shm_msg->pid = get_user_pid(name_snd, first_user);
                    
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "%s", msg_recv);

                    shm_msg->type = TYPE_CLIENT_MSG;
                    if(find_user(shm_msg->pid, first_user) == NULL)
                        shm_msg->read = '!';

                    shm_msg->pid = get_user_pid(name_snd, first_user);
                    free(name_snd);
                } else if(strncmp(shm_msg->cmd, "info", 4) == 0) {
                    strncpy(shm_msg->content, "type /stats to get server statistics", MAX_MSG_LENGTH);
                } else if(strncmp(shm_msg->cmd, "stats", 5) == 0) {
                    snprintf(shm_msg->content, MAX_MSG_LENGTH, "channels: %d, users: %d, messages: %d", st->channels, st->users, st->messages);
                } else if(strncmp(shm_msg->cmd, "chans", 5) == 0) {
                    resp = calloc(MAX_MSG_LENGTH, sizeof(char));

                    chann *ch = first_channel;
                    if(ch != NULL) {
                        snprintf(resp, MAX_MSG_LENGTH, "%s", ch->name);
                        ch = ch->next;
                    }

                    while(ch != NULL) {
                        char *tmp = calloc(MAX_MSG_LENGTH, sizeof(char));
                        snprintf(tmp, MAX_MSG_LENGTH, "%s, %s", resp, ch->name);
                        snprintf(resp, MAX_MSG_LENGTH, "%s", tmp);

                        if(strlen(resp) >= MAX_MSG_LENGTH)
                            break;
                        
                        ch = ch->next;
                        free(tmp);
                    }

                    strncpy(shm_msg->content, resp, MAX_MSG_LENGTH);
                    free(resp);
                } else if(strncmp(shm_msg->cmd, "users", 5) == 0) {
                    resp = calloc(MAX_MSG_LENGTH, sizeof(char));

                    user *u = first_user;
                    if(u != NULL) {
                        snprintf(resp, MAX_MSG_LENGTH, "%s", u->name);
                        u = u->next;
                    }

                    while(u != NULL) {
                        char *tmp = calloc(MAX_MSG_LENGTH, sizeof(char));
                        snprintf(tmp, MAX_MSG_LENGTH, "%s, %s", resp, u->name);
                        snprintf(resp, MAX_MSG_LENGTH, "%s", tmp);

                        if(strlen(resp) >= MAX_MSG_LENGTH)
                            break;
                        
                        u = u->next;
                        free(tmp);
                    }

                    strncpy(shm_msg->content, resp, MAX_MSG_LENGTH);
                    free(resp);
                } else {
                    strncpy(shm_msg->content, "WRONG COMMAND! available commands: /info, /chans, /users, /join <channel>, /name <newname>, /pm <username> <msg>.", MAX_MSG_LENGTH);
                }
                
                if(strncmp("pm", shm_msg->cmd, 2) != 0)
                    snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "resp");
            } else {
                st->messages++;
                strncpy(shm_msg->channel, u->channel, MAX_CHAN_LEN);
                shm_msg->type = TYPE_CLIENT_MSG;
                snprintf(shm_msg->cmd, MAX_CMD_LENGTH, "msg");
                
                strncpy(shm_msg->channel, get_user_channel(shm_msg->pid, first_user), MAX_CHAN_LEN);

                if(count_users_on_channel(shm_msg->channel, first_user) < 2)
                    shm_msg->read = '!';
            }
            
            if(command == 0) {
                sem_post(semfd);
            }
        }
    }
    
    clear(0);
    
    return EXIT_SUCCESS;
}

static void clear(int sig) {
    // remove shared memory file
    if(shm_unlink(SHM_PATH) != 0)
        error("shm_unlink()");
    
    // remove named semaphore
    if(sem_unlink(SEM_PATH) != 0)
        error("sem_unlink()");
    
    write_stats("stats", st);

    error("Bye! ");
}
