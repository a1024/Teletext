Teletext: An ASCII text editor.

Intended for source code editing.


The font texture is a replacable PNG file:
	- Texture name should be 'font.PNG'
	- Black is inside, otherwise outside.
	- Texture dimensions should be a POWER OF TWO.
	- Should have the character size in the first two pixels:

For example an 8x12 font texture starts with the two pixels:
r	g	b	r	g	b
8	8	8	12	12	12


An optional Signed Distance Field (SDF) font texture can be generated
using sdf_generator.cpp:
	- Texture name should be 'font_sdf.PNG'
	- It should be a single-channel texture
	- The first 14 bytes are:

SDF_slope: An 8 byte double precision slope of the signed distance field
b0, b1:	(x, y) grid start
b2, b3:	(x, y) grid stride
b4, b5:	(x, y) character dimensions


TODO:
- Cursor should remember column value when changing line
- Syntax highlighting for C/C++
- Bracket matching - green: matched, red: unmatched
- Region folding
- Find & replace
- Tab & space character marks
- Non-printable characters & extended ASCII
- Text wrap
- Toggle display line numbers
- Go to line number
- Line operations: sort lines, ...etc
- Split view
- Compare files
- Status bar
- Go to previous/next cursor location
- State file to save session
- Config file to save settings
- Save on write
- Hex mode
- Change tracking

DONE:
- Rectangular selection
- Port to Linux
- Multiple tabs
- Tab bar/sidebar
- Drag tab to reorder
- Indentation with tab/shift tab
- Drag selection
- Reimplement text as vector<string>
- Reimplement text history
- Signed Depth Field rendering
- Change case
