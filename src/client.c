#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>//for winsize struct for resizine handler
#include <netdb.h>//hostent
#include <arpa/inet.h>//inet_addr
#include <pthread.h>//threading
#include <ncurses.h>
#include <libnotify/notify.h>
#include <glib.h>
#include <signal.h>//for resize handler

#define PORT 9001

//ncurses windows
WINDOW *outterChatwin;//this window just servers as the box() for chatwin
WINDOW *chatwin;//all contents of the chat are stored in the window, which is inside outterChatwin
WINDOW *outterSendwin;
WINDOW *sendwin;

int chatline = -1;//keeps track of what line the chat window is on
int sendline = 0;

//socket
int sock;
int msgsize = 378;

bool cont = 1;

void serv_connect();
void draw_windows();
void *sender();
void *listener();

void resizeHandler(int sig);

void error(const char *msg);

int main(int argc, char *argv[]) {
  serv_connect();
  draw_windows();

  signal(SIGWINCH, resizeHandler);
	
  //set up threads
  pthread_t threads[2];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  //create threads
  pthread_create(&threads[0], &attr, sender, NULL);
  pthread_create(&threads[1], &attr, listener, NULL);

  while(cont);//keep threads alive
  close(sock);
  endwin();
  return 0;
}

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void serv_connect() {
  //getting ip of hostname
  char *hostname = "localhost";
  char ip[100];
  char buffer[msgsize];
  struct hostent *he;
  struct in_addr **addr_list;
  int i;

  if ((he = gethostbyname(hostname)) == NULL) {
    herror("gethostbyname");
    exit(1);
  }

  addr_list = (struct in_addr **)he->h_addr_list;

  for(int i = 0; addr_list[i] != NULL; i++)
    strcpy(ip, inet_ntoa(*addr_list[i]));

  printf("Connecting to %s...\n", hostname);

  //int sock;
  struct sockaddr_in server;
  char *msg;
  char serv_reply[msgsize];

  //creating socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) error("Failed to create socket");

  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);

  //connect to the server
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    error("Failed to connect to server");

  printf("Connected to %s\n", hostname);
}

void draw_windows() {
  //create the windows
  clear();
  initscr();
  cbreak();
  chatline = -1;
  
  //boundaries of windows
  //h = height of chat window
  //w = width
  //nh = height of outer send window
  //startx/starty = starting point to draw window
  int h, nh, w, starty,startx;

  h = LINES - 3;
  w = COLS - 4;
  starty = 0;
  startx = 2;

  outterChatwin = newwin(h, w, starty, startx);
  box(outterChatwin, 0, 0);

  chatwin = newwin(h - 2, w - 2, starty + 1, startx + 1);
  scrollok(chatwin, TRUE);

  nh = 3;
  starty = h;
  outterSendwin = newwin(nh, w, starty, startx);
  box(outterSendwin, 0, 0);
  sendwin = newwin(1, w - 2, starty + 1, startx + 1);

  wsetscrreg(chatwin, 1, h - 2);

  refresh();
  wrefresh(outterChatwin);
  wrefresh(chatwin);
  wrefresh(sendwin);
  wrefresh(outterSendwin);
}

void *sender() {
  int nsock = sock;
  char buffer[msgsize];

  while(1) {	
    //zero the buffer for the message
    bzero(buffer, msgsize);
    //get the input from the user in the sendwin window
    mvwgetnstr(sendwin, 0, 0, buffer, (msgsize - 1));	
		
    //if /quit or /exit then exit client to terminal
    if (strcmp(buffer, "/quit") == 0 || strcmp(buffer, "/exit") == 0) {
      cont = 0;
      pthread_exit(NULL);
    } else if (strcmp(buffer, "/clear") == 0) {
      werase(chatwin);
      refresh();
      wrefresh(chatwin);
      chatline = -1;
    } else if (strcmp(buffer, "/connect") == 0) {
      serv_connect();
      draw_windows();
    } else {
      //add "\n" at the end of of the buffer
      strncat(buffer, "\n", (msgsize) - strlen(buffer));
      write(nsock, buffer, strlen(buffer));
    }
		
    //erase everything from sendwin and then re-draw sendwin
    werase(sendwin);
    //refresh sendwin after re-draw
    refresh();
    wrefresh(sendwin);
    wrefresh(outterSendwin);
  }
}

void *listener() {
  int nsock = sock;
  int srvmsgsize = 400;//message size of the message from the server
  char buffer[srvmsgsize];
  NotifyNotification *n;
  notify_init("chat notify");

  while(1) {
    bzero(buffer, srvmsgsize);
    refresh();
    wrefresh(chatwin);
    wrefresh(sendwin);		
    read(nsock, buffer, srvmsgsize);
		
    //create new notification titled c-chat
    //msg from c-chat is the body
    n = notify_notification_new("c-chat", buffer, 0);
    //notification remains up for 3 seconds
    notify_notification_set_timeout(n, 3000);
		
    //display the notification
    if(!notify_notification_show(n, 0))
      printf("Notification failed to show\n");
		
    //check what line we're currently on
    //if at max chatlines for the window then scroll
    //
    //LINES - 6 because the height of chatwin is LINES - 5, and another - 1 because of the bottom and top border

    if (chatline != LINES - 6) chatline++;
    else scroll(chatwin);

    //print the message to the chatwin screen
    mvwprintw(chatwin, chatline, 0, buffer);
  }	
}

void resizeHandler(int sig){
  endwin();
  draw_windows();

  //clear sendwin buffer or else it will send blank messages
  char buffer[msgsize];
  mvwgetnstr(sendwin, 0, 0, buffer, (msgsize - 1));
}
