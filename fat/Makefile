
fat-objs := fs.o bootsec.o charset.o entry.o

CC := gcc

.PHONY: all
all: ${fat-objs} lib

lib: ${fat-objs}
	${CC} -fPIC -shared -o libfat.so ${fat-objs}

clean:
	rm ${fat-objs}
	rm libfat.so

fs.o: fs.c fs.h
bootsec.o: bootsec.c bootsec.h
charset.o: charset.c charset.h
