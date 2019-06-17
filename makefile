#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
EXM_DIR=./exemplo

all: fs
	ar crs $(LIB_DIR)/libt2fs.a  $(BIN_DIR)/t2fs.o $(LIB_DIR)/apidisk.o

fs:
	$(CC) -c $(SRC_DIR)/t2fs.c -o $(BIN_DIR)/t2fs.o -Wall

shell: all
	$(CC) -o t2shell $(EXM_DIR)/t2shell.c -L$(LIB_DIR) -lt2fs.a -Wall

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(EXM_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
