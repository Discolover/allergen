vector_test: vector_test.c vector.o
	cc -g -W -o vector_test vector_test.c vector.o

vector.o: vector.c vector.h
	cc -W -c vector.c
clean:
	rm -fv vector_test vector.o
