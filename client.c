#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"
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

void handleGetSequence(int socket_desc, char *server_response, char* server_message, char* filename, char *filename2) {
  printf("filename: %s\n", filename);
  if(recv(socket_desc, server_response, 8196, 0) > 0){
    FILE* file = fopen(filename2, "w+");  
    fprintf(file, "%s", server_response);
  }
}

void handlePutSequence(int socket_desc, char *server_response, char *server_message, char* filename, char* filename2) {
  printf("Handling put sequence: %s, %s\n", server_response, filename);
  read_file_to_buffer(server_message, filename);
  printf("server_message: %s\n", server_message);
  send(socket_desc, server_message, strlen(server_message), 0);
}

int main(int argc, char* argv[])
{
  struct sockaddr_in server_addr;
  //char server_message[2000], server_response[2000];
  char* server_message = malloc(2000);
  char* server_response = malloc(2000);
  char *command = malloc(4);
  char *filename = malloc(50);
 
  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(server_response,'\0',sizeof(server_response));
  printf("argc: %d\n", argc);
  if (argc == 2) {
    sprintf(server_message, "%s %s", argv[1], argv[2]);
  } else if(argc >= 2) {
    sprintf(server_message, "%s %s %s", argv[1], argv[2], argv[3]);
  }
  printf("Server_message: %s\n", server_message);
  if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    errExit("signal SIGINT");
  }

  if (signal(SIGSEGV, sigintHandler) == SIG_ERR) {
      errExit("signal SIGSEGV");
  }

  if (argc > 1) {
    command = argv[1];
  }

  if (argc > 2) {
    filename = argv[2];
  }
  
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

  //while(!SHUTDOWN) {
  // Get input from the user:
  //gets(server_message);
  
  //printf("%s\n", server_message);
  // Send the message to server:
  if(send(socket_desc, server_message, strlen(server_message), 0) < 0){
    printf("Unable to send message\n");
    return -1;
  }
  
  // Receive the server's response:
  //if(recv(socket_desc, server_response, 8196, 0) < 0){
  //  printf("Error while receiving server's msg\n");
  //  return -1;
  //}
  
  //printf("Server's response: %s\n",server_response);
  if(strcmp(command, "GET") == 0) {
    //FILE* file = fopen("client_file", "w+");  
    //fprintf(file, "%s", server_response);
    handleGetSequence(socket_desc, server_response, server_message, filename, argv[3]);
  } else if(strcmp(command, "PUT") == 0) {
    handlePutSequence(socket_desc, server_response, server_message, filename, argv[3]); 
  } else {
    printf("%s\n", server_response);
  }
  // Close the socket:
  close(socket_desc);
  
  exit(EXIT_SUCCESS);
}
