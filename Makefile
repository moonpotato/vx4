CC = gcc
CFLAGS = -std=gnu11 -Wall -g

LIBS = -ldl

EXEC = run.out
HEADERS = $(*.h)
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(EXEC)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(EXEC) $(OBJECTS)

