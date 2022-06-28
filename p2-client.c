#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8087
typedef struct data data;

//Estrucutura de información contenida en index
struct data
{
	int IDo;  //ID origen
	int IDd;  //ID destino
	int h;    //hora
	float t;  //tiempo
	int next; //siguiente dato enlazado
};

float buscarTiempo(data dato,int sock);

int main()
{
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	char* mesage = "Connect Client";
	//printf("\n");
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	

	int ido=0; //id origen
	int idd=0; //id destino
	int h=0;   //hora

	float time=-1; //tiempo
	int chs; //entrada de usuario

	//Menú
	printf("====================================\n");
	printf("Bienvenido\n");
	printf("\n");
	printf("1. Ingresar origen \n");
	printf("2. Ingresar destino \n");
	printf("3. Ingresar hora \n");
	printf("4. Buscar tiempo de viaje medio \n");
	printf("5. Salir\n");
	printf("====================================\n");
	printf("\n");
	while (1){
		
		//lectura de dato ingresado por usuario
		printf("Opción ");
		scanf("%d",&chs);
		if (chs == 1){
			printf("Ingrese ID del origen: ");
			scanf("%d",&ido);
			printf("\n");
		} else if (chs == 2){
			printf("Ingrese ID del destino: ");
			scanf("%d",&idd);
			printf("\n");
		} else if (chs == 3){
			printf("Ingrese hora del día: ");
			scanf("%d",&h);
			printf("\n");
		} else if (chs == 4){
			// llamado a función para consultar tiempo
			data dato = {ido,idd,h,-1,-1};
			float tiempo = buscarTiempo(dato,sock);
			// Si existe tiempo se imprime en caso contrario se imprime 'NA'
			if (tiempo != -1){
				printf("Tiempo de viaje medio: %.02f\n\n",tiempo);
			} else {
				printf("NA\n\n");
			}
		} else if (chs == 5){
			// envia dato en -1 para finalizar proceso de buscarTiempo
			data dato = {-1,-1,-1,-1,-1};
			float tiempo = buscarTiempo(dato,sock);
			close(client_fd);
			break;
		} else {
    			printf("Error! Opción no valida.\n");
    			printf("====================================\n");
			printf("1. Ingresar origen \n");
			printf("2. Ingresar destino \n");
			printf("3. Ingresar hora \n");
			printf("4. Buscar tiempo de viaje medio \n");
			printf("5. Salir\n");
			printf("====================================\n");
			printf("\n");
		}
		
	}

    return 0;
}

//Función que retorna tiempo consultado 
float buscarTiempo(data dato,int sock)
{
	
	//printf("Connect Server\n");
	send(sock, &dato, sizeof(data), 0);
	//printf("Data recived  \n");
	recv(sock, &dato, sizeof(data), 0);
	// closing the connected socket
	//printf("disconnected from the server\n");

	return dato.t;
}
