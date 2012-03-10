CFLAGS=`pkg-config --cflags sdl` -Os -g -march=native -Wall -Wextra -std=c99
LDFLAGS=`pkg-config --libs sdl` -lGL -Os -g -march=native -Wall -Wextra -std=c99
OBJECTS=main.o scene.o

all: latheific latheific-strip latheific-pack

latheific: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

latheific-strip: latheific
	strip $< -R .comment -R .gnu.version -R .eh_frame -o $@
	sstrip $@

latheific-pack: latheific-strip
	cp $< $<-copy
	gzip -9 -n $<-copy
	echo "a=/tmp/I;tail -n+2 \$$0|zcat>\$$a;chmod +x \$$a;\$$a;rm \$$a;exit" >$@
	cat $<-copy.gz >>$@
	chmod +x $@
	rm $<-copy.gz

clean:
	rm -f $(OBJECTS) latheific latheific-strip latheific-pack *-copy *-copy.gz *~