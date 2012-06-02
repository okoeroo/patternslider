RM = /bin/rm
CAT = /bin/cat
# CC = gcc
CC = clang

CFLAGS = \
    -g -W -Wall -pedantic -Wbad-function-cast -Wcast-align \
    -Wcast-qual -Wchar-subscripts -Winline \
    -Wmissing-prototypes -Wnested-externs -Wpointer-arith \
    -Wredundant-decls -Wshadow -Wstrict-prototypes \
    -Wpointer-arith -Wno-long-long \
    -Wcomment -Wmissing-include-dirs \
    -O -Wuninitialized \
    -D_FILE_OFFSET_BITS=64

CPPFLAGS = \
	-Iinclude/ \
	-I. \

BIN   =  ./bin/test

OBJS  = \
	src/main.o


all: $(BIN)
#	$(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(OBJS) $(LFLAGS)

# $(OBJS): 
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(RM) -f $(BIN) $(OBJS) *~

distclean: clean
