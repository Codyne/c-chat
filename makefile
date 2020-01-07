all: server client

server:
	gcc `pkg-config --cflags --libs libnotify` src/server.c -o c-server -lpthread

client:
	gcc `pkg-config --cflags --libs libnotify` src/client.c -o c-client -lpthread -lncurses -lnotify

clean:
	rm -f c-server c-client
