CFLAGS = -std=c99 -W -Wall -O3 -D_GNU_SOURCE
LDFLAGS = -ldl

all: xbutton.so xbutton32.so

test: xbutton.so xbutton32.so
	LD_PRELOAD=./xbutton.so xterm

xbutton.so: xbutton.c
	$(CC) -o $@ $< -shared -fPIC $(CFLAGS) $(LDFLAGS)

xbutton32.so: xbutton.c
	$(CC) -o $@ $< -m32 -shared -fPIC $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.so

