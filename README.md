# c-chat
#### not a good chat application in C

A terrible project from 2017 when I wanted to start learning about socket programming.

The server runs by default on port 9001 with 255 max users, both of which determined arbtrarily in server.c. Connect to the server with telnet or the included client program. Only pthread.h is needed to compile the server.

The included client also by default uses port 9001 and can be changed in client.c. The client has an extremely minimal ncurses text user interface created with ncurses, and includes user notifications through libnotify.

Simply run `make` to compile both the client and the server and then run them

The client requires libncurses-dev and libnotify-dev\
`sudo apt-get install libncurses-dev libnotify-dev`

You can also compile them individually instead

Compiling and running the server\
`make server`\
`./c-serv`

Compiling and running the client\
`make client`\
`./c-client`

Check out [Chat-Server](https://github.com/yorickdewid/Chat-Server/) for a much better implementation, helped me a lot through starting the whole thing.
