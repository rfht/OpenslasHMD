CFLAGS +=	-I/usr/include/SDL2
#LDFLAGS +=	-L/usr/lib/x86_64-linux-gnu \
#		-L/usr/local/lib \
LDFLAGS +=	-lGLEW -lGL -lopenhmd -lSDL2 -lm -ljson-c

all: OpenslasHMD

OpenslasHMD: gl.o maploader.o main.o
	$(CC) -o OpenslasHMD gl.o maploader.o main.o -lGLEW -lGL -lopenhmd -lSDL2 -lm -ljson-c -lSDL2_mixer

gl.o: gl.c gl.h
	$(CC) -c $(CFLAGS) gl.c

maploader.o: maploader.c maploader.h
	$(CC) -c $(CFLAGS) maploader.c

main.o: main.c gl.h maploader.h
	$(CC) -c $(CFLAGS) main.c

.PHONY: clean
clean:
	rm *.o OpenslasHMD
