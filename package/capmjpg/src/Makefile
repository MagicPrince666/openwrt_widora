#CXX = mips-openwrt-linux-musl-g++
#CC = mips-openwrt-linux-musl-gcc

TARGET	= capmjpg

DIR		= .
INC		= -I.
CFLAGS	= -Wall -std=c++11

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.cpp))

OBJS	= $(patsubst %.cpp,%.o,$(FILES))

all:$(OBJS) $(TARGET)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CXX) -o $@ $(OBJPATH)/*.o

$(OBJPATH):
	mkdir -p $(OBJPATH)

clean:
	-rm $(OBJPATH)/*.o
	-rm $(TARGET)