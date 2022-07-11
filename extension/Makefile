# Cross-compilation on Linux:

# CFLAGS   =  -I/usr/include -g `sdl2-config --cflags`
# LDFLAGS  = `sdl2-config --libs` -lSDL2_gfx -lSDL2_ttf -lm
# PROG = tetris
# # Use whichever is available on the system
# CC = arm-linux-gnueabi-gcc
# # CC = arm-none-eabi-gcc

# OBJS   = input.o graphics.o tetris.o shuffle.o sds.o menu.o main.o 

# # Top-level rule to create the program.
# all: $(PROG)

# # Compiling other source files.
# %.o: %.c %.h defs.h
# 	$(CC) $(CFLAGS) -c $<

# # Linking the program
# $(PROG): $(OBJS)
# 	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

# # Cleaning everything that can be automatically recreated with "make"
# clean:
# 	rm $(PROG) *.o

# Compilation on Raspberry Pi

CFLAGS   = -g `sdl2-config --cflags`
LDFLAGS  = `sdl2-config --libs` -lSDL2_gfx -lSDL2_ttf -lm
PROG = tetris
CC = gcc

OBJS   = input.o graphics.o tetris.o shuffle.o sds.o menu.o main.o 

# Top-level rule to create the program.
all: $(PROG)

# Compiling other source files.
%.o: %.c %.h defs.h
	$(CC) $(CFLAGS) -c $<

# Linking the program
$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LDFLAGS)

# Cleaning everything that can be automatically recreated with "make"
clean:
	rm $(PROG) *.o
