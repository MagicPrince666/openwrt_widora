TARGET=sample_upgrade
SRC_C=sample_upgrade.c
CFLAGS=-lpthread -lar8020
OBJ_C   := $(patsubst %.c,%.o,$(SRC_C))

all:$(TARGET)

$(TARGET): $(SRC_C)
	@$(CC) $^ $(CFLAGS) -o $(TARGET)
	
clean:
	@rm $(TARGET) -f

.PHONY: all clean $(TARGET)
