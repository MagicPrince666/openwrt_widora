#CPP = g++

TARGET1	= http_server
TARGET2	= https_server 

DIR		= . 
INC		= -I.
CFLAGS	= -std=c++11 -O3 -g -Wall -Wextra -Wsign-conversion
LDFLAGS = -lboost_thread -lboost_filesystem -lboost_system -lssl -lcrypto -lz -lrt

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:$(OBJS) $(TARGET1) $(TARGET2)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET1):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/http_examples.o $(LDFLAGS)

$(TARGET2):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/https_examples.o $(LDFLAGS)

$(OBJPATH):
	mkdir -p $(OBJPATH)

clean:
	-rm $(OBJPATH)/*.o
	-rm $(TARGET1) $(TARGET2)