Teletext: An ASCII text editor.

Intended for source code editing.


The font texture is a replacable PNG file with the following properties:
	- Black = inside, otherwise = outside.
	- Texture dimensions should be a power of two.
	- Should have the character size in the first two pixels:

For example: an 8x12 font texture starts with the two pixels:
r	g	b	r	g	b
8	8	8	12	12	12


TODO:
- Status bar
- Find & replace
- Optional syntax highlighting for C/C++
- Config file
- Multiple tabs
- Save on write
- Port to Linux

DONE:
- Rectangular selection
