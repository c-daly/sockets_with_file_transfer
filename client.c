#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "utils.h"
#include "config.h"

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
  // Receive the server's response:
  if(recv(socket_desc, server_response, 8196, 0) < 0){
    printf("Error while receiving server's msg\n");
  } else {
    printf("response: %s\n", server_response);
  }
 
  read_file_to_buffer(server_message, filename);
  printf("server_message: %s\n", server_message);
  send(socket_desc, server_message, strlen(server_message), 0);
  // get error code
  if(recv(socket_desc, server_response, 8196, 0) > 0) {
    printf("response: %c\n", server_response[0]);
  }
}

void handleInfoSequence(int socket_desc, char *server_response, char *server_message, char* filename, char* filename2) {
  if(recv(socket_desc, server_response, 8196, 0) > 0){
    printf("response: %s", server_response);
  }

}


void handleRMSequence(int socket_desc, char *server_response, char *server_message, char* filename, char* filename2) {
  printf("Handle RM reached\n");
  printf("sm: %s\n", server_message);
  if(recv(socket_desc, server_response, 8196, 0) > 0){
    printf("response: %s", server_response);
  }
}


void handleMDSequence(int socket_desc, char *server_response, char *server_message, char* filename, char* filename2) {
  printf("Handle MD reached\n");
  printf("sm: %s\n", server_message);
  if(recv(socket_desc, server_response, 8196, 0) > 0){
    printf("response: %s", server_response);
  }
}


void init_sockets() {
  struct sockaddr_in server_addr;  

  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    errExit("Unable to create socket");
  }
  
  printf("Socket created successfully\n");
  
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(49153);
  //server_addr.sin_addr.s_addr = inet_addr("192.168.5.120");
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  // Send connection request to server:
  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    errExit("Unable to connect");
  }
  printf("Connected with server successfully\n");
}

void craft_initial_server_message(char* server_message, int argc, char* argv[]) {
  if (argc == 2) {
    sprintf(server_message, "%s %s", argv[1], argv[2]);
  } else if(argc >= 2) {
    sprintf(server_message, "%s %s %s", argv[1], argv[2], argv[3]);
  }

}

int command_is_rm(char* command) {
  return strcmp(command, "RM") == 0; 
}

int command_is_info(char* command) {
  return strcmp(command, "INFO") == 0; 
}

int command_is_md(char* command) {
  return strcmp(command, "MD") == 0; 
}
void craft_server_message(char* server_message, char* command, char* filename, char* filename2, char* full_path1, char* full_path2) {

    //if(strcmp(command, "RM") == 0) {
    if(command_is_rm(command) ||
       command_is_info(command) ||
       command_is_md(command)) {
      sprintf(full_path1, "%s%s", SERVER_PATH, filename);
      sprintf(server_message, "%s %s", command, full_path1);
    } else {
      memset(server_message,'\0',sizeof(server_message));
      sprintf(server_message, "%s %s %s", command, full_path1, full_path2);
    }
}

void populate_command_args(int argc, char* argv[], char* command, char* filename, char* filename2) {
  if(argc < 2) {
    printf("Not enough args!");
  } 
  sprintf(command, "%s", argv[1]);
  
  printf("argv[2]: %s\n", argv[2]);
  if(argc <= 3) {
    sprintf(filename, "%s", argv[2]);
  }

  sprintf(filename2, "%s", argv[3]);
  printf("filename: %s\n", filename);
}


void register_error_handlers() {
  if (signal(SIGINT, sigintHandler) == SIG_ERR) {
    errExit("signal SIGINT");
  }

  if (signal(SIGSEGV, sigintHandler) == SIG_ERR) {
      errExit("signal SIGSEGV");
  }
}

void route_command_message(char* command, char* server_response, char* server_message, char* filename, char* filename2) {
  char* full_path1 = malloc(250);
  char* full_path2 = malloc(250);

  craft_server_message(server_message, command, filename, filename2, full_path1, full_path2);  

  if(send(socket_desc, server_message, strlen(server_message), 0) < 0){
    printf("Unable to send message\n");
    return;
  }

  if(strcmp(command, "GET") == 0) {
    strcpy(full_path1, filename);
    strcat(SERVER_PATH, full_path1);
    handleGetSequence(socket_desc, server_response, server_message, filename, filename2);
  } else if(strcmp(command, "PUT") == 0) {
    handlePutSequence(socket_desc, server_response, server_message, filename, filename2); 
  } else if(strcmp(command, "INFO") == 0) {
    handleInfoSequence(socket_desc, server_response, server_message, filename, filename2); 
  } else if(strcmp(command, "RM") == 0) {
    //strcat(SERVER_PATH, full_path1);
    printf("path: %s\n", full_path1);
    printf("server message: %s\n", server_message);
    handleRMSequence(socket_desc, server_response, server_message, full_path1, filename2); 
  } else if(strcmp(command, "MD") == 0) {
    printf("path: %s\n", full_path1);
    printf("server message: %s\n", server_message);
    handleMDSequence(socket_desc, server_response, server_message, full_path1, filename2);
  } else {
    printf("else branch\n");
    printf("%s\n", command);
  }
}

int main(int argc, char* argv[])
{
  char* server_message = malloc(2000);
  char* server_response = malloc(2000);
  char *command = malloc(4);
  char *filename = malloc(50);
  char *filename2 = malloc(50);
 
  register_error_handlers();

  populate_command_args(argc, argv, command, filename, filename2);
  // Clean buffers:
  memset(server_message,'\0',sizeof(server_message));
  memset(server_response,'\0',sizeof(server_response));

  printf("Server_message: %s\n", server_message);

  //if (argc > 1) {
  //  command = argv[1];
  //}

  //if (argc > 2) {
  //  filename = argv[2];
  //  filename2 = argv[3];
  //}
 
  init_sockets();
  //craft_initial_server_message(server_message, argc, argv);
  printf("command: %s\n", command);
  // Send the message to server:
  route_command_message(command, server_response, server_message, filename, filename2);
  //printf("Server's response: %s\n",server_response);
  //if(strcmp(command, "GET") == 0) {
  //  handleGetSequence(socket_desc, server_response, server_message, filename, filename2);
  //} else if(strcmp(command, "PUT") == 0) {
  //  handlePutSequence(socket_desc, server_response, server_message, filename, filename2); 
  //} else {
  //  printf("%s\n", server_response);
  //}

  // Close the socket:
  close(socket_desc);
  
  exit(EXIT_SUCCESS);
}
