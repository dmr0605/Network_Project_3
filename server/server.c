/*

Programming project 3rd
Supervisor: Nguyen Hoai Son, PhD
Author: Hoang Cuong

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define SIZE 1500

int searchRequest();
void server() ;
void response_viewfile();
void doit(void * arg) ;
void showlist(void * arg);
void server_download() ;

void client_download() ;
static void recvfrom_alarm(int signo);

//define struct parameter 
struct paramenter{
  struct sockaddr_in  clientaddr;
  int fd ;
  char * fileName;
};


//define main function
int main(){

  pthread_t tcp_id, udp_id, showlist_id;
  
// create a new thread with attributes specified DEFAULT
  char *message1 = "Thread tcp...";
  char *message2 = "Thread udp...";
  char *message3 = "Show list...";
  int iret1, iret2, iret3;
  iret1 =  pthread_create(&tcp_id,NULL,(void*)server_download, (void *) message1);
  iret2 =  pthread_create(&udp_id,NULL,(void*)server, (void *) message2);
  iret3 =  pthread_create(&showlist_id,NULL,(void*)response_viewfile, (void *) message3);

  for(;;){
    int number;
    //    server();
    number = searchRequest();
    if(number == 0){
      printf (" ..........................................................\n");
      printf("NOTICE - There're no server conform your requiring.......... \n");
      printf("----------------------------------------------------------\n");
      printf("1 ------------------Enter keyword 1 to continue searching \n");
      printf("2 ..................Enter keyword 2 to View file from server\n");
      printf("3 ------------------Enter any number to escape  \n");
      printf ("...........................................................\n");
      fflush(stdin);
      int c;
      scanf("%d",&c);
      if(c == 1)
	continue;
      if(c == 2)
	view_file();
      else
	return 1;
    }
    else{
      printf (" ..........................................................\n");
      printf("NOTICE - Choose option to download ..................,...... \n");
      printf("-----------------------------------------------------.-----\n");
      printf("Option 1 ......Enter keyword 1 to search another keyword... \n");
      printf("Option 2 ......Enter keyword 2 view list download file comfortable\n");
      printf("Option 3 ......Press any keyword differents to escapce... \n");
      printf ("...........................................................\n");
      fflush(stdin);
      int c;
      scanf("%d",&c);
      switch(c){
      case 1 :
	continue;
      case 2 :
	client_download();
	continue;
      default :
	return (1);
      }
    }
    printf("--------------------------------------------------\n");
  }
  return 0;
}


static void recvfrom_alarm(int signo){
	return;
}

//function searchRequest
int searchRequest(){

  int result = 0;
  int sockfd;
  int nBytes;
  int len;
  int len_sock_server;

  struct sockaddr_in serveraddr;
  len_sock_server = sizeof(serveraddr);

  bzero(&serveraddr, len_sock_server);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(12345);
  serveraddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	
  char *fn = (char *) malloc (SIZE);
  if(fn== NULL) {
    printf ( " Sorry for this convenience but ... Error \n");
    return 1; // return number differ zero -> error
  }
  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("error when create socket");
    return 1; //number signal error creating...
  }

  printf("Enter filename for searching.... : \n");
  scanf("%s",fn);
  printf("Thanks, Your filename is '%s'. Please waiting sending signal time to server \n", fn);
  printf("Sending ...\n");


  // set property broadcast work for socket ...
  const int on = 1;

  // set the option specified by SO_BROADCAST
  setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

// ham sysv_signal thay the ham signal
  sysv_signal(SIGALRM, recvfrom_alarm);


  nBytes = sendto(sockfd, fn, strlen(fn), 0,(struct sockaddr*) &serveraddr , sizeof(serveraddr));
  alarm(5);
  for(;;){
    struct sockaddr_in	preply_addr;
    len = sizeof(preply_addr);
    char * recvString;
    recvString = (char*)malloc(SIZE);
    int n;
    n = recvfrom(sockfd, recvString, SIZE, 0, (struct sockaddr*)&preply_addr, (socklen_t*)&len);
    if(n < 0){
      if (errno == EINTR){
	break;
      }
      else 
	continue;
    }

    const char *str;
    str = (char*)malloc(SIZE);

    /*
      function converts the Internet host address in given in network byte order to a string in standard numbers-and-dots notation. The string is returned in a statically allocated buffer, which subsequent calls will overwrite.  
    */
    str = inet_ntoa(preply_addr.sin_addr);
    printf("<+> from %s :\n%s \n",str,recvString);
    result ++;
  }
  return result;
}


void server(){
  pthread_detach(pthread_self());
  int fd;
  int nBytes;

  if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("error when create socket");
    exit(1);
  }
// khai bao cau truc dia chi cua server , clien
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  int len_sock_server = sizeof(serveraddr);
  int len_sock_client = sizeof(clientaddr);

  bzero(&serveraddr, len_sock_server);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(12345);
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // lang nghe ket noi
  if(bind(fd, (struct sockaddr*) &serveraddr,len_sock_server) < 0){
    perror("could not open socket");
    exit(1);
  }

 for(;;){
    pthread_t newThread;
// nhan ten file tu may khach.
    char * fn_server;
    fn_server = (char*)malloc(SIZE);
    nBytes = recvfrom(fd, fn_server, SIZE, 0 ,(struct sockaddr*) &clientaddr, (socklen_t*) &len_sock_client);
    if(fn_server == NULL)
      {
	printf (" Error transaction and server can't recieve filename ...\n");
	return ;
      }

    printf (" File da nhan dc tu may khach .... '%s'\n", fn_server);

    if(nBytes < 0){
      if(errno == EINTR){
	printf("server : SIGALRM : \n");
	continue;
      }
      else{
	perror("recvfrom of server : ");
	break;
      }
    }
// gan gia tri cho cac thanh phan cua bien cau truc paramenter.
    struct paramenter paramenters;
    paramenters.fd = fd;
    paramenters.fileName = (char*)malloc(SIZE);
    strcpy(paramenters.fileName,fn_server);
    paramenters.clientaddr = clientaddr;
// tao thread xu li viec tra loi request tu cac may.
    int t = pthread_create(&newThread,NULL,(void*) doit,(void *)&paramenters);
    if(t != 0){
      perror("could not create new thread\n");
    }
  }
}


void doit(void * arg){
  pthread_detach(pthread_self());
  int nbyte;
  long size = 0;

  struct paramenter * assig;
  assig = (struct paramenter *)arg;

  char buff[SIZE] = "ls /tmp/k51mmt/ |grep ";
  strcat(buff,assig->fileName);
  char buff2[] = " > newfile";
  strcat(buff,buff2);
  system(buff);
	
	// mo file newfile.
  FILE * finput;
  finput = fopen("newfile", "r") ;
  if(finput == NULL){
    perror("Error when open new file");
		return ;
  }
	//kiem tra file rong hay khong. 
  fseek (finput, 0, SEEK_END);
  size = ftell(finput);

  if(size == 0){
    return ;
  }
  else{
		//chuyen vi tri con tro doc file ve dau file.
    fseek (finput,0,SEEK_SET);
	
    char * ch;
    ch = (char *) malloc(SIZE);
    int n;
    n = fread(ch,sizeof(char),SIZE,finput);
    if(n < 0){
      printf("can't read file\n");
    }
    // gui du lieu toi may da hoi.
    nbyte = sendto((assig->fd),ch,n,0,(struct sockaddr*)&(assig->clientaddr),sizeof((assig->clientaddr)));
    if(nbyte < 0){
      perror("error when send data\n");
    }
  }
  fclose(finput);
}



void client_download(){
	// nhap dia chi server
  char serverIP[15];
  printf("Nhap dia chi ip cua server : ");
  scanf("%s",serverIP);
// khai bao socket 
  int nbytes;
// tao ket noi tcp de download file.
  int TCP_socket;
  TCP_socket = socket(AF_INET,SOCK_STREAM,0);
  if(TCP_socket < 0){
    perror("Error when create socket");
    return;
  }
// khai bao dia chi may , giao thuc tang duoi, cong cua socket.
  struct sockaddr_in TCP_server_addr;
  bzero(&TCP_server_addr,sizeof(TCP_server_addr));
  TCP_server_addr.sin_family = AF_INET;
  TCP_server_addr.sin_port = htons(54321);
  TCP_server_addr.sin_addr.s_addr = inet_addr(serverIP);
// connect toi server.
  if(connect(TCP_socket,(struct sockaddr*)&TCP_server_addr,sizeof(TCP_server_addr)) < 0){
    perror("Error when connect to server");
    return;
  }
  char correctName[50];
  printf("Go chinh xac ten file can download : \n");
  scanf("%s",correctName);
  nbytes = send(TCP_socket,correctName,50,0);
  FILE * foutput;
  foutput = fopen(correctName, "wb");
  if(foutput == NULL){
    perror("Error when create new file");
    return;
  }
//nhan du lieu tu client.
  char * ch;
  int nbyte;
  for(;;){
    int n;
    ch =(char*)malloc(512);
    nbyte = recv(TCP_socket,ch,512,0);
    if(nbyte < 512){
      if(nbyte > 0)
//n = fwrite(ch,sizeof(char),strlen(ch),foutput) ;
	n = fwrite(ch,sizeof(char),nbyte,foutput);
      break;
    }
    else 
      n = fwrite(ch,sizeof(char),512,foutput);
  }

  fclose(foutput);
  printf("download completed \n");
  printf("----------------------------------------------------------\n");
}

// Ham download phuc vu yeu cau download mot file voi ten chinh xac.
void server_download(){
  pthread_detach(pthread_self());
// tao mot socket tcp cho viec truyen du lieu
  int serverSocket;
  int newsocket;
  serverSocket = socket(AF_INET,SOCK_STREAM,0);
  if(serverSocket < 0){
    perror("Error of create a new socket");
    return;
  }
// dat che do su dung lai dia chi 
  int yes = 1;
// "Address already in use" error message 
  if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("setsockopt() error");
    exit(1);
  }
// khai bao cau truc dia chi server cung nhu client.
  struct sockaddr_in sock_addr_server;
  struct sockaddr_in sock_addr_client;
  int len_sock_server = sizeof(sock_addr_server);
  int len_sock_client = sizeof(sock_addr_client);

  bzero(&sock_addr_server,len_sock_server);
  sock_addr_server.sin_family = AF_INET;
  sock_addr_server.sin_port = htons(54321);
  sock_addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	
  if(bind(serverSocket,(struct sockaddr*)&sock_addr_server,len_sock_server) < 0){
    perror("Error when assign address for socket");
    return ;
  }
// lang nghe kenh truyen 
  if(listen(serverSocket,5) < 0){
    perror("Error when listen connection");
    return;
  }
  for(;;){
    int nbyte;
// chap nhan ket noi tu client.
    newsocket = accept(serverSocket,(struct sockaddr*)&sock_addr_client,(socklen_t*)&len_sock_client);
    if(newsocket < 0 ){
      perror("Error when accept a connection from client");
      return;
    }
// nhan ten file chinh xac can download tu client
    char correctName[50];
    nbyte = recv(newsocket,correctName,50,0);
// khai bao file .
    FILE * fileDownload;
    char filename[100] = "/tmp/k51mmt/";
    strcat(filename,correctName); // ham noi hai xau lai voi nhau
    fileDownload = fopen(filename,"rb");
    if(fileDownload == NULL){
      perror("Error when open new file");
      return;
    }
// qua trinh download
    char * ch;
    int n = 0;
    for(;;){
      ch = (char*) malloc(512);
      n = fread(ch,sizeof(char),512,fileDownload);
      if(n < 512){
	if (n > 0){
	  nbyte = send(newsocket,ch,n,0);
	  if(nbyte < 0){
	    perror("Error when write data");
	  }
	}
	if (feof(fileDownload)){
	  break;
	}
      }
      else {
	nbyte = send(newsocket,ch,512,0);
	if(nbyte < 0){
	  perror("Error when write data");
	}
      }
    }
    fclose(fileDownload);
//close(newsocket) ;
  }
}
















//function searchRequest
int view_file(){

  int result = 0;
  int sockfd;
  int nBytes;
  int len;
  int len_sock_server;

  struct sockaddr_in serveraddr;
  len_sock_server = sizeof(serveraddr);

  bzero(&serveraddr, len_sock_server);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(2345); // port 2345 : xem view file....
  serveraddr.sin_addr.s_addr = inet_addr("255.255.255.255");
	
  char *fn = "view_list";
  if(fn== NULL) {
    printf ( " Sorry for this convenience but ... Error \n");
    return 1; // return number differ zero -> error
  }
  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("error when create socket");
    return 1; //number signal error creating...
  }

  // set property broadcast work for socket ...
  const int on = 1;

  // set the option specified by SO_BROADCAST
  setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

// ham sysv_signal thay the ham signal
  sysv_signal(SIGALRM, recvfrom_alarm);


  nBytes = sendto(sockfd, fn, strlen(fn), 0,(struct sockaddr*) &serveraddr , sizeof(serveraddr));
  alarm(5);
  for(;;){
    struct sockaddr_in	preply_addr;
    len = sizeof(preply_addr);
    char * recvString;
    recvString = (char*)malloc(SIZE);
    int n;
    n = recvfrom(sockfd, recvString, SIZE, 0, (struct sockaddr*)&preply_addr, (socklen_t*)&len);
    if(n < 0){
      if (errno == EINTR){
	break;
      }
      else 
	continue;
    }

    const char *str;
    str = (char*)malloc(SIZE);

    /*
      function converts the Internet host address in given in network byte order to a string in standard numbers-and-dots notation. The string is returned in a statically allocated buffer, which subsequent calls will overwrite.  
    */
    str = inet_ntoa(preply_addr.sin_addr);
    printf("<+> from %s :\n%s \n",str,recvString);
    result ++;
  }
  return result;
}












void response_viewfile(){
  pthread_detach(pthread_self());
  int fd;
  int nBytes;

  if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("error when create socket");
    exit(1);
  }
// khai bao cau truc dia chi cua server , clien
  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;
  int len_sock_server = sizeof(serveraddr);
  int len_sock_client = sizeof(clientaddr);

  bzero(&serveraddr, len_sock_server);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(2345);
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // lang nghe ket noi
  if(bind(fd, (struct sockaddr*) &serveraddr,len_sock_server) < 0){
    perror("could not open socket");
    exit(1);
  }

 for(;;){
    pthread_t newThread;
// nhan ten file tu may khach.
    char * fn_server;
    fn_server = (char*)malloc(SIZE);
    nBytes = recvfrom(fd, fn_server, SIZE, 0 ,(struct sockaddr*) &clientaddr, (socklen_t*) &len_sock_client);
    if(fn_server == NULL)
      {
	printf (" Error transaction and server can't recieve filename ...\n");
	return ;
      }

    printf (" File da nhan dc tu may khach .... '%s'\n", fn_server);

    if(nBytes < 0){
      if(errno == EINTR){
	printf("server : SIGALRM : \n");
	continue;
      }
      else{
	perror("recvfrom of server : ");
	break;
      }
    }
// gan gia tri cho cac thanh phan cua bien cau truc paramenter.
    struct paramenter paramenters;
    paramenters.fd = fd;
    paramenters.fileName = (char*)malloc(SIZE);
    strcpy(paramenters.fileName,fn_server);
    paramenters.clientaddr = clientaddr;
// tao thread xu li viec tra loi request tu cac may.
    int t = pthread_create(&newThread,NULL,(void*) showlist,(void *)&paramenters);
    if(t != 0){
      perror("could not create new thread\n");
    }
  }
}












void showlist(void * arg){
  pthread_detach(pthread_self());
  int nbyte;
  long size = 0;

  struct paramenter * assig;
  assig = (struct paramenter *)arg;

  char buff[SIZE] = "ls -all /tmp/k51mmt/ >> newfile";
  system(buff);

	// mo file newfile.
  FILE * finput;
  finput = fopen("newfile", "r") ;
  if(finput == NULL){
    perror("Error when open new file");
		return ;
  }
	//kiem tra file rong hay khong. 
  fseek (finput, 0, SEEK_END);
  size = ftell(finput);

  if(size == 0){
    return ;
  }
  else{
		//chuyen vi tri con tro doc file ve dau file.
    fseek (finput,0,SEEK_SET);
	
    char * ch;
    ch = (char *) malloc(SIZE);
    int n;
    n = fread(ch,sizeof(char),SIZE,finput);
    if(n < 0){
      printf("can't read file\n");
    }
    // gui du lieu toi may da hoi.
    nbyte = sendto((assig->fd),ch,n,0,(struct sockaddr*)&(assig->clientaddr),sizeof((assig->clientaddr)));
    if(nbyte < 0){
      perror("error when send data\n");
    }
  }
  fclose(finput);
}


