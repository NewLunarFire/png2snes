.include "Header.inc"
.include "Snes_Init.asm"

VBlank:    ; Needed to satisfy interrupt definition in "header.inc"
  rti

Start:
  ; Initialize the SNES.
  Snes_Init

  sep #$20 ; Set the A register to 8-bit.
  rep #$10 ; Set the X and Y registers to 16-bit

  jsr DMAPalette ; Load Palette in CGRAM
  jsr DMATiles ; Load Tiles in VRAM

  stz $2105           ; Set Video Mode 0, 8x8 tiles, 4 color BG1/BG2/BG3/BG4
  lda #$04            ; Set BG1's Tile Map offset to $0400 (Word address)
  sta $2107           ; And the Tile Map size to 32x32
  stz $210B           ; Set BG1's Character VRAM offset to $0000 (word address)

  lda #$01            ; Enable BG1
  sta $212C

  stz $210E
  stz $210E

CLEARTILEMAP:
  lda #$80
  sta $2115 ; Set Word Write mode to VRAM (increment after $2119)
  ldx #$0400
  stx $2116
  lda #$09
  sta $4300 ; Set DMA Mode (word, no increment)
  lda #$18
  sta $4301 ; Write to VRAM ($2118)
  ldx #ZERO ; Load Zero Address
  stx $4302 ; Write DMA Source Address
  stz $4304 ; Take it from bank 0
  ldx #1024
  stx $4305 ; 1024 Bytes to transfer

  lda #$01
  sta $420B ; Initiate VRAM DMA Transfer

  ; Now, load up some data into our tile map
  ldx #$0400 + (15*32) + 10
  stx $2116
  ldx #0

GETNEXTCHAR:
  lda TEXT.W,X ; Load next character
  beq TURNON ; If it's a 0, quit
  tay
  sty $2118 ; Store result in next OAM table
  inx ; Go to next character
  jmp GETNEXTCHAR

TURNON:
  lda #15
  sta $2100 ; Set brightness to 15 (100%)

  ; Loop forever.
Forever:
  wai
  jmp Forever

DMAPalette:
  stz $2121 ; Set CGRAM Address to 0
  stz $4300 ; Set DMA Mode (byte, increment)
  lda #$22
  sta $4301 ; Write to CGRAM ($2122)
  ldx #PALETTE
  stx $4302 ; Write Source Address
  stz $4304 ; Take from Bank 0
  ldx #4
  stx $4305 ; 4 Bytes to transfer (2 colors)

  lda #$01
  sta $420B ; Initiate CGRAM DMA Transfer

  rts

DMATiles:
  lda #$80
  sta $2115 ; Set Word Write mode to VRAM (increment after $2119)
  stz $2116
  stz $2117 ; Set VRAM Address to 0
  lda #$01
  sta $4300 ; Set DMA Mode (word, increment)
  lda #$18
  sta $4301 ; Write to VRAM ($2118)
  ldx #TILES ; Load Tile Address
  stx $4302 ; Write DMA Source Address
  stz $4304 ; Take it from bank 0
  ldx #2048
  stx $4305 ; 2048 Bytes to transfer

  lda #$01
  sta $420B ; Initiate VRAM DMA Transfer

  rts


TEXT:
  .db "Hello World!"

ZERO:
  .db 0

PALETTE:
  .incbin "font.cgr"

TILES:
  .incbin "font.vra"
