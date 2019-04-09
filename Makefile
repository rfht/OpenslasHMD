LDFLAGS +=	-lGLEW -lGL -lSDL2 -lSDL2_mixer -ljson-c -lm -lopenhmd

all: OpenslasHMD

# TODO: add resourceloader.o in here later when merged
OpenslasHMD: gl.o maploader.o main.o
	$(CC) -o OpenslasHMD gl.o maploader.o main.o $(LDFLAGS)

gl.o: gl.c gl.h
	$(CC) -c $(CFLAGS) gl.c

# for later when branch has been merged
#resourceloader.o: resourceloader.c resourceloader.h cgltf.h
#	$(CC) -c $(CFLAGS) resourceloader.c

maploader.o: maploader.c maploader.h
	$(CC) -c $(CFLAGS) maploader.c

main.o: main.c gl.h maploader.h # resourceloader.h
	$(CC) -c $(CFLAGS) main.c

.PHONY: clean
clean:
	rm *.o OpenslasHMD
