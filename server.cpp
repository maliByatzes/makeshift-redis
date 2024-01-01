#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

// do_something simply reads and writes using the connfd
static void do_something(int connfd) {
  // Creates new buffer
  char rbuf[64] = {};
  // Read int the buffer using the read syscall
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);

  // Handle read() error
  if (n < 0) {
    msg("read() error");
    return;
  }
  // Print the contents read from the client
  printf("Client says: %s\n", rbuf);

  // Send back "world" to the client
  char wbuf[] = "world";
  write(connfd, wbuf, sizeof(wbuf)); // NOTE: Returns number of bytes of written
                                     // which is best to handle
}

int main() {
  // Get the socket fd from the socket syscall
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  // Manipulat the socket_fd options by using the setsockopt syscall
  // SOL_SOCKET - set the level of where to search for an address,
  //              in this case, search the address within the socket itself
  // SO_REUSEADDR - forcibly binds to an address if even its in use by
  // another process
  int val = 1;
  setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // Bind and listen to an address using bind() and listen() syscalls
  struct sockaddr_in addr = {};
  // Use IPV4 address configuration
  addr.sin_family = AF_INET;
  // Assign to port 1234
  addr.sin_port = ntohs(1234);
  // With a wildcard address of 0.0.0.0
  addr.sin_addr.s_addr = ntohl(0);

  int rv = bind(socket_fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // Invoke the listen syscall
  rv = listen(socket_fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  // Loop through each connection
  while (true) {
    struct sockaddr_in client_addr = {};
    // Get the length of client_addr socket
    socklen_t socklen = sizeof(client_addr);
    // Invoke the accept syscall
    int connfd = accept(socket_fd, (struct sockaddr *)&client_addr, &socklen);
    // Handle error, connfd < 1
    if (connfd < 0) {
      continue;
    }

    // Peform some operations on the connection fd
    do_something(connfd);
    // Close the fd when done
    close(connfd);
  }

  return 0;
}
