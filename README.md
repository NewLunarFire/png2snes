# PNG to SNES Graphics converter

This project is to be able to generate CGRAM and VRAM data for Super Nintendo
from Color-Indexed (e.g. that uses a palette) PNG files.

## Usage
`png2snes FILE`

That's about it. The program will return different errors along the way if
there are problems with the supplied PNG file.

## Compiling
`make` and you're in business.

## Creating Color-Indexed PNGs
In GIMP, select Image -> Mode -> Indexed...
In Photoshop, Select Image -> Mode -> Index Color

## Technical info
The PNG specification defines a PLTE chunk. This PLTE chunk is a list of
24bpp RGB colors that constitutes the palette. The max size of the palette is
256 colors.

We fetch the palette using libpng and convert the colors to the 15bpp BGR
format used by the SNES. This data can then after be loaded in CGRAM on the
SNES.
