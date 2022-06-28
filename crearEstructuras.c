#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAXCHAR 1000 //tamaño de linea a leer
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

bool CrearTablaHash();
bool CrearArchivoIndex();
int HashFunction(data dato);


int main() {

	printf("Inicia creación de estructuras.\n");

	if (!CrearTablaHash()){
		printf("Error al crear el archivo hash.\n");
		return 1;
	};
	
	if (!CrearArchivoIndex()){
		printf("Error al generar el archivo index.\n");
		return 1;
	};

	printf("Finalizado.\n");
	
	return 0;
}

//Creación de estructura de tabla Hash
bool CrearTablaHash()
{
	//Abre archivo, si presenta error retorna -1
	FILE *F;
	F = fopen("hash", "wb");	
	if (F == NULL) {
		return false;
	};
	
	//Estructura de tamaño MAXID*MAXID
	int i;
	int tam = MAXID*MAXID;
	for(i=0;i<tam;++i){
		hash dato = {i,-1};
		fwrite(&dato,sizeof(hash),1,F);
	};
	
	fflush(F);
	fclose(F);
	return true;
}

bool CrearArchivoIndex()
{
	FILE *FInfo;
	FILE *FIndex;
	FILE *FHash;
	char row[MAXCHAR];
	
	//Se Abren archivos, si presentan error retorna -1
	
	FInfo = fopen("bogota-cadastral-2020-1-All-HourlyAggregate.csv","r");
	if (FInfo == NULL) {
		return false;
	};
	FIndex = fopen("index", "wb");
	if (FIndex == NULL) {
		return false;
	};

	FHash = fopen("hash", "r+b");
	if (FHash == NULL) {
		return -1;
	};
	
	//Se omite primera fila de información
	fgets(row, MAXCHAR, FInfo);
	int i = 0;
	while (fgets(row, MAXCHAR, FInfo))
	{
		//Se guardan los primeros datos en la estructura
		data dato;
		dato.IDo = atoi(strtok(row, ","));
		dato.IDd = atoi(strtok(NULL, ","));
		dato.h = atoi(strtok(NULL, ","));
		dato.t = atof(strtok(NULL, ","));
		dato.next = -1;

		hash hashLeido;
		//Se consulta el IDhash para el dato leido
		int hashID = HashFunction(dato);
		
		//Se mueve a la posición del dato en la tabla hash y se optiene la posición a la cual esta apuntando
		fseek(FHash, sizeof(hash)*hashID, SEEK_SET);
		fread(&hashLeido,sizeof(hash),1,FHash);
		int posicion = hashLeido.posicion;
		hashLeido.ID = hashID;
		//Si ya esta apuntando a una posición, el nuevo dato apuntara a esa posición
		if(posicion >= 0){
			dato.next = posicion;
		}
		//Se guarda el dato en la tabla index
		fwrite(&dato,sizeof(data),1,FIndex);
		
		//Se guarda hash con nueva apuntando a nueva posición
		hash hashNuevo;
		hashNuevo.ID = hashID;
		hashNuevo.posicion = i;
		fflush(FHash);
		fseek(FHash, sizeof(hash)*hashID, SEEK_SET);
		fwrite(&hashNuevo,sizeof(hash),1,FHash);
		fflush(FHash);
		
		// Aumenta consucutivo de la posición
		++i;
	}
	
	//Se cierran archivos
	fflush(FIndex);
	fclose(FIndex);
	fflush(FHash);
	fclose(FHash);
	fclose(FInfo);
	return true;
}

//Función Hash
int HashFunction(data dato)
{   
	return (MAXID*(dato.IDo-1))+(dato.IDd-1);
};

