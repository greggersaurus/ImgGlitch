/////////////////////////////////////////////////////////////////////////////
// \file 	ImgGlitch.cpp
// \author 	Gregory Gluszek
// \date	12/17/11
// \brief	Class for messing up a perfectly good image.
///////////////////////////////////////////////////////////////////////////////

#include "ImgGlitch.h"

/**
 * Constructor.
 */
tcImgGlitch::tcImgGlitch()
: mpRasterIn(NULL)
, mpRasterChunk(NULL)
, mnChunkX(0)
, mnChunkY(0) 
, mnChunkWidth(0)
, mnChunkHeight(0)
, mpRasterOut(NULL)
{

}

/**
 * Desctructor.
 */
tcImgGlitch::~tcImgGlitch()
{
}

/**
 * Specify the input image to be messed with.
 * 	
 * @param apFilename The name of the image file to open.
 *	
 * @return 0 on success.
 */
int 
tcImgGlitch::openImgIn(char* apFilename)
{
	return -1;
}

/**
 * Specify and write the raster data to the output file.	
 *
 * @param apFilename The name of the file to be written. If exists
 *	file will be overwritten.
 *
 * @return 0 on sucess.
 */
int 
tcImgGlitch::writeImgOut(char* apFilename)
{
	return -1;
}

/**
 * Pull a chunk of data from the input image for processing.
 *
 * @param anX The X pixel location of the upper left corner of the
 * 	image chunk to process.
 * @param anY The Y pixel location of hte upper left corner of the 
 * 	image chunk to process.
 * @param anWidth The width, in pixels, of the chunk to process.
 * @param anHeight The height, in pixels of hte chunk to process.
 * @param abCut Set to true if you want the chunk to be removed
 * 	from the original image as opposed to just being copied.
 * 
 * @return 0 on success.
 */
int 
tcImgGlitch::pullChunk(uint32_t anX, uint32_t anY, uint32_t anWidth, 
	uint32_t anHeight, bool abCut)
{
	return -1;
}

/**
 * Flip the chunk being process.
 *
 * @param abHoriz If true chunk is flipped horizontally.
 * @param abVert If true chunk is flipped vertically.
 *
 * @return 0 on sucess.
 */
int 
tcImgGlitch::flipChunk(bool abHoriz, bool abVert)
{
	return -1;
}

int 
tcImgGlitch::corruptChunk()
{
	return -1;
}

/**
 * Paste chunk into output raster buffer.
 *
 * @param anTransX Translation in X direction from original chunk 
 *	location.
 * @param anTransY Translation in Y direction from original chunk
 *	location.
 * @param aeCombMethod Combination method for writing to output raster.
 *
 * @return 0 on sucess.
 */
int 
tcImgGlitch::pastChunk(uint32_t anTransX, uint32_t anTransY, 
	teCombMethod aeCombMethod)
{
	return -1;
}

