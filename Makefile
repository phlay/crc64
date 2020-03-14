USE_ASM=yes

CFLAGS = -pedantic -Wall -O2
LDFLAGS =

AS = nasm
ASFLAGS = -f elf64

OBJ = main.o map.o
SPEED_OBJ = speed.o


ifeq ($(USE_ASM), yes)
	LDFLAGS += -no-pie
	OBJ += crc64.o
	SPEED_OBJ += crc64.o
else
	OBJ += lookup.o crc64-lookup.o
	SPEED_OBJ += lookup.o crc64-lookup.o
endif


.SUFFIXES: .asm
.PHONY: all clean cleanall


%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

all: crc64

clean:
	rm -f *.o crc64 gentbl speed

cleanall: clean
	rm -f lookup.c

crc64: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

gentbl: gentbl.o
	$(CC) -o $@ gentbl.o

# re-generate lookup.c if needed
lookup.c: gentbl.c
	$(CC) -o gentbl gentbl.c
	./gentbl > lookup.c

speed: $(SPEED_OBJ)
	$(CC) -o $@ $(SPEED_OBJ) $(LDFLAGS)
