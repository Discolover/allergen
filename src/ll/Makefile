ll: ll.c lex.yy.o
	cc -D DEBUG -g -W -o ll -I "${HOME}/.local/include" ll.c utils.c \
	lex.yy.o -L "${HOME}/.local/lib" -lvector -lbitset

lex.yy.o: lex.yy
	flex lex.yy && \
	cc -o lex.yy.o -W -c lex.yy.c

clean:
	rm -v ll lex.yy.o lex.yy.c
