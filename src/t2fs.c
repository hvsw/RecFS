
/**
*/
#include "../include/t2fs.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "sys/fcntl.h"
#include "../include/apidisk.h"
#include "../include/tree.h"
#include "../include/RecFile.h"


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

RecFile workingDirectory;

RecFile rootDirectory;

t2fs_disk *superblock;

int bitmap[BLOCK_COUNT];

#define OPENED_FILES_LIMIT 20
int openedFilesNumber = 0;
tree openedFiles[OPENED_FILES_LIMIT];
#define ERROR_INVALID_HANDLE -1
int isValidHandle(int handle) {
    return (handle > 0) && (handle < OPENED_FILES_LIMIT-1) && (handle < openedFilesNumber-1);
}

#pragma pack(push, 1)

int diskInitialized = 0;

/*-----------------------------------------------------------------------------
Função:	Informa a identificação dos desenvolvedores do T2FS.
-----------------------------------------------------------------------------*/
int identify2 (char *name, int size) {
    strncpy (name, "Álvaro Souza - 231162\nHenrique Valcanaia - 240501\nLucas Bauer - 237054\nLucas Lauck - 285688\n", size);
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
    return 0;
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
    if (superblock != NULL) {
        free(superblock);
    }

    superblock = (t2fs_disk*) malloc(sizeof(*superblock));

    superblock->version = VERSION; // just in case..
    superblock->sectorSize = 0x100; // 0d256
    superblock->partitionTableStart = 0x8;
    superblock->partitionCount = 0x1;
    superblock->partitionStart = 0x28; // 0d40

    int lastBlockAddress = DISK_SIZE - superblock->partitionStart - BITMAP_SIZE - DIRTREE_SIZE;
    superblock->partitionEnd = lastBlockAddress;
    
    int partitionNameSize = 24;
    unsigned char buffer[partitionNameSize];
    strncpy(superblock->partitionName, "PART_SEM_QUE_VEM", sizeof(buffer));  
    
    BYTE superblockData[SECTOR_SIZE];
    superblockToBuffer(superblockData);

    int writeResult = write_sector(0, superblockData);
    if (writeResult < 0) {
        return writeResult;
    }

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
int fileExists(*filename){
    return -1;
}

int _create2(char *filename){
    return -1;
}

FILE2 create2 (char *filename) {
    if (strcmp(filename,"") ==0 || strcmp(filename,"/") ==0) {
		return -1;
    }
    
    int initResult = initIfNeeded();
    if (initResult < 0) {
        printf("Problemas na inicialização do disco\n");
        return initResult;
    }

    int fileExistsResult = fileExists(filename);
    switch(fileExistsResult){
        case -1:
            if(debug){
                printf("Erro ao pesquisar arquivo na árvore\n");
            }
            return fileExistsResult;
        case 1:
            if(debug){
                printf("Arquivo encontrado\n");
            }
            int deleteResult = delete2(filename);
            if (deleteResult < 0) {
                return deleteResult;
            }
            break;
        default: break;
    }

    int create2Result = _create2(filename);
    if(create2Result < 0) {
        return create2Result;
    }

	return 0;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para remover (apagar) um arquivo do disco.
-----------------------------------------------------------------------------*/
int delete2 (char *filename) {
	return -1;
}

RecFile getFileFromPath(char *filename) {
    RecFile file;
    memcpy(&file.name, "mocked file", 11);
    file.size = 100;
    file.startingBlock = 0;
    file.type = RECFILE_TYPE_FILE;
    return file;
}

/*-----------------------------------------------------------------------------
Função:	Função que abre um arquivo existente no disco.
-----------------------------------------------------------------------------*/
FILE2 open2 (char *filename) {
    RecFile file = getFileFromPath(filename);
    openedFiles[openedFilesNumber++] = file;
	return openedFilesNumber;
}

/*-----------------------------------------------------------------------------
Função:	Função usada para fechar um arquivo.
-----------------------------------------------------------------------------*/
int close2 (FILE2 handle) {
    if (!isValidHandle(handle)) {
        return ERROR_INVALID_HANDLE;
    }
    free(&openedFiles[openedFilesNumber]);
    openedFilesNumber--;
    
	return 0;
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
    if (!isValidHandle(handle)) {
        return ERROR_INVALID_HANDLE;
    }
    
    struct tree file = openedFiles[handle];
    int fileSectorOffset = file.file.startingBlock * SECTORS_PER_BLOCK;
    
    int numberOfSectorsNeeded = size / SECTOR_SIZE;
    int remainder = size % SECTOR_SIZE;
    if (remainder != 0) { numberOfSectorsNeeded++; }
    
    int fileSectorIndex;
    for (fileSectorIndex = 0; fileSectorIndex < numberOfSectorsNeeded; fileSectorIndex++) {
        int sectorOffset = fileSectorOffset + fileSectorIndex * SECTOR_SIZE;
        int writeSectorResult = write_sector(fileSectorIndex, buffer + sectorOffset);
        if (!writeSectorResult) {
            return writeSectorResult;
        }
    }
    
	return 0;
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
