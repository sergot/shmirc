#include "file.h"

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

void write_stats(char *filename, stats *s) {
    int fd = open(filename, O_WRONLY | O_TRUNC);

    char chs[10], uss[10], msgs[10];
    snprintf(chs, 10, "%d", s->channels);
    snprintf(uss, 10, "%d", s->users);
    snprintf(msgs, 10, "%d", s->messages);

    write(fd, chs, strlen(chs));

    write(fd, "\n", 1);

    write(fd, uss, strlen(uss));

    write(fd, "\n", 1);

    write(fd, msgs, strlen(msgs));

    write(fd, "\n\n", 2);
    

    close(fd);
}

void read_stats(char *filename, stats *s) {
    int fd = open(filename, O_RDONLY), r;
    int line = 1, i, pos, pos2;
    char file_line[MAX_LINE_LEN];

    char buf[MAX_LINE_LEN];


    while(1) {
        r = read(fd, buf, MAX_LINE_LEN);
        if(r == 0) {
            break;
        }
        else if(r == -1) {
            break;
        } else {
            for(i = 0; i < r; i++) {
                if(buf[i] == '\n') {
                    if(line == 1) {
                        strncpy(file_line, buf, i);
                        printf("FILELINE: %s | ", file_line);
                        s->channels = atoi(file_line);
                        printf("ATOI: %d\n", s->channels);
                        pos = i;
                    } else if(line == 2) {
                        strncpy(file_line, buf+pos, i);
                        printf("FILELINE: %s | ", file_line);
                        s->users = atoi(file_line);
                        printf("ATOI: %d\n", s->users);
                        pos = i;
                    } else if(line == 3) {
                        strncpy(file_line, buf+pos, i);
                        printf("FILELINE: %s | ", file_line);
                        s->messages = atoi(file_line);
                        printf("ATOI: %d\n", s->messages);
                    }
                    line++;
                    continue;
                }
            }
        }
    }

    close(fd);
}
