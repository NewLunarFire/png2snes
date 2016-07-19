AS=wla-65816
ASFLAGS=-o

LD=wlalink
LDFLAGS=-vr

PNG2SNES=../../png2snes
PNG2SNESFLAGS=--bitplanes 2 --tilesize 8
TARGET=Hello

PNG=font.png
SRC=$(TARGET).asm
INC=Header.inc $(PNG:.png=.asm) Snes_Init.asm

OBJ=$(TARGET).obj
LINK=$(TARGET).link
SMC=$(TARGET).smc

.PHONY: rmlink

all: $(SMC)

$(SMC): $(INC) $(LINK) $(OBJ)
	$(LD) $(LDFLAGS) $(LINK) $(SMC)

$(LINK):
	echo '[objects]' > $(LINK)

$(OBJ): $(SRC) $(INC)
	$(AS) $(ASFLAGS) $< $@
	echo $@ >> $(LINK)

font.asm: font.png
	$(PNG2SNES) $(PNG2SNESFLAGS) $< > $@

clean:
	rm $(OBJ) $(LINK) $(SMC) $(PNG:.png=.asm)

rmlink:
	rm $(LINK)
