
all: count compare parse

CXXFLAGS=-Wall -g3 -O2 -MMD

# Force everything to rebuild every time.
.PHONY: compare count clean tar

count: reduced.c boot pairtest
	@./pairtest
	@echo -n "Byte count is "
	@tr '\n' ' ' < reduced.c|sed 's/ //g'|wc -c
	@./boot

# full.c is the verbose compilable C source.
full.c: pair.c pure.c
	cat pair.c pure.c > full.c

# reduced.c is the final compressed C source.
reduced.c: full.c pure.pl
	perl pure.pl <full.c >reduced.c

# Check that full and reduced compile to identical binaries.  This works on
# Linux i386 with gcc-2.95.4.  Your milage may vary.
compare: full reduced
	@cmp full reduced && echo "full and reduced match :-)"

full: full.c
	gcc -s -o full full.c

reduced: reduced.c
	gcc -s -o reduced reduced.c

# We add -Wno-unused to avoid warnings from the ',' expressions in pure.c.
tree.o: tree.cc
	g++ ${CXXFLAGS} -Wno-unused  -c -o tree.o tree.cc

boot: boot.cc
	g++ ${CXXFLAGS} -o boot boot.cc

parse: parse.o tree.o bitstream.o
	g++ ${CXXFLAGS} -o parse parse.o tree.o bitstream.o

pairtest: pair.c

clean:
	rm -f *.o *.d *.s *~ reduced full parse pairtest boot full.c reduced.c

tar: busy.tar.gz

busy.tar.gz: Makefile README.txt *.c *.cc *.hh
	tar cfz busy.tar.gz $^

-include *.d
