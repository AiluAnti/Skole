## Author: Steffen Viken Valvaag <steffenv@cs.uit.no> 
LIST_SRC=linkedlist.c
MAP_SRC=hashmap.c
WORDFREQS_SRC=wordfreqs.c common.c $(LIST_SRC) $(MAP_SRC)
HEADERS=common.h list.h map.h

all: wordfreqs

wordfreqs: $(WORDFREQS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(WORDFREQS_SRC)

clean:
	rm -f *~ *.o *.exe wordfreqs
