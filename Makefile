CC=gcc
CFLAGS=-Wall -Wextra
LINKS=/usr/local/lib/libspng_static.a /opt/homebrew/opt/zlib/lib/libz.a
OUTPUT=trimpng

all:
	$(CC) $(CFLAGS) -o $(OUTPUT) trimpng.c $(LINKS)

clean:
	rm -f $(OUTPUT)