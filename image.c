//*****************************************************************
//EDGARDO ADRIÁN FRANCO MARTÍNEZ
//(C) Agosto 2010 Versión 1.5
//Lectura, escritura y tratamiento de imagenes BMP


// Archivo modificado por Mariela Curiel par leer toda la imagen en la memoria
// y hacer la conversion de los pixeles en una funci'on. Esto facilita la programacion posterior
// con hilos.

// Archivo modificado por Pablo Ariza y Alejandro Castro
// Fecha de la ultima modificacion: 10-03-2017
//Compilación: "gcc image.c -o image"
//Ejecución: "./image [Ruta imagen original] [Ruta imagen destino]"
//Observaciones
//*****************************************************************




//*****************************************************************
//LIBRERIAS INCLUIDAS
//*****************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

//*****************************************************************
//DEFINICION DE CONSTANTES DEL PROGRAMA
//*****************************************************************
//********************************************************************************
//DECLARACION DE ESTRUCTURAS
//********************************************************************************
//Estructura para almacenar la cabecera de la imagen BMP y un apuntador a la matriz de pixeles
typedef struct BMP
{
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;					//(4 Bytes) Reservado
	int offset;						//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;			//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int alto;						//(4 Bytes) Ancho (numero de píxeles horizontales)
	int ancho;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;			//(2 Bytes) Numero de planos de color
	short int profundidadColor;		//(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;				//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;			//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;				//(4 Bytes) Cantidad de colores usados
	int coloresImportantes;			//(4 Bytes) Cantidad de colores importantes
	unsigned char ***pixel; 			//Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
}BMP;

typedef struct Hilo
{
	BMP *img;
	int n_hilos;
	int hilo_id;
}Hilo;
//*****************************************************************
//DECLARACIÓN DE FUNCIONES
//*****************************************************************
void abrir_imagen(BMP *imagen, char ruta[]);		//Función para abrir la imagen BMP
void crear_imagen(BMP *imagen, char ruta[]);	//Función para crear una imagen BMP
void convertir_imagen1(void *argv_convertir);
void convertir_imagen2(void *argv_convertir);
void convertir_imagen3(void *argv_convertir); //2 sera el numero de hilos

/*********************************************************************************************************
//PROGRAMA PRINCIPAL
//*********************************************************************************************************/
int main (int argc, char* argv[])
{
  int i, j, k, rc, n_hilos, *hilo_id, opcion; 				//Variables auxiliares para loops
	BMP img;				//Estructura de tipo imágen
	char IMAGEN[45];		//Almacenará la ruta de la imagen
	pthread_t *hilo;
	Hilo *argv_convertir;

	//******************************************************************
	//Si no se introdujo la ruta de la imagen BMP
	//******************************************************************
	if (argc!=5)
	//Si no se introduce una ruta de imágen
	{
		printf("\nError\nDebe ser ejecutado de la forma: %s [imagenIn].bmp [imagenOut].bmp [opcion] [nhilos] \n",argv[0]);
		exit(1);
	}
	//Almacenar la ruta de la imágen
	strcpy(IMAGEN,argv[1]);
	opcion = atoi( argv[3] );
	n_hilos = atoi( argv[4] );
	hilo_id = calloc( n_hilos, sizeof( int ) );
	hilo = calloc( n_hilos, sizeof( pthread_t ) );
	argv_convertir = calloc( n_hilos, sizeof( Hilo ) );
	//***************************************************************************************************************************
	//0 Abrir la imágen BMP de 24 bits, almacenar su cabecera en la estructura img y colocar sus pixeles en el arreglo img.pixel[][]
	//***************************************************************************************************************************
	abrir_imagen(&img,IMAGEN);
	printf("\n*************************************************************************");
	printf("\nIMAGEN: %s",IMAGEN);
	printf("\n*************************************************************************");
	printf("\nDimensiones de la imágen:\tAlto=%d\tAncho=%d\n",img.alto,img.ancho);
	for( i = 1; i <= n_hilos; i++ )
	{
		hilo_id[i - 1] = i;
		argv_convertir[i - 1].img = &img;
		argv_convertir[i - 1].n_hilos = n_hilos;
		argv_convertir[i - 1].hilo_id = hilo_id[i - 1];
		switch (opcion) {
			case 1:
				rc = pthread_create(&hilo[i - 1], NULL, (void *)convertir_imagen1, (void *) &argv_convertir[i -1]); //2 sera el numero de hilos
				if( rc != 0 )
				{
					perror("pthread_create: ");
					exit(1);
				}
				break;
			case 2:
				rc = pthread_create(&hilo[i - 1], NULL, (void *)convertir_imagen2, (void *) &argv_convertir[i -1]); //2 sera el numero de hilos
				if( rc != 0 )
				{
					perror("pthread_create: ");
					exit(1);
				}
				break;
			case 3:
				rc = pthread_create(&hilo[i - 1], NULL, (void *)convertir_imagen3, (void *) &argv_convertir[i -1]); //2 sera el numero de hilos
				if( rc != 0 )
				{
					perror("pthread_create: ");
					exit(1);
				}
				break;
		}
	}
	//***************************************************************************************************************************
	//1 Crear la imágen BMP a partir del arreglo img.pixel[][]
	//***************************************************************************************************************************

	//Terminar programa normalmente
	for( i = 0; i < n_hilos; i++ )
	{
		pthread_join( hilo[ i ], NULL );
	}
	crear_imagen(&img,argv[2]);
	printf("\nImágen BMP tratada en el archivo: %s\n",argv[2]);
	pthread_exit(0);
}

//************************************************************************
//FUNCIONES
//************************************************************************
//*************************************************************************************************************************************************
//Función para abrir la imagen, colocarla en escala de grisis en la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//*************************************************************************************************************************************************
void abrir_imagen(BMP *imagen, char *ruta)
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir
	int i,j,k;
        unsigned char P[3];

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "rb+" );
	if(!archivo)
	{
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se encontro\n",ruta);
		exit(1);
	}

	//Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
	fseek( archivo,0, SEEK_SET);
	fread(&imagen->bm,sizeof(char),2, archivo);
	fread(&imagen->tamano,sizeof(int),1, archivo);
	fread(&imagen->reservado,sizeof(int),1, archivo);
	fread(&imagen->offset,sizeof(int),1, archivo);
	fread(&imagen->tamanoMetadatos,sizeof(int),1, archivo);
	fread(&imagen->alto,sizeof(int),1, archivo);
	fread(&imagen->ancho,sizeof(int),1, archivo);
	fread(&imagen->numeroPlanos,sizeof(short int),1, archivo);
	fread(&imagen->profundidadColor,sizeof(short int),1, archivo);
	fread(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fread(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fread(&imagen->pxmh,sizeof(int),1, archivo);
	fread(&imagen->pxmv,sizeof(int),1, archivo);
	fread(&imagen->coloresUsados,sizeof(int),1, archivo);
	fread(&imagen->coloresImportantes,sizeof(int),1, archivo);

	//Validar ciertos datos de la cabecera de la imágen
	if (imagen->bm[0]!='B'||imagen->bm[1]!='M')
	{
		printf ("La imagen debe ser un bitmap.\n");
		exit(1);
	}
	if (imagen->profundidadColor!= 24)
	{
		printf ("La imagen debe ser de 24 bits.\n");
		exit(1);
	}

	//Reservar memoria para la matriz de pixels

	imagen->pixel=malloc (imagen->alto* sizeof(char *));
	for( i=0; i<imagen->alto; i++){
		imagen->pixel[i]=malloc (imagen->ancho* sizeof(char*));

	}
    for( i=0; i<imagen->alto; i++){
        for( j=0; j<imagen->ancho; j++)
    imagen->pixel[i][j]=malloc(3*sizeof(char));
	}

	//Pasar la imágen a el arreglo reservado en escala de grises
	//unsigned char R,B,G;

	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++){
		  for (k=0;k<3;k++) {
        fread(&P[k],sizeof(char),1, archivo); //Lectura de los pixeles
        imagen->pixel[i][j][k]=(unsigned char)P[k]; 	//asignacion del valor del pixel al arreglo
      }
		}
	}


	//Cerrrar el archivo
	fclose(archivo);
}

//****************************************************************************************************************************************************
//Función para crear una imagen BMP, a partir de la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************
void convertir_imagen1(void *argv_convertir)
{
	int i,j,k, alto = 0, ancho = 0, n_hilos, hilo_id;
	BMP *imagen;
  unsigned char temp;

	imagen = ((Hilo*) (argv_convertir))->img;
	n_hilos = ((Hilo*) (argv_convertir))->n_hilos;
	hilo_id = ((Hilo*) (argv_convertir))->hilo_id;
	if(hilo_id != n_hilos)
		ancho = ( ( imagen->ancho ) / n_hilos ) * hilo_id;
	else
		ancho = imagen->ancho;
	for (i = 0;i<imagen->alto;i++) {
		j = ( ( imagen->ancho ) / n_hilos ) * ( hilo_id - 1 );
		for (;j<ancho;j++) {
			for (k=0;k<3;k++)
        imagen->pixel[i][j][k]=(imagen->pixel[i][j][2]*0.3)+(imagen->pixel[i][j][1]*0.59)+ (imagen->pixel[i][j][0]*0.11);
		}
  }
	pthread_exit(NULL);
}

void convertir_imagen2(void *argv_convertir)
{
	int i,j,k, alto = 0, ancho = 0, n_hilos, hilo_id;
	BMP *imagen;
  unsigned char temp;

	imagen = ((Hilo*)(argv_convertir))->img;
	n_hilos = ((Hilo*)(argv_convertir))->n_hilos;
	hilo_id = ((Hilo*)(argv_convertir))->hilo_id;
	if(hilo_id != n_hilos)
		ancho = ( ( imagen->ancho ) / n_hilos ) * hilo_id;
	else
		ancho = imagen->ancho;
	for (i = 0;i<imagen->alto;i++) {
		j = ( ( imagen->ancho ) / n_hilos ) * ( hilo_id - 1 );
		for (;j<ancho;j++) {
			for (k=0;k<3;k++)
        imagen->pixel[i][j][k]=((imagen->pixel[i][j][2]+imagen->pixel[i][j][1] + imagen->pixel[i][j][0]))/3;
		}
  }
	pthread_exit(NULL);
}

void convertir_imagen3(void *argv_convertir){
  int i,j,k, alto = 0, ancho = 0, n_hilos, hilo_id;
	BMP *imagen;
  unsigned char temp;

	imagen = ((Hilo*) (argv_convertir))->img;
	n_hilos = ((Hilo*) (argv_convertir))->n_hilos;
	hilo_id = ((Hilo*) (argv_convertir))->hilo_id;
	if(hilo_id != n_hilos)
		ancho = ( ( imagen->ancho ) / n_hilos ) * hilo_id;
	else
		ancho = imagen->ancho;
	for (i = 0;i<imagen->alto;i++) {
		j = ( ( imagen->ancho ) / n_hilos ) * ( hilo_id - 1 );
		for (;j<ancho;j++) {
			for (k=0;k<3;k++)
        imagen->pixel[i][j][k]= 255 - imagen->pixel[i][j][k];
		}
  }
	pthread_exit(NULL);
}


void crear_imagen(BMP *imagen, char ruta[])
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir

	int i,j,k;

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "wb+" );
	if(!archivo)
	{
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se pudo crear\n",ruta);
		exit(1);
	}

	//Escribir la cabecera de la imagen en el archivo
	fseek( archivo,0, SEEK_SET);
	fwrite(&imagen->bm,sizeof(char),2, archivo);
	fwrite(&imagen->tamano,sizeof(int),1, archivo);
	fwrite(&imagen->reservado,sizeof(int),1, archivo);
	fwrite(&imagen->offset,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoMetadatos,sizeof(int),1, archivo);
	fwrite(&imagen->alto,sizeof(int),1, archivo);
	fwrite(&imagen->ancho,sizeof(int),1, archivo);
	fwrite(&imagen->numeroPlanos,sizeof(short int),1, archivo);
	fwrite(&imagen->profundidadColor,sizeof(short int),1, archivo);
	fwrite(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fwrite(&imagen->pxmh,sizeof(int),1, archivo);
	fwrite(&imagen->pxmv,sizeof(int),1, archivo);
	fwrite(&imagen->coloresUsados,sizeof(int),1, archivo);
	fwrite(&imagen->coloresImportantes,sizeof(int),1, archivo);

	//Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++)
		{

                    for (k=0;k<3;k++)
		       fwrite(&imagen->pixel[i][j][k],sizeof(char),1, archivo);  //Escribir el Byte Blue del pixel


		}
	}
	//Cerrar el archivo
	fclose(archivo);
}
