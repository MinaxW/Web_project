CC     = gcc
CFLAGS = -Wall -Wextra -I./lib -I./input
SRCS   = main.c mongoose/mongoose.c input/input.c
OUT    = server

.PHONY: all run clean

all: $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
