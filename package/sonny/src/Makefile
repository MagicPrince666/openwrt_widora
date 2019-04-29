TARGET	= sonny_ps2 

DIR		= . 
INC		= -I. 
CFLAGS	= -Wall
LDFLAGS = 

OBJPATH	= .

FILES	= $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))

OBJS	= $(patsubst %.c,%.o,$(FILES))

all:$(OBJS) $(TARGET)

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c -o $(OBJPATH)/$(notdir $@) $< 

$(TARGET):$(OBJPATH)
	$(CC) -o $@ $(OBJPATH)/*.o $(LDFLAGS)

clean:
	-rm $(OBJPATH)/*.o
	-rm $(TARGET)
	