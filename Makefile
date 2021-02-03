INC_DIR = ./include
OTHER_DIR = ./third_party
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN_DIR = ./bin

LINK_INC += -I third_party/xgboost/include
LINK_INC += -I third_party/xgboost/rabit/include

LINK_LIB += ./third_party/xgboost/lib/libxgboost.a
LINK_LIB += ./third_party/xgboost/rabit/lib/librabit.a
LINK_LIB += ./third_party/xgboost/dmlc-core/libdmlc.a

SRC = $(wildcard ${SRC_DIR}/*.cpp)
OBJ = $(patsubst %.cpp, ${OBJ_DIR}/%.o, $(notdir ${SRC}))
LIBS = -lpthread -fopenmp

TARGET = main
BIN_TARGET = $(BIN_DIR)/${TARGET}

CC = g++ -std=c++11
CXXFLAGS = -g -Wall -D_REENTRANT -IS{INC_DIR} -IS{OTHER_DIR}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -o $@ $(LIBS) $(LINK_INC) $(LINK_LIB)

${OBJ_DIR}/%.o: ${SRC_DIR}/%.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@ ${LINK_INC}

clean:
	/bin/rm -rf $(OBJ_DIR)/*.o
	/bin/rm -rf $(BIN_DIR)/*
	/bin/rm -rf data/*
