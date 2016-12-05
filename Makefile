CXX = -g -Wall -Wextra
LDFLAGS = -lm
CC = gcc
CLIENT = client
SERVER = server
CLEANER = rm
CLEANER_FLAGS = -rf

all: clean $(CLIENT) $(SERVER)

$(CLIENT):
	$(CC) $(CXX) $(CLIENT).c -o $(CLIENT) $(LDFLAGS)

$(SERVER): 
	$(CC) $(CXX) $(SERVER).c -o $(SERVER)

clean:
	$(CLEANER) $(CLEANER_FLAGS) $(CLIENT) $(SERVER)
