Teletext: An ASCII text editor.

Intended for source code editing.


The font texture is a replacable PNG file with the following properties:
	- Black = inside, otherwise = outside.
	- Texture dimensions should be a POWER OF TWO.
	- Should have the character size in the first two pixels:

For example: an 8x12 font texture starts with the two pixels:
r	g	b	r	g	b
8	8	8	12	12	12


TODO:
- Syntax highlighting for C/C++
- Text wrap
- Find & replace
- Go to line number
- Change case
- Tab character mark
- Non-printable characters & extended ASCII
- Line operations: sort lines, ...etc
- Bracket matching: green: matched, red: unmatched
- Split view
- Compare files
- Status bar
- Go to previous/next cursor location
- Region folding
- State file
- Config file
- Save on write
- Hex mode

DONE:
- Rectangular selection
- Port to Linux
- Multiple tabs
- Tab bar/sidebar
- Drag tab to reorder
- Indentation with tab/shift tab
- Drag selection
- Reimplement with vector<string>
