#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "utils.h"
#include "config.h"


char client_message[8196], server_message[8196];
//int socket_desc, client_sock;
pthread_t pthread[10]; // 10 max concurrent connections
struct sockaddr_in server_addr, client_addr;
int SHUTDOWN = 0;


//char* send_file(FILE *fp, int sockfd, int size)
//{
//  char *data = malloc(size + 1);
//
//  printf("%s\n", data);
//  fseek(fp, 0, SEEK_SET);
//  fread(data, size, 1, fp); 
//  send(sockfd, data, size, 0);
//  return data;
//}

char* handle_info(char* data, char* filename) {
  printf("Handle Info\n");
  sprintf(data, "ls -la %s", filename);
  printf("data: %s\n", data);
  FILE* p = popen(data, "r"); 
  read_fp_to_buffer(data, p);
  return data;
}

int handle_put(int* client_sock, char* data, char* filename) {
  send(*client_sock, "ACK", 3, 0);
  recv(*client_sock, data, 8192, 0);
  printf("data: %s\n", data);
  printf("filename: %s\n", filename);
  return save_buffer_to_file(data, filename);
}

int handle_rm(char* path) {
  printf("rm path: %s\n", path);
  return remove(path);
}

int handle_md(char* path) {
  //TODO: permissions?
  printf("In md\n");
  return mkdir(path, 0700);
}

void* handle_get(char* data, char* filename) {
  read_file_to_buffer(data, filename);
  return data;
}

/*
 * thread worker that handles input from 
 * connected sockets
 */
void* handle_socket(void* arg) {
  int* client_sock = (int*) arg; 
  char* data = malloc(8196);
  char* cmd = malloc(4);
  char* filename = malloc(50);
  char* filename2 = malloc(50);

  while(!SHUTDOWN) {
   // Receive client's message:
    if (recv(*client_sock, client_message, 
            MESSAGE_SIZE, 0) > 0){
      printf("client_message: %s\n", client_message);
      cmd = strtok(client_message, " ");
      filename = strtok(NULL, " ");
      filename2 = strtok(NULL, " ");

      if(strcmp(cmd, "GET") == 0) 
      {
        data = (char*)handle_get(data, filename);
      } 
      else if(strcmp(cmd, "PUT") == 0) {
        int res = handle_put(client_sock, data, filename2);

        if(res > 0) {
          sprintf(data, "%d", res);
        } else {
          sprintf(data, "%d", -1);
        }
      } else if(strcmp(cmd, "INFO") == 0) {
          data = (char*) handle_info(data, filename);
      } else if(strcmp(cmd, "RM") == 0) {
          printf("rm filename: %s\n", filename);
          int res = handle_rm(filename);
          sprintf(data, "%d", res);      
      } else if(strcmp(cmd, "MD") == 0) {
          int res = handle_md(filename);
          sprintf(data, "%d", res);
      } else {
        data = "Unrecognized command";
      }
      sprintf(server_message, "%s", data);
      send(*client_sock, server_message, strlen(data), 0); 
   }
    if(*client_message) {
      printf("Msg from client: %s\n", client_message);
      *client_message = NULL;
    }
  }
}

int main_loop(int socket_desc) {
  int index = 0;
  int client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  char server_message[8196], client_message[8196];
  pthread_t pthread[10]; // 10 max concurrent connections
  while(1) {
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);
  
      if (client_sock < 0){
        printf("Can't accept\n");
        return -1;
      } else {
        printf("Client connected at IP: %s and port: %i\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));
        if(pthread_create(&(pthread[index]), NULL, handle_socket, &client_sock) != 0 ) {
          printf("Thread creation failed!");
        } else {
          index++;
        }
      }
  }
}

int init_sockets() {
  int socket_desc;
  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if(socket_desc < 0){
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");
  
  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(49153);
  server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  
  // Bind to the set port and IP:
  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");
  return socket_desc;
 
}

int listen_for_incoming_connections(int socket_desc) {
  // Listen for clients:
  if(listen(socket_desc, 1) < 0){
    printf("Error while listening\n");
    return -1;
  }
  printf("\nListening for incoming connections.....\n");
 
}
int main(void)
{
  int socket_desc = init_sockets();

  if(listen_for_incoming_connections(socket_desc) < 0) {
    // error occurred
    return -1;
  }

  main_loop(socket_desc);
 
  return 0;
}
