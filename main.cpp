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
	lcGlitch.writeImgOut("poo.tiff");
	return 0;
}

