CC=gcc
LDFLAGS=-lrt -pthread
SOURCES=src/help.c
SOURCES_SERVER=src/server.c
SOURCES_CLIENT=src/client.c
HEADERS=src/help.h \
		src/msg.h \
		src/settings.h
OBJ_DIR=obj
SRC_DIR=src
SERVER=shm_server
CLIENT=shm_client

all: server client
server:
	$(CC) $(SOURCES_SERVER) $(SOURCES) $(HEADERS) $(LDFLAGS) -o $(SERVER)
client:
	$(CC) $(SOURCES_CLIENT) $(SOURCES) $(HEADERS) $(LDFLAGS) -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
