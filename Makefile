#DEBUG=-g
DEBUG=-DNDEBUG

#SMALLTYPE=-DWINDOWED
SMALLTYPE=-DBLINKENLICHTS -DWINDOWED

CFLAGS=`pkg-config --cflags sdl` -Os $(DEBUG) $(SMALLTYPE) -ffast-math -fomit-frame-pointer -march=i686 -Wall -Wextra -std=c99
LDFLAGS=-lGL -Os $(DEBUG) -march=native -Wall -Wextra -std=c99
OBJECTS=main.o scene.o

all: latheific latheific-strip latheific-pack

latheific: $(OBJECTS)
	$(CC) $^ `pkg-config --libs sdl` $(LDFLAGS) -o $@

latheific-small: smallmain.o scene.o
	$(LD) -dynamic-linker /lib/ld-linux.so.2 $^ /usr/lib/libSDL.so /usr/lib/i386-linux-gnu/mesa/libGL.so -o $@

latheific-strip: latheific-small
	strip $< -R .comment -R .gnu.version -R .eh_frame -o $@
	sstrip $@

latheific-pack: latheific-strip
	cp $< $<-copy
	gzip -9 -n $<-copy
	echo "a=/tmp/I;tail -n+2 \$$0|zcat>\$$a;chmod +x \$$a;\$$a \$$@;echo \$$?;rm \$$a;exit" >$@
	cat $<-copy.gz >>$@
	chmod +x $@
	rm $<-copy.gz

clean:
	rm -f $(OBJECTS) smallmain.o linker.o latheific latheific-small latheific-strip latheific-pack *-copy *-copy.gz *~