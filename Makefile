CC = gcc
XX = g++
CFLAGS = -Wall -O3 -g -fPIC
TARGET = rtsp_stream_2_ws

INC_PATH = -I . -I /usr/local/include
LIBS_PATH = -L /usr/local/lib
LIBS = -lavutil -lavcodec -lavformat -lavdevice -lswresample -lswscale -lpthread -lwebsockets

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PATH)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_PATH)

SOURCES = $(wildcard *.c *.cpp)
$(warning "sources $(SOURCES)")
OBJS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET) : $(OBJS)
	$(XX) $(OBJS) -o $@ $(LIBS) $(LIBS_PATH)
clean:
	rm -rf *.o $(TARGET)

