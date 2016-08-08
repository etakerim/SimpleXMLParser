#Vzorový makefile
#dalsie vlajky: -g -03  -Os -save-temps -fmudflap 
#-march=native (-S -fverbose-asm) -s(strip) -Werror
#clang, -fdump-<ipa>-all(tree,ipa,rtl)

CC = gcc
CFLAGS = -c -O2 -std=c99 -Wall -Wextra -pedantic #-g 
INCLUDES = -I../include/
LDFLAGS =
SOURCES = main.c xmlparser.c bstrlib.c vector.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ../bin/program

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) ${INCLUDES} $< -o $@

clean:
	-rm -f *.o core *.core
