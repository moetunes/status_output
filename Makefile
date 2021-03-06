CFLAGS+= -g -std=c99 -pedantic -Wall
LDADD+= -lX11 -lprocps -liw
LDFLAGS=
EXEC=snap_output

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

SRC = status.c
OBJ = ${SRC:.c=.o}

all: $(EXEC)
${EXEC}: ${OBJ}

	$(CC) $(LDFLAGS) -s -O2 -ffast-math -fno-unit-at-a-time -o $@ ${OBJ} $(LDADD)

install: all
	install -Dm 755 snap_output $(DESTDIR)$(BINDIR)/$(EXEC)

clean:
	rm -fv $(EXEC) *.o

