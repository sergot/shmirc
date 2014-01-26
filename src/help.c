#include "help.h"
#include "msg.h"

void error(char *s) {
    perror(s);
    
    exit(EXIT_FAILURE);
}

char first_char(char *s) {
    return *s;
}

int find_space(char *s) {
    int i;
    for(i = 0; i < strlen(s); i++)
        if(s[i] == ' ')
            break;
    
    if(i == strlen(s))
        return -1;
    
    return i;
}

void remove_cmd(char *s) {
    int i = find_space(s);
    
    memmove(s, s + i + 1, strlen(s));
}

void first_word(char *s, char *dest) {
    int i = find_space(s);
    
    if(i == -1) return;
    
    strncpy(dest, s, i);
}

void get_msg(char *s, char *dest) {
    int i = find_space(s);
    
    strncpy(dest, s+i+1, MAX_MSG_LENGTH);
}

void cmd(char *s, char *cmd) {
    int len = find_space(s);
    if(len == -1)
        len = strlen(s);
    else
        len -= 1;
    
    strncpy(cmd, s + 1, len);
}

int getLine (char *prmpt, char *buff, size_t sz) {
    int ch, extra;

    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return IN_NO_INPUT;

    // if it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? IN_TOO_LONG : IN_OK;
    }

    // otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    
    return IN_OK;
}

// returns position of \n
int get_newline(char *buf, int size) {
   int i;
   for(i = 0; i < size; i++)
       if(buf[i] == '\n')
           return i;
   return -1;
}
