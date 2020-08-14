
CC=gcc
CXX=g++
#CXX=arm-linux-gnueabi-g++
BIN=main
LINKOBJ=cJSON.o ocppUtils.o ocppProtocl.o easywsclient.o ocppclient.o test.o msgQueue.o httpClient.o
OBJ=cJSON.o ocppUtils.o ocppProtocl.o easywsclient.o ocppclient.o test.o msgQueue.o httpClient.o

all:$(BIN)

$(BIN):$(OBJ)
	$(CXX) $(LINKOBJ) -o $(BIN) -lpthread

clean:
	rm $(OBJ) $(BIN)

