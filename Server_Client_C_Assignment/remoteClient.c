#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
# include <netdb.h>

#define MAXLINE 512 
char* itoa(int val, int base);

int main ( int argc , char * argv []) {
	FILE *fp,*fp2;
	int PORT=atoi(argv[2]);
	int linenum=0,endCnt=0,i=0,countLines=0;
	int clientSocket,ret,recSock;
	struct sockaddr_in serverAddr,cliaddr,servaddr;
	struct hostent * rem ;
	char buffer[512];
	char *endMsg;
	char *lineMsg;
	char *infile;
	char *nameF={"output."};
	pid_t pid1;
	
	char c;

	//------------------create socket for TCP------------------
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		perror("[-]Error in connection.");
		exit(EXIT_FAILURE);
	}
	printf("[+]Client Socket is created.\n");
	
	/* Find server address */
	if (( rem = gethostbyname ( argv [1]) ) == NULL ) {
		herror ( " gethostbyname " ) ; 
		exit (1) ;
	}

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	memcpy (& serverAddr.sin_addr , rem->h_addr , rem->h_length ) ;
	serverAddr.sin_port = htons(PORT);
	
	//------------------create socket for datagram------------------ 
	if ( (recSock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
		perror("socket creation failed"); 
		exit(EXIT_FAILURE); 
	} 
	      
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	      
	// Filling server information 
	servaddr.sin_family    = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(atoi(argv[3])); 
	      
	// Bind the socket with the server address 
	if ( bind(recSock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
      
	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		perror("[-]Error in connection.\n");
		exit(EXIT_FAILURE);
		//exit(1);
	}
	printf("[+]Connected to Server.\n");

    // count input file lines to know when to close
    fp= fopen(argv[4],"r");
	for (c = getc(fp); c != EOF; c = getc(fp)){ 
		if (c == '\n') // Increment count if this character is newline 
	    	countLines = countLines + 1; 
	}
	fseek(fp, 0, SEEK_SET);  /* same as rewind(f); */

	pid1=fork(); //create child to receive messages
	while(1){
		if (pid1!=0){
			//------------------parent reads file line by line and sends to server------------------
      		while (i<10 && (fgets ( buffer, sizeof buffer, fp ) != NULL)  ){ /* read a line */
				i++;
				linenum++;
				char *result = malloc(strlen(buffer) + strlen(argv[3])+strlen(itoa(linenum,10)) + 3);
				strcpy(result, argv[3]);
				strcat(result, ".");
				strcat(result, itoa(linenum,10));
				strcat(result, ".");
				strcat(result, buffer);
				//printf("send data: %s\n", result);
				send(clientSocket, result, 512, 0);
				bzero(result, sizeof(result));
			}

			if (linenum==countLines){  //if client sends every message, parent waits for child to die
				wait(NULL);
				printf("[+]All messages received successfully\n[+]Terminating client!\n");							
				break;}
			else{  						//wait 5 secs 
				i=0;
				sleep(5);
			}
		}
		else{   //child receives
			int len, n; 
    		len = sizeof(cliaddr);  //len is value/resuslt 
    		n = recvfrom(recSock, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
			buffer[n] = '\0'; 
			//printf("received data : %s\n", buffer); 
    			 
			endMsg= strtok(buffer,".");
			lineMsg=strtok(NULL,".");
			infile=strtok(NULL,"\0");
			
			char *res = malloc(strlen(nameF) + strlen(argv[3])+ strlen(lineMsg) + 1);   //res is the output filename
			strcpy(res, nameF);
			strcat(res,argv[3]);
			strcat(res, ".");
			strcat(res, lineMsg);
			
			if(atoi(endMsg)==1)     //count for complete answers
				endCnt++;
		
			if(strcmp(infile,":error")==0){
				//printf("Got from server "":error"" Invalid command\n");
				fp2=fopen(res,"w");
				fprintf(fp2," ");
				fclose(fp2);
			}
			else{
				fp2=fopen(res,"a");
				fprintf(fp2,"%s",infile);
				fclose(fp2);
			}
		}
	
		memset(buffer,0,strlen(buffer));
		//memset(infile,0,strlen(infile));

		if(endCnt==countLines){      //all messages received, close sockets 
			close(recSock);
			close(clientSocket);
			break;
		}	
	}
	return 0;
}

char* itoa(int val, int base){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}
