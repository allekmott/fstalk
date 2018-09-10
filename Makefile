CC=gcc
CFLAGS=-Wall -std=c89 -MD

EXE=fstalk
DEPS=
OBJS=main.o

.PHONY: all
all: $(EXE)

%.o: $.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: rebuild
rebuild: clean all

.PHONY: clean
clean:
	rm -f $(EXE)
	rm -f *.o
	rm -f *.d
	rm -f *~


