CXX = -g -Wall -Wextra
LDFLAGS = -lm
CC = gcc
CLIENT = client
SERVER = server
CLEANER = rm
CLEANER_FLAGS = -rf

all: clean $(CLIENT) $(SERVER)

$(CLIENT):
	$(CC) $(CXX) ftrans.c $(CLIENT).c -o $(CLIENT) $(LDFLAGS)

$(SERVER): 
	$(CC) $(CXX) ftrans.c $(SERVER).c -o $(SERVER) $(LDFLAGS)

clean:
	$(CLEANER) $(CLEANER_FLAGS) $(CLIENT) $(SERVER)
