.PHNOY: all clean
all:hello
hello: tes.o te.o
	gcc -o hello tes.o te.o
tes.o:tes.c
	gcc -c tes.c
te.o:te.c
	gcc -c te.c
clean:
	rm -f tes.o te.o hello
