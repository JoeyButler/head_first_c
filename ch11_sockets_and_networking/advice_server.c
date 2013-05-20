/*
 * Simple Server example from page 454 in Head First C
 * View /etc/services to see which ports common services bind to.
 *
 * DISCLAIMER: Most of the source code was taken from the examples in the book.
 * I would not consider the following to be well designed or written.
 * The purpose of the exercise was to simply learn the API and practice building
 * a server.
 */

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
  /* BLAB: Bind, Listen, Accept, Begin */
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

    if(!fork()) { // Inside the child process
      // Close the listener that the parent opened.
      close(listener_d);

      /* Begin */
      char* msg = "Internet Knock-Knock Protocol Server\r\nVersion 1.0\r\nKnock Knock!\r\n> ";
      if(send(connect_d, msg, strlen(msg), 0) == -1) error("send");

      char input_buffer[40000] = "";
      int ll = 80;
      read_in(connect_d, input_buffer, ll);
      fprintf(stderr, "The user said: %s\n", input_buffer);

      int cmp = strcmp(input_buffer, "Who's there?\r");

      if(cmp != 0) {
        if(send(connect_d, "Follow the rules\n", 18, 0) == -1)
          error("User didn't say \"Who's there?\"");
      }

      if(send(connect_d, "Oscar\n", 6, 0) == -1)
        error("send");

      read_in(connect_d, input_buffer, ll);
      fprintf(stderr, "The user said: %s\n", input_buffer);

      if(send(connect_d, "Oscar silly question, you get a silly answer\n", 46, 0) == -1)
        error("send");

      sleep(1);
      close(connect_d);
      exit(0);
    }

    // The parent needs to clean up as well.
    close(connect_d);
  }

  close(listener_d);
  return 0;
}

int read_in(int socket, char* buffer, int len) {
  int slen = len;
  int recv_num = recv(socket, buffer, slen, 0);
  while((recv_num > 0) && (buffer[recv_num-1] != '\n')) {
    buffer++;
    slen -= recv_num;
    recv_num = recv(socket, buffer, slen, 0);
  }
  if(recv_num < 0) return recv_num;
  else if(recv_num == 0) buffer[0] = '\0';
  else buffer[recv_num-1] = '\0';
  return len - slen;
}

/* UTILS */

void error(char* msg) {
  printf("%s: %s\n", msg, strerror(errno));
  exit(1);
}

