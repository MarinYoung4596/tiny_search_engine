INC_DIR = ./include
SEG_DIR = ./jieba
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

SRC = $(wildcard ${SRC_DIR}/*.cpp)
OBJ = $(patsubst %.cpp, ${OBJ_DIR}/%.o, $(notdir ${SRC}))
LIBS = -lpthread

TARGET = main
BIN_TARGET = $(BIN_DIR)/${TARGET}

CC = g++ -std=c++11
CXXFLAGS = -g -Wall -D_REENTRANT -IS{INC_DIR} -IS{SEG_DIR}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBS)

${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	/bin/rm -rf $(OBJ_DIR)/*.o
	/bin/rm -rf $(BIN_DIR)/*
	/bin/rm -rf data/*
