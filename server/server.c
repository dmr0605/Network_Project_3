/*
Programming project 3rd


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


int searchRequest();
void server() ;
void doit(void * arg) ;
void server_download() ;
void client_download() ;
static void recvfrom_alarm(int signo);

//define struct parameter 
struct paramenter{
	struct sockaddr_in  clientaddr ;
	int fd ;
	char * fileName ;
};


//define main function
int main(){
	pthread_t tcp_id ;
	pthread_t udp_id ;
	// luong dap ung yeu cau download .
	pthread_create(&tcp_id,NULL,(void*)server_download,NULL) ;
	// luong dap ung yeu cau tim kiem .
	pthread_create(&udp_id,NULL,(void*)server,NULL) ;
	// chuc nang request.
	for(;;){
		int number ;
		number = searchRequest() ;
		if(number == 0){
			printf("There're no server which named contain keyword entering... \n") ;
			printf("----------------------------------------------------------\n") ;
			printf("Enter another keyword searching.... \n") ;
			printf("Enter idiot number to escape (Not number 1 )... \n") ;
			fflush(stdin) ;
			int c ;
			scanf("%d",&c) ;
			if(c == 0)
				continue ;
			else
				exit(1) ;
		}
		else{
			printf("Nhap 1 de tim kiem voi tu khoa khac \n") ;
			printf("Nhap 2 de xem list file phu hop download file tu 1 server xac dinh\n") ;
			printf("Nhap so bat ky (khac 1,2) de thoat \n") ;
			fflush(stdin) ;
			int c ;
			scanf("%d",&c) ;
			switch(c){
				case 1 :
					continue ;
				case 2 :
					client_download() ;
					continue ;
				default :
					exit(1) ;
			}
		}
		printf("--------------------------------------------------\n") ;
	}
     return 0 ;
}


static void recvfrom_alarm(int signo){
	return;
}
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
	
  char fileName[25];
  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
    perror("error when create socket");
    return 1; //number signal error creating...
  }

  printf("Nhap ten file can tim kiem : \n");
  scanf("%s",fileName);
// thiet lap thuoc tinh broadcast cho socket.
  const int on = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
// ham sysv_signal thay the ham signal

  sysv_signal(SIGALRM, recvfrom_alarm);
  nBytes = sendto(sockfd, fileName, strlen(fileName), 0,(struct sockaddr*) &serveraddr , sizeof(serveraddr));
  alarm(5);

  for(;;){
    struct sockaddr_in	preply_addr;
    len = sizeof(preply_addr);
    char * recvString;
    recvString = (char*)malloc(1500);
    int n;
    n = recvfrom(sockfd, recvString, 1500, 0, (struct sockaddr*)&preply_addr, (socklen_t*)&len);	
		if(n < 0){
			if (errno == EINTR){
				break;
			}
			else
				continue;
		}
 		//char out[128];
		const char *str;
		str = (char*)malloc(50) ;
		str = inet_ntoa(preply_addr.sin_addr) ;
		printf("<+> from %s :\n%s \n",str,recvString) ;
		result ++ ;
	}	
	return result ;
}


void server(){
	pthread_detach(pthread_self()) ;
	int fd ;
	int nBytes ;	

     if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0){
		perror("error when create socket") ;
          exit(1) ;
     }

	// khai bao cau truc dia chi cua server , client
     struct sockaddr_in serveraddr ;
	struct sockaddr_in clientaddr ;
	int len_sock_server = sizeof(serveraddr) ;
	int len_sock_client = sizeof(clientaddr) ;

	bzero(&serveraddr, len_sock_server) ;
     serveraddr.sin_family = AF_INET ;
     serveraddr.sin_port = htons(12345) ;
     serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// lang nghe ket noi 
	if(bind(fd, (struct sockaddr*) &serveraddr,len_sock_server) < 0){
		perror("could not open socket") ;
		exit(1) ;
	}

	for(;;){
		pthread_t newThread ;
		// nhan ten file tu may khach.
		char * fileName ;
		fileName = (char*)malloc(25) ;
		nBytes = recvfrom(fd, fileName, 25, 0 ,(struct sockaddr*) &clientaddr, (socklen_t*) &len_sock_client);
		if(nBytes < 0) {	
			if(errno == EINTR){
				printf("server : SIGALRM : \n") ;
				continue ;
			}
			else{
				perror("recvfrom of server : "); 
				break;
			}
		}
		
		// gan gia tri cho cac thanh phan cua bien cau truc paramenter.
		struct paramenter paramenters ;
		paramenters.fd = fd ;
		paramenters.fileName = (char*)malloc(25) ;
		strcpy(paramenters.fileName,fileName) ;
		paramenters.clientaddr = clientaddr ;

		// tao thread xu li viec tra loi request tu cac may.
		int t = pthread_create(&newThread,NULL,(void*) doit,(void *)&paramenters) ;
		if(t != 0){
			perror("could not create new thread\n") ;
		}
	}
}


void doit(void * arg){
  pthread_detach(pthread_self());
  int nbyte;
  long size = 0;

  struct paramenter * assig;
  assig = (struct paramenter *)arg;

  char buff[100] = "ls /tmp/k51mmt/ |grep ";
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
    ch = (char *) malloc(1500);
    int n;
    n = fread(ch,sizeof(char),1500,finput);
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
	char serverIP[15] ;
	printf("Nhap dia chi ip cua server : ") ;
	scanf("%s",serverIP) ;

	// khai bao socket 

	int nbytes ;

	// tao ket noi tcp de download file.
	int TCP_socket ;
	TCP_socket = socket(AF_INET,SOCK_STREAM,0) ;
	if(TCP_socket < 0){
		perror("Error when create socket") ;
		return ;
	}

	// khai bao dia chi may , giao thuc tang duoi, cong cua socket.
	struct sockaddr_in TCP_server_addr ;

	bzero(&TCP_server_addr,sizeof(TCP_server_addr)) ;
	TCP_server_addr.sin_family = AF_INET ;
	TCP_server_addr.sin_port = htons(54321) ;
	TCP_server_addr.sin_addr.s_addr = inet_addr(serverIP) ;

	// connect toi server.
	if(connect(TCP_socket,(struct sockaddr*)&TCP_server_addr,sizeof(TCP_server_addr)) < 0){
		perror("Error when connect to server") ;
		return ;
	}
	
	char correctName[50] ;
	printf("Go chinh xac ten file can download : \n") ;
	scanf("%s",correctName) ;
	nbytes = send(TCP_socket,correctName,50,0) ;

	FILE * foutput ;
	foutput = fopen(correctName, "wb") ;
	if(foutput == NULL){
		perror("Error when create new file") ;
		return ;
	}
	//nhan du lieu tu client.
	char * ch ;
	int nbyte ;
	for(;;){
		int n;
		ch =(char*)malloc(512) ;
		nbyte = recv(TCP_socket,ch,512,0) ;
		if(nbyte < 512){
			if(nbyte > 0)
				//n = fwrite(ch,sizeof(char),strlen(ch),foutput) ;
				n = fwrite(ch,sizeof(char),nbyte,foutput) ;
			break ;
		}
		else
			n = fwrite(ch,sizeof(char),512,foutput) ;
	}	

	fclose(foutput) ;

	printf("download completed \n") ;
	printf("----------------------------------------------------------\n") ;
}

// Ham download phuc vu yeu cau download mot file voi ten chinh xac.
void server_download(){
	pthread_detach(pthread_self()) ;
	// tao mot socket tcp cho viec truyen du lieu.
	int serverSocket ;
	int newsocket ;
	serverSocket = socket(AF_INET,SOCK_STREAM,0) ;
	if(serverSocket < 0){
		perror("Error of create a new socket") ;
		return ;
	}

	// dat che do su dung lai dia chi 
	int yes = 1;
     // "Address already in use" error message 
     if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){	
		perror("setsockopt() error");
		exit(1);
     }

	//
	// khai bao cau truc dia chi server cung nhu client.
	struct sockaddr_in sock_addr_server ;
	struct sockaddr_in sock_addr_client ;
	int len_sock_server = sizeof(sock_addr_server) ;
	int len_sock_client = sizeof(sock_addr_client) ;

	bzero(&sock_addr_server,len_sock_server) ;
	sock_addr_server.sin_family = AF_INET ;
	sock_addr_server.sin_port = htons(54321) ;
	sock_addr_server.sin_addr.s_addr = htonl(INADDR_ANY) ;
	
	if(bind(serverSocket,(struct sockaddr*)&sock_addr_server,len_sock_server) < 0){
		perror("Error when assign address for socket") ;
		return ;
	}

	// lang nghe kenh truyen 
	if(listen(serverSocket,5) < 0){
		perror("Error when listen connection") ;
		return ;
	}
	for(;;){	
		int nbyte ;
		// chap nhan ket noi tu client.
		newsocket = accept(serverSocket,(struct sockaddr*)&sock_addr_client,(socklen_t*)&len_sock_client) ;
		if(newsocket < 0 ){
			perror("Error when accept a connection from client") ;
			return ;
		}
		// nhan ten file chinh xac can download tu client.
		char correctName[50] ;
		nbyte = recv(newsocket,correctName,50,0) ;
		
		// khai bao file .
		FILE * fileDownload ;
		char filename[100] = "/tmp/k51mmt/" ;
		strcat(filename,correctName) ; // ham noi hai xau lai voi nhau
		fileDownload = fopen(filename,"rb") ;
		if(fileDownload == NULL){
			perror("Error when open new file") ;
			return ;
		}
	
		// qua trinh download
		char * ch ;
		int n = 0 ;
		for(;;){
			ch = (char*) malloc(512) ;
			n = fread(ch,sizeof(char),512,fileDownload) ;
			if(n < 512){
				if (n > 0){
					nbyte = send(newsocket,ch,n,0) ;
					if(nbyte < 0){
						perror("Error when write data") ;
					}
				}	
				if (feof(fileDownload)){
					break ;
				}
			}
			else {
				nbyte = send(newsocket,ch,512,0) ;
				if(nbyte < 0){
					perror("Error when write data") ;
				}
			}
		}
		fclose(fileDownload) ;
		//close(newsocket) ;
	}
}

