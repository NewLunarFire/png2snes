# PNG to SNES Graphics converter

This project is to be able to generate CGRAM and VRAM data for Super Nintendo
from Color-Indexed (e.g. that uses a palette) PNG files.

## Usage
`png2snes [OPTIONS] FILE`

That's about it. The program will return different errors along the way if
there are problems with the supplied PNG file.

Options:

* --tilesize: Specify if tiles should be 8x8, 16x16 or 32x32.
* --bitplanes: Number of bits per pixel in the output. Defaults to the least bitplanes required from the PNG bit depth.
* --output=BASENAME: Basename of the output file (instead of stdout). In text mode, generates BASENAME_cgram.asm for the palette and BASENAME_vram.asm for the tiles.
* --binary: Outputs file to binary format. Generates BASENAME.cgr for the palette and BASENAME.vra for the tiles.
* --verbose: Verbose mode (default), print diagnostic information in stderr
* --quiet: No diagnostic output to stderr

## Compiling
`make` and you're in business. The project depends only on libpng.

## Example
An Hello World example is provided in the example/Hello World folder. You need WLA-DX 9.6 installed. Again, `make` and you're in business.

## Why use PNG for making SNES graphics?
There are multiple reasons for using PNG to create your SNES tiles and sprites before converting them to CGRAM and VRAM data:

* The format is lossless. Formats like JPEG are not well suited for game graphics because the color information changes slightly evertime you save introducing artifacts. But you should know this already.
* It supports 24-bit colors. Both in indexed and non-indexed modes. The SNES only has 15-bit color, so a narrowing conversion has to be done, but 24-bit is a standard.
* It supports transparency. The SNES uses color 0 as the transparency color, so I intend to map transparent pixels to this color in the future. This means that you don't have to find a weird color and map that to color 0 in your palette for the pixels that will be transparent.
* It is widely used and implemented. All desktop managers and browsers support PNG out-of-the-box, and most if not all graphic editors can save to PNG. So it doesn't matter if you draw your images in Gimp, Paint.NET or GraphicsGale, you can save it as PNG. Also, you can already find spritesheets in PNG on the net.
* It supports using a palette. This means that you can control which colors you use and limit yourself to a narrow range of colors if you use 4-colors or 16-colors mode on the SNES, and also use colors that won't overlap after the 24-bit to 15-bit narrowing conversion.
* It is compressed. You can save your spritesheets in a compressed lossless format, and the program is able to inflate your format and generate your sprites at build time.
* It has a polyvalent format. The format requires certain mandatory chunks to be present, but you can add ancillary chunks and the program will just ignore them. So you can add additional information in those chunks without worrying about png2snes not understanding them.

## Creating Color-Indexed PNGs
In GIMP, select Image -> Mode -> Indexed...

In Photoshop, Select Image -> Mode -> Index Color
