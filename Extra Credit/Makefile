CFLAGS=-W -Wall -g -pedantic -std=gnu99

file_crawler: fileCrawler.o libADTS.a
	gcc $(CFLAGS) -pthread -o file_crawler fileCrawler.o libADTS.a

libADTS.a: iterator.o linkedlist.o treeset.o re.o
	rm -f libADTS.a
	ar r libADTS.a iterator.o linkedlist.o treeset.o re.o
	ranlib libADTS.a

clean:
	rm -f *.o libADTS.a fileCrawler

iterator.o: tsiterator.c tsiterator.h
linkedlist.o: tslinkedlist.c tslinkedlist.h tsiterator.h
treeset.o: tstreeset.c tstreeset.h iterator.h
re.o: re.c re.h
fileCrawler.o: fileCrawler.c tsiterator.h tslinkedlist.h tstreeset.h re.h
