TARGET1	= tcp_transfer_s
TARGET2	= tcp_transfer_c

DIR		= .
INC		= -I. 
CFLAGS	= -Wall
LDFLAGS = 

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:$(OBJS) $(TARGET1) $(TARGET2)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET1):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/tcp_transfer_s.o md5.o $(LDFLAGS)

$(TARGET2):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/tcp_transfer_c.o md5.o $(LDFLAGS)

clean:
	-rm $(OBJPATH)/*.o
	-rm $(TARGET1) $(TARGET2)
	