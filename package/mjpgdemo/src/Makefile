#CXX = mips-openwrt-linux-musl-g++
#CC = mips-openwrt-linux-musl-gcc

TARGET	= mipgrecord

DIR		= .
INC		= -I.
CFLAGS	= -Wall

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

OBJS	= $(patsubst %.c,%.o,$(FILES))

all:$(OBJS) $(TARGET)

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CC) -o $@ $(OBJPATH)/*.o

$(OBJPATH):
	mkdir -p $(OBJPATH)

clean:
	-rm $(OBJPATH)/*.o
	-rm $(TARGET)