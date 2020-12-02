#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MSGSIZE 512 

const char* validCommands[5]={"ls" , "cat" , "cut" , "grep" , "tr"};
void trimLeading(char * str);
bool StartsWith(char *a, const char *b);
char* itoa(int val, int base);

int main ( int argc , char * argv []) {
	FILE* fp;

	char *portToSend;
	char *lineNum;
	char *fileName=malloc(15);
	int i,j,k,f=0, p[2],pos=0;
	int port=atoi(argv[1]);
	int sockfd,recSock, ret;
	int newSocket;
	int numOfChildren=atoi(argv[2]);
	char *endCommand;
	char inbuf[MSGSIZE]; 
	char buffer[MSGSIZE];
	char payload[MSGSIZE];
	char string[MSGSIZE];
	char* command;
	pid_t childpid;
	struct sockaddr_in serverAddr, servaddr;
	struct sockaddr_in newAddr;
	socklen_t addr_size;
	long  bytes;
	//-------------------create socket for TCP-----------------
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY) ;

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %s\n", argv[1]);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}
	else{
		printf("[-]Error in binding.\n");
	}
	//---------- create pipe -----------
	if (pipe(p) < 0) 
        exit(1);

	//---------- create children -----------
	for(i=0 ; i<numOfChildren; i++){
		if ((childpid=fork()) == 0 ){
			break ;
		}
	}
	

	while(1){
		if(childpid!=0)
		{       // parent
			newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
			printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
			if(newSocket < 0)
			{
				perror( "accept " ) ;
				exit(1);
			}
			//---------- reveive tcp msg and write it in pipe-----------
			while(recv(newSocket, buffer, 512, 0)>0 ) {
				if (buffer[strlen(buffer)-1] == '\n') 
				{
					buffer[strlen(buffer)-1]= '\0';
				}
				write(p[1], buffer, MSGSIZE);
			}
		}
		else{
			close(sockfd);
			read(p[0], inbuf, MSGSIZE);
			//printf("inbuf %s \n",inbuf);
			portToSend= strtok(inbuf,".");
			lineNum=strtok(NULL,".");
			endCommand=strtok(NULL,"\0");

			// -----------Create datagram socket for response 
			if ( (recSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
				perror("UDP socket creation failed"); 
				exit(EXIT_FAILURE); 
			} 
			memset(&servaddr, 0, sizeof(servaddr)); 
		    // Filling server information 
			servaddr.sin_family = AF_INET; 
			servaddr.sin_port = htons(atoi(portToSend)); 
			servaddr.sin_addr.s_addr = INADDR_ANY; 

			trimLeading(endCommand);
			for (i=0; i<strlen(endCommand); i++){    //stop at ;
				if(endCommand[i]==';'){
					endCommand[i]='\0';
					break;
				}
			}
			//-------- parsing command to check id its valid-------------
			for (i=0; i<5; i++){
				if(StartsWith(endCommand, validCommands[i])) { 
					for (j=0; j<strlen(endCommand); j++){    //stop at ;
						if(endCommand[j]=='|'){
							for (k=0; k<5; k++){
								if(StartsWith(endCommand, validCommands[k])) {	
									f=1;
									break;
								}
							}
							if (f==0){
								endCommand[j]='\0';	
							}
							f=0;
						}
					}
					break;
				}
				else {
					if(i==4){
						endCommand[0]='\0';
					}
				}
			}


			if(endCommand[0]!='\0' && strlen(endCommand)<100){
				strcat(endCommand," 2>&1");
				//printf("command :%s",endCommand);
				fp = popen(endCommand, "r");

				while((bytes=fread(string, 1, (508-strlen(lineNum)), fp))!=0){
					if (strlen(string)==508-strlen(lineNum)){
						strcpy(payload,"0.");
					}
					else{
						strcpy(payload,"1.");
					}
					string[bytes]='\0';
					strcat(payload,lineNum);
					strcat(payload, ".");
					strcat(payload, string);
					pos=pos+508-strlen(lineNum);
					sendto(recSock, (const char *)payload, strlen(payload), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

					//printf("sending to client= %s\n", payload);
					memset(string,0,strlen(string));
					memset(payload,0,strlen(payload));
				}
				pos=0;
				pclose(fp);  
			}
			else{
				strcpy(payload,"1.");
				strcat(payload,lineNum);
				strcat(payload, ".");
				strcat(payload,":error");
				sendto(recSock, (const char *)payload, strlen(payload), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));  
				//printf("sending to client= %s\n", payload);
			}
			bzero(buffer, sizeof(buffer));
			bzero(inbuf, sizeof(inbuf));
			bzero(payload, sizeof(payload));
			bzero(endCommand, sizeof(endCommand));
		}
	}
	close(newSocket);
	return 0;
}
bool StartsWith(char *a, const char *b)
{
   trimLeading(a);
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

void trimLeading(char * str)
{
    int index, i, j;
    index = 0;
    /* Find last index of whitespace character */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }
    if(index != 0)
    {
        /* Shit all trailing characters to its left */
        i = 0;
        while(str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0'; // Make sure that string is NULL terminated
    }
}

char* itoa(int val, int base){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}
