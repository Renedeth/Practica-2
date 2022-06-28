#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8087

#define MAXID 1160 //tamaño de ID

typedef struct data data;
typedef struct hash hash;

//Estrucutura de información contenida en index
struct data
{
	int IDo;  //ID origen
	int IDd;  //ID destino
	int h;    //hora
	float t;  //tiempo
	int next; //siguiente dato enlazado
};

//Estrucutura de la información contenida en tabla hash
struct hash
{
	int ID;        //hash ID 
	int posicion;  //ubicación en index 
};

int HashFunction(data dato);
float BuscarTiempo(data dato, int posicion);
int BuscarPosicionEnHash(data dato);


int main(){
	
	time_t timer;
	char time_s[26];
	struct tm* tm_info;

	int sockfd, ret;
	struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	char buffer[1024];
	pid_t childpid;
	
	int option = 1;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
	
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	//printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	//printf("[+]Bind to port %d\n", PORT);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening...\n");
	}else{
		printf("[-]Error in binding.\n");
	}

	//printf("Inicia proceso de busqueda...\n");
   
	float tiempo;
	int hash;
	data dato;

	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		//printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

			while(1){
				recv(newSocket, &dato, sizeof(data), 0);
				
				//printf("Client: - IDo: %i, IDd: %i, h: %i, t: %f, next: %i \n",dato.IDo,dato.IDd,dato.h,dato.t,dato.next);
				// Consula la posicion en tabla hash
				int posicion = BuscarPosicionEnHash(dato);

				// Consulta el tiempo
				tiempo = BuscarTiempo(dato,posicion);
				//printf("Tiempo: %.02f\n",tiempo);
				
				// Guarda tiempo en dato
				dato.t = tiempo;
				send(newSocket, &dato, sizeof(data), 0);
				timer = time(NULL);
				
				tm_info = localtime(&timer);

				strftime(time_s, 26, "%Y-%m-%d %H:%M:%S", tm_info);
				
				if(dato.IDo == -1){
					//printf("Finaliza por salida de menú.\n");
					break;
				}
				
				FILE *fp = fopen("log.txt", "a");
				if (fp == NULL)
				{
					printf("Error opening the file");
				}
				fprintf(fp, "[%s] Cliente [%s:%d] [%i-%i-%i]\n",time_s, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), dato.IDo, dato.IDd, dato.h);
				fclose(fp);
				//printf("[%s] Cliente [%s:%d] [%i-%i-%i]\n",time_s, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), dato.IDo, dato.IDd, dato.h);
			}
		}

	}

	close(newSocket);

}

//Función Hash
int HashFunction(data dato)
{
	return (MAXID*(dato.IDo-1))+(dato.IDd-1);
};

//Consulta tiempo
float BuscarTiempo(data dato, int posicion)
{
	//Abre archivo, si presenta error retorna -1
	FILE *F;
	F = fopen("index", "rb");
	if (F == NULL) {
		return -1;
	};
	
	//Se mueve a posición indicada
	fseek(F, sizeof(data)*posicion, SEEK_SET);
	//Se lee dato y se cierra archivo
	data datoEncontado;
	fread(&datoEncontado,sizeof(data),1,F);
	fclose(F);
	//Imprime dato leido
	//printf("IDo: %i, IDd: %i, h: %i, t: %f, next: %i \n",datoEncontado.IDo,datoEncontado.IDd,datoEncontado.h,datoEncontado.t,datoEncontado.next);
	
	//Valida que dato encontrado sea igual que el dato ingresado
	if(datoEncontado.IDo == dato.IDo && datoEncontado.IDd == dato.IDd && datoEncontado.h == dato.h){
		return datoEncontado.t;
	//Si son diferentes valida si el dato encontrado tiene un dato enlazado y si es así va a buscar en esa posición
	} else if (datoEncontado.next >= 0){
		return BuscarTiempo(dato, datoEncontado.next);
	//Si no se encuentra, se retorna -1
	} else {
		return -1;
	}
};


//Busca posición en la tabla hash
int BuscarPosicionEnHash(data dato)
{	
	//Abre archivo, si presenta error retorna -1
	FILE *F;
	F = fopen("hash", "rb");
	if (F == NULL) {
		return -1;
	};
	
	hash hashLeido;
	//Se mueve a posición indicada
	fseek(F, sizeof(hash)*HashFunction(dato), SEEK_SET);
	//Se lee dato y se cierra archivo
	fread(&hashLeido,sizeof(hash),1,F);
	fclose(F);
	
	//Se retorna posición encontrada
	return hashLeido.posicion;
};
