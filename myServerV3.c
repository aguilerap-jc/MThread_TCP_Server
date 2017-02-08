#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>
#include <pthread.h>

//#define AMOUNT_OF_SENSORS 2

void *serverForClient(void *arg);

void *killAll(void *arg);

void doprocessing (int sock, int thNum);

char *startRoutine;

int scanFinished = 0;

void main(int argc, char **argv){
  if(argc != 3){
	printf("Enter The arguments Needed\nNumber of Sensors and Y/n for Init Scanning\n");
  	return;
  }
  int AMOUNT_OF_SENSORS = atoi(argv[1]);
  int tn[AMOUNT_OF_SENSORS];
  char initAll = argv[2][0];
  argv[2][0]= 'n';
  int i;
  if(scanFinished)
	  initAll = 'n';

  if(initAll == 'y' || initAll == 'Y'){
	  printf("Fill of Start Routine");
	  startRoutine = (char *)malloc(sizeof(char)* AMOUNT_OF_SENSORS);
	  for(i = 0 ; i < AMOUNT_OF_SENSORS; i++){
  		startRoutine[i] = 'y';
	  }
  }
  pthread_t handleID[AMOUNT_OF_SENSORS];

  for(i = 0; i < AMOUNT_OF_SENSORS; i++){
    tn[i] = i;
    pthread_create(&handleID[i], NULL, serverForClient, &tn[i]);
  } 


  for(i = 0; i < AMOUNT_OF_SENSORS; i++){
    pthread_join(handleID[i],NULL);
  }
  scanFinished = 1;
}

void *serverForClient(void *arg){
   int *thread = (int *)arg;
   int thN = *thread;
   int portno = 27000 + thN;
   //char sPort[5] ;
   //sprintf(sPort, "%d", port);
   //printf("%s \n",sPort);  

   int sockfd, newsockfd, clilen;
   char buffer[512];
   struct sockaddr_in serv_addr, cli_addr;
   int n, pid;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
   
   /* Now start listening for the clients, here
      * process will go in sleep mode and will wait
      * for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   while (1) {
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      
      /* Create child process */
      pid = fork();
		
      if (pid < 0) {
         perror("ERROR on fork");
         exit(1);
      }
      
      if (pid == 0) {
         /* This is the client process */
         close(sockfd);
         doprocessing(newsockfd,thN);
         exit(0);
      }
      else {
         close(newsockfd);
      }
		
   } /* end of while */
}

void doprocessing (int sock, int thNum) {
   int n;
   char buffer[512];
//////////////////////FILE CREATION//////////////////
    FILE *fp;	
    char sPort[5];
    char textName[15];
    int port = 27000 + thNum;
    
    sprintf(sPort, "%d", port);
    printf("%s \n",sPort);

    if(thNum < 10){
        char num[1];
        sprintf(num, "%d", thNum);
	strcpy(textName,"MyDataLog0");
	strcat(textName, num);
	strcat(textName, ".txt");
    }else{
	char myCharNum[2];
	sprintf(myCharNum, "%d", thNum);
	strcpy(textName,"MyDataLog");
	strcat(textName, myCharNum);
	strcat(textName, ".txt");
    }

    fp = fopen(textName,"w");
///////////////////////////////////////////////////////
   bzero(buffer,512);
   n = read(sock,buffer,511);
   

   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   if(buffer[0] == 'S'){
   	printf("Here is the message: %s\n", buffer);
	if(startRoutine[thNum] == 'Y' || startRoutine[thNum] == 'y'){
		n = write(sock, "YES",3);
		printf("Startinf Reading on Sensor Number: %d" , thNum);
	}
	bzero(buffer,511);
	n = read(sock, buffer,511);
   	while(strcmp(buffer,"END")){
		bzero(buffer,512);
		n = read(sock, buffer, 511);
		if(n > 0 ){
   			printf("My Data%d: %s\n",thNum, buffer);
			fprintf(fp, "%s", buffer);
		}
   	}
	startRoutine[thNum] = 'n';
   }
  
   printf("Ending transmission. \nFinal Data received: %s\n",buffer);
   n = write(sock,"I got your message",18);
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   fclose(fp);   
}

void *killAll(void *arg){
	char *p = (char *)arg;
	char abort = '0';
	while(abort != 'k'){
		abort = getc(stdin);
	}
	p[0] = 'A';
}
