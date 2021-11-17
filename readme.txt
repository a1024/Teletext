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
- Non-printable characters & extended ASCII
- Tab marker
- Drag selection
- Split view
- Compare files
- Text wrap
- Status bar
- Go to previous/next cursor location
- Find & replace
- Go to line number
- Syntax highlighting for C/C++
- State file
- Config file
- Save on write

DONE:
- Rectangular selection
- Port to Linux
- Multiple tabs
- Tab bar/sidebar
- Drag tab to reorder
- Indentation with tab/shift tab
