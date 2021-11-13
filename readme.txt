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
- Tab bar/sidebar
- Split view
- Indentation with tab/shift tab
- Wrap option
- Status bar
- Go to previous/next cursor location
- Find & replace
- Optional syntax highlighting for C/C++
- State file
- Config file
- Save on write

DONE:
- Rectangular selection
- Port to Linux
- Multiple tabs
