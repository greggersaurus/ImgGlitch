/////////////////////////////////////////////////////////////////////////////
// \file 	main.cpp
// \author 	Gregory Gluszek
// \date	12/17/11
// \brief	Entry point for ImgGlitch command line.
///////////////////////////////////////////////////////////////////////////////

#include "ImgGlitch.h"

#include <stdio.h>

int main(int argc, char* argv[])
{
	tcImgGlitch lcGlitch;

	lcGlitch.openImgIn("test.tiff", true);
	lcGlitch.pullChunk(0, 0, 100, 100, false);
	lcGlitch.pastChunk(0, 0, tcImgGlitch::eeOr);
	lcGlitch.pastChunk(100, 0, tcImgGlitch::eeOr);
	lcGlitch.writeImgOut("poo.tiff");
	return 0;
}

