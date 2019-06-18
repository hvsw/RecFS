
CC=gcc -g -w
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./teste
EXM_DIR=./exemplo


all: objetos mvObjetos libt2fs.a mvLib shell

objetos: $(SRC_DIR)/t2fs.c $(INC_DIR)/t2fs.h
	$(CC) -c $(SRC_DIR)/t2fs.c -Wall

mvObjetos:
	mv *.o $(BIN_DIR)

libt2fs.a: 
	ar crs libt2fs.a $(BIN_DIR)/*.o $(LIB_DIR)/*.o

mvLib:
	mv *.a $(LIB_DIR)

shell:
	$(CC) -o t2shell $(EXM_DIR)/t2shell.c -L$(LIB_DIR) -lt2fs

clean:
	rm -rf t2shell $(LIB_DIR)/*.a $(BIN_DIR)/t2fs.o  $(TST_DIR)/*.o $(SRC_DIR)/*~
