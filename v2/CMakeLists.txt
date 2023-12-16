# CMakeList.txt : CMake project for Teletext, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (teletext WIN32
	console.c
	graphics.c
	teletext.c
	text.c
	util.c
	window.c
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET teletext PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
