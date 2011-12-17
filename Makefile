
CC=g++
CFLAGS=-Wall
SOURCES=	main.cpp \
		ImgGlitch.h \
		ImgGlitch.cpp 
LIBS=-ltiff

all: 
	${CC} ${CFLAGS} ${LIBS} ${SOURCES} -o ImgGlitch

clean:
	rm -rf *.o ImgGlitch
