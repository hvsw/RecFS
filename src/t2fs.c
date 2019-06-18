
/**
*/
#include "../include/t2fs.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "sys/fcntl.h"

#define MAX_FILES 1000
char *files[MAX_FILES];

#define SECTOR_SIZE 256
#define SECTORS_PER_BLOCK 256
#define DISK_FILE "../t2fs_disk.dat"

//typedef unsigned char BYTE;
//typedef unsigned short int WORD;
//typedef unsigned int DWORD;

#pragma pack(push, 1)

/** Superbloco */
typedef struct  {
	//char    buffer[4];          
	char    version[2];        	/* Vers�o atual desse sistema de arquivos: (valor fixo 0x7E3=2019; 1=1� semestre). */
	//WORD    superblockSize; 	/* Quantidade de setores l�gicos que formam o superbloco. (fixo em 1 setor) */
	DWORD	DiskSize;			/* Tamanho total, em bytes, da parti��o T2FS. Inclui o superbloco, a �rea de FAT e os clusters de dados. */
	DWORD	NofSectors;			/* Quantidade total de setores l�gicos da parti��o T2FS. Inclui o superbloco, a �rea de FAT e os clusters de dados. */
	DWORD	SectorsPerCluster;	/* N�mero de setores l�gicos que formam um cluster. */
	DWORD	pFATSectorStart;	/* N�mero do setor l�gico onde a FAT inicia. */
	DWORD	RootDirCluster;		/* Cluster onde inicia o arquivo correspon-dente ao diret�rio raiz */
	DWORD	DataSectorStart;	/* Primeiro setor l�gico da �rea de blocos de dados (cluster 0). */
} t2fs_disk;

// TODO: Esse valor talvez seja um pouco grande,
// foi escolhido esse pois geralmente arquivos de audio
// em gravação tem um tamanho consideravel, sendo dificil
// encontrar arquivos muito pequenos, acredito que poderiamos
// ate aumentar pra sei la 1MB.
#define BLOCK_SIZE SECTOR_SIZE * SECTORS_PER_BLOCK  // 64KB

int diskInitialized = 0;

/*-----------------------------------------------------------------------------
Função:	Informa a identificação dos desenvolvedores do T2FS.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size) {
    strncpy (name, "Álvaro Souza - \nHenrique Valcanaia - 240501\nLucas Bauer - 237054\nLucas Lauck - 285688\n", size);
    return 0;
}

/*-----------------------------------------------------------------------------
Função:	Formata logicamente o disco virtual t2fs_disk.dat para o sistema de
		arquivos T2FS definido usando blocos de dados de tamanho
		corresponde a um múltiplo de setores dados por sectors_per_block.
-----------------------------------------------------------------------------*/
int format2 (int sectors_per_block) {
	if(!diskInitialized){
		t2fs_disk disco;
		diskInitialized = 1;
		FILE *arq;
		arq = fopen("/home/aluno/shared/t2fs_disk.dat", "r");

		if(arq == NULL){
			printf("problemas na abertura do arquivo de disco %d\n", errno);
			return -1;
		}

		fseek(arq, 0, SEEK_SET); //aponta ponteiro para inicio do arquivo

		char buffer[2];

		unsigned long bytesRead = fread(buffer, 1, 2, arq);

		printf("numero de bytes lidos: %lu\n", bytesRead);

		//int i;
		//for (i = 0; i < 1; ++i)
		//{
			//disco.version = buffer[i];
			printf("o que foi lido: %02X\n", buffer[0]);
			printf("o que foi lido: %02X\n", buffer[1]);
		//}

		strncpy(disco.version, buffer, sizeof(disco.version));



		printf("valor da versão salvo no disco %02X\n", disco.version[0]);
		printf("valor da versão salvo no disco %02X\n", disco.version[1]);
	return 0;
	}
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um novo arquivo no disco e abrí-lo,
		sendo, nesse último aspecto, equivalente a função open2.
		No entanto, diferentemente da open2, se filename referenciar um
		arquivo já existente, o mesmo terá seu conteúdo removido e
		assumirá um tamanho de zero bytes.
-----------------------------------------------------------------------------*/
FILE2 create2 (char *filename) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para remover (apagar) um arquivo do disco.
-----------------------------------------------------------------------------*/
int delete2 (char *filename) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função que abre um arquivo existente no disco.
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para fechar um arquivo.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para realizar a leitura de uma certa quantidade
		de bytes (size) de um arquivo.
-----------------------------------------------------------------------------*/
int read2 (FILE2 handle, char *buffer, int size) {
    return -1;
    
    FILE *diskFile = fopen(DISK_FILE, O_RDONLY);
    
    // TODO: Encontrar o primeiro bloco do arquivo
    int fileFirstBlock = 0;
    fseek(diskFile, fileFirstBlock, SEEK_SET);
    
    int numberOfSectorsToRead = size / SECTOR_SIZE;
    int remainder = size % SECTOR_SIZE;
    if (remainder != 0) { numberOfSectorsToRead++; }
    
    unsigned long bytesRead = fread(buffer, SECTOR_SIZE, numberOfSectorsToRead, diskFile);
    
    fclose(diskFile);
    return (int) bytesRead;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para realizar a escrita de uma certa quantidade
		de bytes (size) de  um arquivo.
-----------------------------------------------------------------------------*/
int write2 (FILE2 handle, char *buffer, int size) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para truncar um arquivo. Remove do arquivo
		todos os bytes a partir da posição atual do contador de posição
		(current pointer), inclusive, até o seu final.
-----------------------------------------------------------------------------*/
int truncate2 (FILE2 handle) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Altera o contador de posição (current pointer) do arquivo.
-----------------------------------------------------------------------------*/
int seek2 (FILE2 handle, DWORD offset) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um novo diretório.
-----------------------------------------------------------------------------*/
int mkdir2 (char *pathname) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para remover (apagar) um diretório do disco.
-----------------------------------------------------------------------------*/
int rmdir2 (char *pathname) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para alterar o CP (current path)
-----------------------------------------------------------------------------*/
int chdir2 (char *pathname) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para obter o caminho do diretório corrente.
-----------------------------------------------------------------------------*/
int getcwd2 (char *pathname, int size) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função que abre um diretório existente no disco.
-----------------------------------------------------------------------------*/
DIR2 opendir2 (char *pathname) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para ler as entradas de um diretório.
-----------------------------------------------------------------------------*/
int readdir2 (DIR2 handle, DIRENT2 *dentry) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para fechar um diretório.
-----------------------------------------------------------------------------*/
int closedir2 (DIR2 handle) {
	return -1;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para criar um caminho alternativo (softlink) com
		o nome dado por linkname (relativo ou absoluto) para um
		arquivo ou diretório fornecido por filename.
-----------------------------------------------------------------------------*/
int ln2 (char *linkname, char *filename) {
	return -1;
}
