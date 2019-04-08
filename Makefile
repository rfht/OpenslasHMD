CFLAGS +=	-I/usr/include/SDL2 -g
#LDFLAGS +=	-L/usr/lib/x86_64-linux-gnu \
#		-L/usr/local/lib \
LDFLAGS +=	-lGLEW -lGL -lopenhmd -lSDL2 -lm -ljson-c -g

all: OpenslasHMD

OpenslasHMD: gl.o resourceloader.o maploader.o main.o
	$(CC) -o OpenslasHMD gl.o resourceloader.o maploader.o main.o -lGLEW -lGL -lopenhmd -lSDL2 -lm -ljson-c -lSDL2_mixer

gl.o: gl.c gl.h
	$(CC) -c $(CFLAGS) gl.c

resourceloader.o: resourceloader.c resourceloader.h cgltf.h
	$(CC) -c $(CFLAGS) resourceloader.c

maploader.o: maploader.c maploader.h
	$(CC) -c $(CFLAGS) maploader.c

main.o: main.c gl.h maploader.h resourceloader.h
	$(CC) -c $(CFLAGS) main.c

.PHONY: clean
clean:
	rm *.o OpenslasHMD
