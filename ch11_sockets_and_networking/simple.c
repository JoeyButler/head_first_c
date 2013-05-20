/* Simple Server example from page 454 in Head First C */
/* View /etc/services to see which ports common services bind to. */

#include <sys/socket.h>
#include <string.h>
#include <libc.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

void error(char* msg);
const int BUFFER_SIZE = 4096;
int read_in(int socket, char * buffer, int len);

int listener_d;

void handle_shutdown(int signal) {
  if(listener_d) close(listener_d);

  fprintf(stderr, "Peace Out!\n");
  exit(0);
}

int main(int argc, const char *argv[])
{
  if (signal(SIGINT, handle_shutdown) == SIG_ERR)
    error("Can't set the interrupt handler");

  /* Servers like to BLAB */
  /* BLAB: Bind, Listen, Accept, Begin*/
  listener_d = socket(PF_INET, SOCK_STREAM, 0);
  if(listener_d == -1) error("Cannot open socket");

  /* Tell the socket to allow us to rebind to the same port if we restart the server.*/
  /* Otherwise the OS will not let us connect to the same port.*/
  int reuse = 1;
  if(setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(int)) == -1)
    error("Can't set the reuse option on the socket.");

  /* Bind */
  struct sockaddr_in name;
  name.sin_family = PF_INET;
  name.sin_port = (in_port_t) htons(30000);
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  int c = bind(listener_d, (struct sockaddr *) &name, sizeof(name));

  /* This error can easily occur, either due to a permissions issue (any port less */
  /* than 1024) or the address is unavailable. The address could be unavailable */
  /* because another server is bound to that port. In order to check this you can */
  /* run `lsof -i :30000` to check the open file handlers that are bound to that */
  /* port. */
  if(c == -1) error("Cannot bind to socket");

  /* Listen */
  if(listen(listener_d, 10) == -1) error("Cannot listen to socket");

  while(1) {
    /* Accept */
    struct sockaddr_storage client_addr;
    unsigned int address_size = sizeof(client_addr);
    int connect_d = accept(listener_d, (struct sockaddr *) &client_addr,
        &address_size);
    if(connect_d == -1) error("Cannot open secondary socket");

    /* Begin */
    char* msg = "Internet Knock-Knock Protocol Server\r\nVersion 1.0\r\nKnock Knock!\r\n> ";
    if(send(connect_d, msg, strlen(msg), 0) == -1) error("send");

    char input_buffer[40000] = "";
    int ll = 80;
    read_in(connect_d, input_buffer, ll);
    fprintf(stderr, "%s", input_buffer);

     char* other_msg = "another msg\r\n";
    strcat(input_buffer, other_msg);
    if(send(connect_d, input_buffer, strlen(input_buffer), 0) == -1) error("send");

    sleep(1);
    close(connect_d);
  }

  close(listener_d);
  return 0;
}

/* int read_in(int socket, char * buffer) {*/
/*   char* msg = buffer;*/
/*   int buffer_size = BUFFER_SIZE;*/
/*   int recv_options = 0;*/
/*   int msg_length = recv(socket, msg, buffer_size, recv_options);*/

/*   [> Read from the socket until we see a newline. <]*/
/*   while((msg_length > 0) && msg[msg_length - 1] != '\n') {*/
/*     msg++; buffer_size -= msg_length;*/
/*     msg_length = recv(socket, msg, buffer_size, recv_options);*/
/*     printf("%s", ".");*/
/*   }*/

/*   if(msg_length < 0)*/
/*     return msg_length;*/
/*   else if(msg_length == 0)*/
/*     buffer[0] = '\0';*/
/*   else*/
/*     msg[msg_length - 1] = '\0';*/

/*   return buffer_size - BUFFER_SIZE;*/
/* }*/


/*int read_in(int socket, char *buf, int len) {*/
/*char *s = buf;*/
/*int slen = len;*/
/*int c = recv(socket, s, slen, 0); while ((c > 0) && (s[c-1] != '\n')) {*/
/*s += c; slen -= c;*/
/*c = recv(socket, s, slen, 0); }*/
/*if (c < 0) return c;*/
/*else if (c == 0) buf[0] = '\0';*/
/*else s[c-1]='\0';*/
/*return len - slen; }*/

int read_in(int socket, char* buffer, int len) {
  char* s = buffer;
  int slen = len;
  int c = recv(socket, s, slen, 0);
  /* There is a bloody bug in here.*/
  while((c > 0) && (s[c-1] != '\n')) {
    s++;
    slen -= c;
    c = recv(socket, s, slen, 0);
  }
  if(c < 0) return c;
  else if(c == 0) buffer[0] = '\0';
  else s[c-1] = '\0';
  return len - slen;
}

/* UTILS */

void error(char* msg) {
  printf("%s: %s\n", msg, strerror(errno));
  exit(1);
}

