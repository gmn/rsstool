###########################################################################
#
# $Makefile - 04/09/13 - Greg Naughton$
#
# Makefile for RSS Power Tool
#
###########################################################################

EXE_NAME = rss
CC = g++
CFLAGS_DBG = -g -Wall -D_DEBUG
O = obj
DO = dbgobj
LIBS=-L/usr/lib/x86_64-linux-gnu -lcurl -lsqlite3 -lncurses 
DBG=-D_DEBUG
CFLAGS=-O2 -Wall


#ifeq ($(MAKECMDGOALS),opt) 
#    override CFLAGS=$(CFLAGS_OPT)
#endif
#ifeq ($(MAKECMDGOALS),install) 
#    override CFLAGS=$(CFLAGS_OPT)
#endif


OBJS = $(O)/main.o \
	$(O)/curseview.o \
	$(O)/tinyxml2.o \
	$(O)/dba_sqlite.o \
	$(O)/tokenizer.o \
	$(O)/unicode.o \
	$(O)/sha1.o \
	$(O)/misc.o  \
	$(O)/html_entities.o \
    $(O)/item_result.o

DBGOBJS = $(DO)/main.o \
	$(DO)/curseview.o \
	$(DO)/tinyxml2.o \
	$(DO)/dba_sqlite.o \
	$(DO)/tokenizer.o \
	$(DO)/unicode.o \
	$(DO)/sha1.o \
	$(DO)/misc.o  \
	$(DO)/html_entities.o \
    $(DO)/item_result.o

all: $(EXE_NAME)

$(EXE_NAME): dirs $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXE_NAME) $(LIBS)
	strip $(EXE_NAME)

debug: dirs $(DBGOBJS)
	$(CC) $(CFLAGS_DBG) $(DBGOBJS) -o $(EXE_NAME) $(LIBS)

dbg: debug

# rule for each object
# must be in uniform directories to work
$(O)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(DO)/%.o: %.cpp
	$(CC) $(CFLAGS_DBG) -c $< -o $@


dirs:
	@if [ ! -d $(O) ]; then rm -rf $(O); mkdir $(O); fi
	@if [ ! -d $(DO) ]; then rm -rf $(DO); mkdir $(DO); fi

clean:
	rm -rf $(O) $(DO)
	rm -f $(EXE_NAME) 
	[ ! -e $(EXE_NAME).dSYM ] || rm -rf $(EXE_NAME).dSYM
	[ ! -e a.out ] || rm -f a.out
	[ ! -e a.out.dSYM ] || rm -rf a.out.dSYM

install: $(EXE_NAME)
	install -m 0755 $(EXE_NAME) /usr/bin/

uninstall:
	rm /usr/bin/$(EXE_NAME)
