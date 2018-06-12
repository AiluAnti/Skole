## Author: Steffen Viken Valvaag <steffenv@cs.uit.no> 
LIST_SRC=  # Insert the file name of your list implementation here
SORTWORDS_SRC=sortwords.c common.c $(LIST_SRC)
REVERSEWORDS_SRC=reversewords.c common.c $(LIST_SRC)
HEADERS=common.h list.h

all: sortwords reversewords

sortwords: $(SORTWORDS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(SORTWORDS_SRC)

reversewords: $(REVERSEWORDS_SRC) $(HEADERS) Makefile
	gcc -o $@ $(REVERSEWORDS_SRC)

clean:
	rm -f *~ *.o *.exe sortwords reversewords
