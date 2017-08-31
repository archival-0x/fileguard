TARGET    = watchman
MAIN_SRCS = $(wildcard src/*.c) $(wildcard src/*/*.c)
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

CC		  = gcc
CFLAGS    = -I. -Iinclude -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gdk-pixbuf-2.0
LDFLAGS   = -lnotify -lyaml

all: $(MAIN_OBJS)
	@$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_OBJS) $(LDFLAGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f src/*.o $(TARGET)
