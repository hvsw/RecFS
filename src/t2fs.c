
/**
*/
#include "../include/t2fs.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "sys/fcntl.h"
#include "../include/apidisk.h"


#define MAX_FILES 1000
char *files[MAX_FILES];

#define SECTORS_PER_BLOCK 256
#define BLOCK_SIZE SECTOR_SIZE * SECTORS_PER_BLOCK  // 64KB

#define BLOCK_COUNT 100


//TODO: Definir tam da arvore
#define DIRTREE_SIZE 0
    
#define DATA_AREA_SIZE SECTOR_SIZE*1

#define BITMAP_SIZE DATA_AREA_SIZE/8

#define DISK_SIZE DATA_AREA_SIZE+40+BITMAP_SIZE+DIRTREE_SIZE

#define DISK_FILE "../t2fs_disk.dat"

#define VERSION 0x7E31

/** Superbloco */
typedef struct  {
    WORD    version;
    WORD    sectorSize;
    WORD    partitionTableStart;
    WORD    partitionCount;
    DWORD   partitionStart; // temos apenas 1 particao
    DWORD   partitionEnd;
    char    partitionName[24];
} t2fs_disk;

t2fs_disk *superblock;

int bitmap[BLOCK_COUNT];

#pragma pack(push, 1)

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


void showSuperblockInfo() {
    printf("%s\n", __PRETTY_FUNCTION__);
    printf("Version: 0x%x\n", superblock->version);
    printf("Sector size: 0x%x\n", superblock->sectorSize);
    printf("Partition table start: 0x%x\n", superblock->partitionTableStart);
    printf("Partition count: 0x%x\n", superblock->partitionCount);
    printf("Partition start: 0x%x\n", superblock->partitionStart);
    printf("Partition end: 0x%x\n", superblock->partitionEnd);
    printf("Partition name: %s\n", superblock->partitionName);
}

int debug = 1;

void populateSuperblockWith(BYTE *buffer) {
    if (*((WORD *)buffer) != VERSION) {
      printf("Versão do sistema de arquivos não suportada!\nEsperado: %d\nEncontrado: 0x%hu\n", VERSION, superblock->version);
      exit(-2);
    }
    
    superblock->version = *((WORD *)buffer);
    superblock->sectorSize = *((WORD *)(buffer + 2));
    superblock->partitionTableStart = *((WORD *)(buffer + 4));
    superblock->partitionCount = *((WORD *)(buffer + 6));
    superblock->partitionStart = *((DWORD *)(buffer + 8));
    superblock->partitionEnd = *((DWORD *)(buffer + 12));
    strncpy(superblock->partitionName, buffer+16, 24);
}

int readSuperblockFromDisk() {
    BYTE buffer[SECTOR_SIZE];
    int resultRead = read_sector(0, buffer);
    if (resultRead < 0) {
        return resultRead;
    }
    populateSuperblockWith(buffer);
    return 0;
}

static int initIfNeeded() {
    if (diskInitialized) {
        return 1;
    }

    superblock = (t2fs_disk*) malloc(sizeof(*superblock));
    int readSuperblockResult = readSuperblockFromDisk();
    if (readSuperblockResult < 0) {
        return readSuperblockResult;
    }

    diskInitialized = 1;
    return 1;
}

void superblockToBuffer(BYTE *buffer) {
    memset(buffer, 0, SECTOR_SIZE);
    int version = VERSION;
    int sectorSize = SECTOR_SIZE;

    int i=0;
    for(;i<SECTOR_SIZE;i++){
        printf("%d", buffer[i]);
    }


    memcpy(buffer,    &version, sizeof(WORD));
    memcpy(buffer+2,  &sectorSize, sizeof(WORD));
    memcpy(buffer+4,  &superblock->partitionTableStart, sizeof(WORD));
    memcpy(buffer+6,  &superblock->partitionCount, sizeof(WORD));
    memcpy(buffer+8,  &superblock->partitionStart, sizeof(DWORD));
    memcpy(buffer+12, &superblock->partitionEnd, sizeof(DWORD));
    memcpy(buffer+16, superblock->partitionName, 24);


}

int _format2() {

    printf("INICIO FORMAT2\n");

    if (superblock != NULL) {
        free(superblock);
    }

    superblock = (t2fs_disk*) malloc(sizeof(*superblock));

    superblock->version = VERSION; // just in case..
    printf("VERSION\n");
    superblock->sectorSize = 0x100; // 0d256
    printf("SECTOR_SIZE");
    superblock->partitionTableStart = 0x8;
    printf("partitionTableStart\n");
    superblock->partitionCount = 0x1;
    printf("partitionCount\n");
    superblock->partitionStart = 0x28; // 0d40

    printf("ATRIBUICOES\n");

    int lastBlockAddress = DISK_SIZE - superblock->partitionStart - BITMAP_SIZE - DIRTREE_SIZE;
    superblock->partitionEnd = lastBlockAddress;

    printf("lastBlockAddress\n");
    
    int partitionNameSize = 24;
    unsigned char buffer[partitionNameSize];
    strncpy(superblock->partitionName, "PART_SEM_QUE_VEM", sizeof(buffer));  
    
    BYTE superblockData[SECTOR_SIZE];
    superblockToBuffer(superblockData);
    printf("SUPERBLOCK\n");

    int writeResult = write_sector(0, superblockData);
    printf("WRITE SECTOR\n");
    if (writeResult < 0) {
        return writeResult;
    }

    printf("FORMAT2\n");

    return 0;
}

int wipeDisk() {
    BYTE zeroFilledArray[SECTOR_SIZE];
    memset(zeroFilledArray, 0, SECTOR_SIZE);

    int numberOfSectors = DISK_SIZE/SECTOR_SIZE;
    int currentSector = 0;
    for (currentSector = 0; currentSector < numberOfSectors; currentSector++) {
        int writeResult = write_sector(currentSector, zeroFilledArray);
        if (writeResult < 0) {
            return writeResult;
        }
    }

    printf("WIPOU\n");

    return 0;
}


int format2 (int sectors_per_block) {
    int wipeDiskResult = wipeDisk();
    if (wipeDiskResult < 0) {
        return wipeDiskResult;
    }

    int formatResult = _format2();
    if (formatResult < 0) {
        return formatResult;
    }

    readSuperblockFromDisk();
    showSuperblockInfo();

    return 0;
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
