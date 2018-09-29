TARGET = libar8020
SRC_C=libar8020.c
CFLAGS=-fPIC -shared

OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

all:$(TARGET)

$(TARGET): $(clean) $(OBJ_C)
	@$(CC) $^ $(CFLAGS) -o $(TARGET).so

%.o:%.c
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(TARGET).so -f

.PHONY: all clean $(TARGET)