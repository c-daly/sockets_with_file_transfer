#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

//https://www.delftstack.com/howto/c/sigint-in-c/
#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); \
                               } while (0)

#define MAX_CONNECTIONS 10

int SHUTDOWN = 0;
int socket_desc;

static void sigintHandler(int sig) {
  close(socket_desc);
  exit(EXIT_SUCCESS);
}
int main(int argc, char* argv[])
{
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];
  char *command = malloc(sizeof(char) * 4);
 
  if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    errExit("signal SIGINT");
  }

  if (argc > 1) {
    command = argv[1];
    printf("%s\n", command);
  }
  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Unable to create socket\n");
    return -1;
  }
  
  printf("Socket created successfully\n");
  
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(49153);
  //server_addr.sin_addr.s_addr = inet_addr("192.168.5.120");
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Unable to connect\n");
    return -1;
  }
  printf("Connected with server successfully\n");
  
  while(!SHUTDOWN) {
    // Get input from the user:
    printf("Enter message: ");
    gets(client_message);
    
    // Send the message to server:
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
      printf("Unable to send message\n");
      return -1;
    }
    
    // Receive the server's response:
    if(recv(socket_desc, server_message, sizeof(server_message), 0) < 0){
      printf("Error while receiving server's msg\n");
      return -1;
    }
    
    printf("Server's response: %s\n",server_message);
  }    
  // Close the socket:
  close(socket_desc);
  
  exit(EXIT_SUCCESS);
}
