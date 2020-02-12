BIN_DIR:=bin/
SRC_DIR:=src/
DEMO_DIR:=demo/

CC:=gcc
CFLAGS:=-I include -lpthread -Wall -g

SRC:=$(wildcard $(SRC_DIR)*.c) $(wildcard $(SRC_DIR)*.S)
OBJS_SRC:=$(patsubst $(SRC_DIR)%, $(BIN_DIR)%, $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(SRC))))

DEMO:=$(wildcard $(DEMO_DIR)*.c)
OBJS_DEMO:=$(patsubst $(DEMO_DIR)%, $(BIN_DIR)%, $(patsubst %.c, %.o, $(DEMO)))

all: $(BIN_DIR)Server $(BIN_DIR)Client $(BIN_DIR)httpserver
.PHONY:all

$(BIN_DIR)httpserver: $(OBJS_SRC) $(OBJS_DEMO)
	$(CC) -o $@ $(BIN_DIR)httpserver.o $(BIN_DIR)coroutine.o $(BIN_DIR)schedule.o $(BIN_DIR)coctx.o $(BIN_DIR)coswapctx.o $(BIN_DIR)co_api.o $(BIN_DIR)co_queue.o $(BIN_DIR)co_tree.o $(BIN_DIR)co_hash.o $(CFLAGS)

$(BIN_DIR)Server: $(OBJS_SRC) $(OBJS_DEMO)
	$(CC) -o $@ $(BIN_DIR)Server.o $(BIN_DIR)coroutine.o $(BIN_DIR)schedule.o $(BIN_DIR)coctx.o $(BIN_DIR)coswapctx.o $(BIN_DIR)co_api.o $(BIN_DIR)co_queue.o $(BIN_DIR)co_tree.o $(BIN_DIR)co_hash.o $(CFLAGS)

$(BIN_DIR)Client: $(BIN_DIR)Client.o
	$(CC) -o $@ $^

$(BIN_DIR)%.o: $(SRC_DIR)%.c
	$(CC) -o $@ -c $^ $(CFLAGS)

$(BIN_DIR)%.o: $(SRC_DIR)%.S
	$(CC) -o $@ -c $^ $(CFLAGS)

$(BIN_DIR)%.o: $(DEMO_DIR)%.c
	$(CC) -o $@ -c $^ $(CFLAGS)

.PHONY:clean

clean:
	@rm -f $(BIN_DIR)*
