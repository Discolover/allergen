bitset_test: bitset_test.c debug_bitset.o bitset.o
	cc -g -W -o bitset_test bitset_test.c debug_bitset.o

bitset.o: bitset.c bitset.h
	cc -g -W -c bitset.c

debug_bitset.o: bitset.c bitset.h
	cc -D DEBUG -o debug_bitset.o -g -W -c bitset.c

clean:
	rm -v bitset_test *.o

install: bitset.o
	mv -v bitset.o ${HOME}/.local/lib/libbitset.a && \
	cp -v bitset.h ${HOME}/.local/include/
