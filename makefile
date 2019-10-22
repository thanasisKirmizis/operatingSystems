CC = gcc
CFLAGS  = -g -Wall
RM = rm -f
TARGET = myshell

all: $(TARGET)

# final link for executable
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

# clean temporary files
clean:
	$(RM) *.o *~