/*
 Rory Bresnahan
 bresnahr@onid.oregonstate.edu
 CS372
 Project 2
 2 connection client-server network application for file transfer
 ftServer accepts a server port command-line argument, this port will be used for the connection.  ftclient and ftserver establishes a TCP control connection,
 if ftServer receives valid command a TCP data connection is made with the client and if given the proper command will reciprocated and 
 transfer a file appointed by the client to the client
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <ctype.h>

#define BUFFSIZE 256
#define COMMANDSIZE 3

in_addr_t theHost;

// to initiate a socket that will listen for a client connection
int start_control_conn(int sockfd, int user_port){

	int control_sockfd;
	int integer = 1;
        struct sockaddr_in control_sockaddr;

        control_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
        if(control_sockfd < 0){
                fprintf(stderr, "Error control socket()\n");
                exit(1);
        }

        control_sockaddr.sin_family = AF_INET;
        control_sockaddr.sin_port = htons(user_port);
        control_sockaddr.sin_addr.s_addr = INADDR_ANY;
	theHost = control_sockaddr.sin_addr.s_addr;
	
	setsockopt(control_sockfd, SOL_SOCKET, SO_REUSEADDR, &integer, sizeof(int));

        if(bind(control_sockfd, (struct sockaddr*)&control_sockaddr, sizeof(control_sockaddr)) < 0){
                fprintf(stderr, "Error in bind\n");
                exit(1);
        }

	return control_sockfd;
	
}
// sends a message through a connection, arguments are connection (type) and message
void send_to_client(int sockfd, char *msg){

        int errorMSGlen = strlen(msg);
               int total = 0;
               int bytesleft = errorMSGlen;
               int n;
               while(total < errorMSGlen){
                       n = send(sockfd, msg+total, bytesleft, 0);
                       if(n==-1){ break;}
                       total += n;
                       bytesleft -= n;
                }

}

//create data connection between server and user
void handle_request(int port_for_data, char *fileBuff){

	int data_sockfd;
        int integer = 1;
        struct sockaddr_in data_sockaddr;

        data_sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if(data_sockfd < 0){
                fprintf(stderr, "Error data socket()\n");
                exit(1);
        }

        data_sockaddr.sin_family = AF_INET;
        data_sockaddr.sin_port = htons(port_for_data);
        data_sockaddr.sin_addr.s_addr = theHost;

	// set socket
        setsockopt(data_sockfd, SOL_SOCKET, SO_REUSEADDR, &integer, sizeof(int));
	
	if(connect(data_sockfd, (struct sockaddr*)&data_sockaddr, sizeof(data_sockaddr)) == -1){
		fprintf(stderr, "Error connecting to data socket\n");
		exit(1);
	}

	send_to_client(data_sockfd, fileBuff);
	close(data_sockfd);
}
int main(int argc, char* argv[]){

	if(argc != 2){
		printf("Please enter command-line argument: port number\n");
		exit(1);
	}

	int user_port = atoi(argv[1]);

	int i;
        for(i = 0; i < strlen(argv[1]); ++i){
        	if(!isdigit(argv[1][i])){
                	printf("Data port number not a valid integer\n");
                        exit(1);
                }
        }

	int control_sockfd, client_control_sockfd;

	control_sockfd = start_control_conn(control_sockfd, user_port);

	if(listen(control_sockfd, 1) < 0){
		fprintf(stderr, "listen error\n");
		exit(1);
	}else{
		printf("Connection established on control port %d\n", user_port);
	}

	int fileSize, port_for_data;
	char *fileBuff;
	char *pointer;
	char commands[COMMANDSIZE];
        char buff[BUFFSIZE];
	char message[BUFFSIZE];
	char cwd[BUFFSIZE];
	FILE *f;
	DIR *d;
	struct dirent* in_file;

	while(1){

		struct sockaddr_in dest;
		memset(&dest, 0, sizeof(dest));
		socklen_t size = sizeof(struct sockaddr_in);
		char host[1024];
		char service[20];

		client_control_sockfd = accept(control_sockfd, (struct sockaddr*)&dest, &size);//NULL, NULL);
		if(client_control_sockfd < 0){
			fprintf(stderr, "Error accepting client connection\n");
		}else{
			getnameinfo( (struct sockaddr*)&dest, sizeof dest, host, sizeof host, service, sizeof service, 0);
			printf("Connection made with client: %s  %s\n", inet_ntoa(dest.sin_addr), host);
		}
		
		memset(buff, '\0', BUFFSIZE);
		if(recv(client_control_sockfd, buff, BUFFSIZE, 0) == -1){
			fprintf(stderr, "Error receiving control message\n");
			exit(1);
		}
		
		// parse the message received from client into words, from http://stackoverflow.com/questions/11198604/c-split-string-into-an-array-of-strings	
		pointer = strtok(buff, " \n\0");
		strncpy(commands, pointer, COMMANDSIZE);
		pointer = strtok(NULL, " \n\0");//pointer saved as data port from client command

		port_for_data = atoi(pointer);
		
		if(strncmp(commands, "-l", 2) == 0){
			
			printf("List directory requested on port %d\n", port_for_data);
			memset(message, '\0', BUFFSIZE);
			// from http://stackoverflow.com/questions/11736060/how-to-read-all-files-in-a-folder-using-c
			// 	http://stackoverflow.com/questions/298510/how-to-get-the-current-directory-in-a-c-program
			if(getcwd(cwd, sizeof(cwd)) == NULL){
				fprintf(stderr, "Error opening directory\n");
			
			}else{		
			
				d = opendir(cwd);
				if(d == NULL){
					strcpy(message, "Directory has no files");
					send_to_client(client_control_sockfd, message);// send error message on control connection
					printf("Error message sent to %s\n", host);
				}else{
					
					while(in_file = readdir(d)){
						if(!strcmp(in_file->d_name, "."))
							continue;
						if(!strcmp(in_file->d_name, ".."))
							continue;
						strcat(message, in_file->d_name);
						strcat(message, "\n");
					}	
				
					message[strlen(message) - 1] = '\0';
					closedir(d);
					printf("Sending directory contents to %s: %d\n", host, port_for_data);
					handle_request(port_for_data, message);
				}		
			}
	
		}else if(strncmp(commands, "-g", 2) == 0){

			
			pointer = strtok(NULL, " \n\0");// parse received statement from client to get file name
			printf("File \"%s\" requested on port %d\n", pointer, port_for_data);			

			f = fopen(pointer, "r");// open the file and handle errors
			if(f == NULL){
				fprintf(stderr, "Error opening file %s.  Sending error message to %s: %d\n", pointer, host, port_for_data);
				memset(message, '\0', BUFFSIZE);
				strcpy(message, "Error: file not found on server!");
				send_to_client(client_control_sockfd, message);// send error message on control connection
			
			}else{
			
				//find file size and create a buffer that size+1, this will handle short and long files		
				fseek(f, 0, SEEK_END);// seek to end to get position
				fileSize = ftell(f);// get filesize
				fseek(f, 0, SEEK_SET);// seek to beggining
				fileBuff = (char*)malloc(fileSize);
				
				int len = fread(fileBuff, sizeof(char), fileSize-1, f);// copy file into fileBuff
				fileBuff[len] = '\0';
			
				printf("Sending \"%s\" to %s: %d\n", pointer, host, port_for_data);
				handle_request(port_for_data, fileBuff);// create data connnection and send data to client
				free(fileBuff);		
			}

		}

		close(client_control_sockfd);
		
	}

	close(control_sockfd);

	return 0;

}
