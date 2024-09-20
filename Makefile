TARGET = m-vba

CROSS_COMPILE =

CC = $(CROSS_COMPILE)gcc

OBJS = gba.o memory.o sound.o system.o sound_output.o core.o input.o \
	font_drawing.o menu.o scaler.o video_blit.o

VPATH = ./src

CFLAGS = -Iinclude -Ofast -flto -fno-common -Wall -std=gnu++11 \
	-DLSB_FIRST -DWANT_16BPP -DFRONTEND_SUPPORTS_RGB565 \
	-DNDEBUG -DWANT_STEREO_SOUND -DVIRTUAL_SURFACE \
	`/usr/bin/sdl-config --cflags`

ASFLAGS = 

LDFLAGS = -lc -lgcc -lm -lSDL -lasound -lz -flto -Wl,--as-needed -Wl,--gc-sections 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	@echo Done!!!

%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(TARGET)