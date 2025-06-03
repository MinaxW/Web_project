CC     = gcc
CFLAGS = -Wall -Wextra -I./lib -I./input -I./cache
SRCS   = src/main.c mongoose/mongoose.c input/input.c src/cache/cache.c
OUT    = server

.PHONY: all run clean

all: $(OUT)

$(OUT): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
