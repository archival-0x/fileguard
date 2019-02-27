TARGET    = fileguard
MAIN_SRCS = $(wildcard src/*.c) $(wildcard src/*/*.c)
MAIN_OBJS = $(MAIN_SRCS:.c=.o)

CC		  = gcc
CFLAGS    = -DLOG_USE_COLOR -I. -Iinclude -I/usr/include/gdk-pixbuf-2.0 `pkg-config --cflags --libs glib-2.0`
LDFLAGS   = -lnotify -lyaml

all: $(MAIN_OBJS)
	@$(CC) $(CFLAGS) -o $(TARGET) $(MAIN_OBJS) $(LDFLAGS)

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -f src/*.o $(TARGET)
