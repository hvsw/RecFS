
/**
*/
#include "../include/t2fs.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "sys/fcntl.h"
#include "../include/apidisk.h"


#define MAX_FILES 1000
char *files[MAX_FILES];

#define SECTOR_SIZE 256
#define SECTORS_PER_BLOCK 256
// TODO: Esse valor talvez seja um pouco grande,
// foi escolhido esse pois geralmente arquivos de audio
// em gravação tem um tamanho consideravel, sendo dificil
// encontrar arquivos muito pequenos, acredito que poderiamos
// ate aumentar pra sei la 1MB.
#define BLOCK_SIZE SECTOR_SIZE * SECTORS_PER_BLOCK  // 64KB

#define BLOCK_COUNT 100

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

static unsigned char readSector(unsigned char *buffer) {
    if (read_sector(0, buffer)) {
        printf("Erro ao ler o superbloco. O arquivo '%s' esta no caminho certo?\n", DISK_FILE);
        exit(-1);
    }
    return buffer;
}

void populateSuperblockWith(BYTE *buffer) {
    superblock->version = *((WORD *)buffer);
    if (superblock->version != VERSION) {
        printf("Versão do sistema de arquivos não suportada!\nEsperado: %d\nEncontrado: 0x%hu\n", VERSION, superblock->version);
        exit(-2);
    }
    
    superblock->sectorSize = *((WORD *)(buffer + 2));
    superblock->partitionTableStart = *((WORD *)(buffer + 4));
    superblock->partitionCount = *((WORD *)(buffer + 6));
    superblock->partitionStart = *((DWORD *)(buffer + 8));
    superblock->partitionEnd = *((DWORD *)(buffer + 12));
    strncpy(superblock->partitionName, buffer+16, 24);
}

void readSuperblock() {
    BYTE buffer[SECTOR_SIZE];
    readSector(buffer);
    populateSuperblockWith(buffer);
}

static void init_t2fs() {
    superblock = (t2fs_disk*) malloc(sizeof(*superblock));
    readSuperblock();
    if (debug) {
        showSuperblockInfo();
    }
}

void superblockToBuffer(unsigned char *buffer) {
    memset(buffer, 0, sizeof(buffer));
    
    sprintf(buffer,    "0x%x", VERSION);
    sprintf(buffer+2,  "0x%x", SECTOR_SIZE);
    sprintf(buffer+4,  "0x%x", superblock->partitionTableStart);
    sprintf(buffer+6,  "0x%x", superblock->partitionCount);
    sprintf(buffer+8,  "0x%x", superblock->partitionStart);
    sprintf(buffer+12, "0x%x", superblock->partitionEnd);
    sprintf(buffer+16, "0x%x", superblock->partitionName);
}

int _format2() {
    int superblockSize = 40, dataAreaSize = BLOCK_SIZE, dirTreeSize, bitmapSize;
    int diskSize = superblockSize + dataAreaSize + dirTreeSize + bitmapSize;
    superblock->version = VERSION; // just in case...
    superblock->sectorSize = 0x100; // 0d256
    superblock->partitionTableStart = 0x8;
    superblock->partitionCount = 0x1;
    superblock->partitionStart = 0x28; // 0d40
    
    int lastBlockAddress = diskSize - superblock->partitionStart - bitmapSize - dirTreeSize;
    superblock->partitionEnd = lastBlockAddress;
    
    int partitionNameSize = 24;
    unsigned char buffer[partitionNameSize];
    strncpy("PART_SEM_QUE_VEM", superblock->partitionName, sizeof(buffer));
    
    unsigned char superblockData[SECTOR_SIZE];
    superblockToBuffer(superblockData);
    
    int writeResult = write_sector(0, superblockData);
    if (writeResult < 0) {
        return writeResult;
    }
    
    return 0;
}

int wipeDisk() {
    unsigned char zeroFilledArray[SECTOR_SIZE];
    memset(zeroFilledArray, 0, SECTOR_SIZE);
    
    int diskSize = 0;
    int numberOfSectors = diskSize/SECTOR_SIZE;
    int currentSector = 0;
    for (currentSector = 0; currentSector < numberOfSectors; currentSector++) {
        int writeResult = write_sector(currentSector, zeroFilledArray) < 0;
        if (writeResult < 0) {
            return writeResult;
        }
    }
    
    return 0;
}


int format2 (int sectors_per_block) {
	if (!diskInitialized) {
        init_t2fs();
        diskInitialized = 1;
    }
    
    int wipeDiskResult = wipeDisk();
    if (wipeDiskResult < 0) {
        return wipeDiskResult;
    }
    
    int formatResult = _format2();
    if (formatResult < 0) {
        return formatResult;
    }
    
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
