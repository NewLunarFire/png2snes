AS=wla-65816
ASFLAGS=-v -o

LD=wlalink
LDFLAGS=-v -r

PNG2SNES=../../png2snes
PNG2SNESFLAGS=--bitplanes 2 --tilesize 8 --binary
TARGET=Hello

PNG=font.png
SRC=$(TARGET).asm
INC=Header.inc $(PNG:.png=.cgr) Snes_Init.asm

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
	$(AS) $(ASFLAGS) $@ $<
	echo $@ >> $(LINK)

font.cgr: font.png
	$(PNG2SNES) $(PNG2SNESFLAGS) $< --output=font

clean:
	rm $(OBJ) $(LINK) $(SMC) $(PNG:.png=.cgr) $(PNG:.png=.vra)

rmlink:
	rm $(LINK)
