CFLAGS = -std=c99 -W -Wall -O3 -D_GNU_SOURCE

all: xbutton.so xbutton32.so

test: xbutton.so
	LD_PRELOAD=./xbutton.so xterm

xbutton.so: xbutton.c
	$(CC) -o $@ $< -shared -fPIC $(CFLAGS)

xbutton32.so: xbutton.c
	$(CC) -o $@ $< -m32 -shared -fPIC $(CFLAGS)

clean:
	rm -f *.so

