/////////////////////////////////////////////////////////////////////////////
// \file 	ImgGlitch.cpp
// \author 	Gregory Gluszek
// \date	12/17/11
// \brief	Class for messing up a perfectly good image.
///////////////////////////////////////////////////////////////////////////////

#include "ImgGlitch.h"

#include <stdio.h>
#include <string.h>

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
	if (NULL != mpRasterIn)
		delete mpRasterIn;
	if (NULL != mpRasterChunk)
		delete mpRasterChunk;
	if (NULL != mpRasterOut)
		delete mpRasterOut;
}

/**
 * Specify the input image to be messed with.
 * 	
 * @param abClear If true wipe the output raster to 0. If false
 *	either allocate memory for palette and don't clear it or if
 *	palette is open from before leave it unmolested.
 *	
 * @return 0 on success.
 */
int 
tcImgGlitch::openImgIn(const char* apFilename, bool abClear)
{
	TIFF* lpTiffIn = TIFFOpen(apFilename, "r");
	char laErrorMsg[1024];

	// Make sure files were opened correctly
	if (!lpTiffIn)
	{
		printf("Error opening input image file \'%s\'.\n", apFilename);
		return -1;
	}

	// Get the image details
	if (!TIFFRGBAImageBegin(&msImgDetailsIn, lpTiffIn, 0, laErrorMsg))
	{
		printf("Error getting input image details: %s", laErrorMsg);
		TIFFClose(lpTiffIn);
		return -1;
	}

	// Clean up old raster data if required
	if (NULL != mpRasterIn)
		delete mpRasterIn;
	
	// Allocate new raster data
	mpRasterIn = (uint32_t*)_TIFFmalloc(msImgDetailsIn.width * 
		msImgDetailsIn.height * sizeof(uint32_t));
	if (NULL == mpRasterIn)
	{
		printf("Error allocating input image raster buffer.\n");
		TIFFClose(lpTiffIn);
		return -1;
	}

	// Attempt to get the data in RGBA format
	if (!TIFFRGBAImageGet(&msImgDetailsIn, mpRasterIn, msImgDetailsIn.width,
		msImgDetailsIn.height))
	{
		printf("Failed to get input image raster data.\n");
		TIFFClose(lpTiffIn);
		return -1;
	}

	TIFFRGBAImageEnd(&msImgDetailsIn);
	TIFFClose(lpTiffIn);

	// Allocate output raster if required
	if (NULL == mpRasterOut || abClear)
	{
		if (abClear && NULL != mpRasterOut)
			delete mpRasterOut;

		// Allocate new raster data
		mpRasterOut = (uint32_t*)_TIFFmalloc(msImgDetailsIn.width * 
			msImgDetailsIn.height * sizeof(uint32_t));
		if (NULL == mpRasterOut)
		{
			printf("Error allocating output img raster buffer.\n");
			return -1;
		}
		
		// Copy over image details
		memcpy(&msImgDetailsOut, &msImgDetailsIn, 
			sizeof(msImgDetailsIn));
	}

	// Clear out output raster if desired
	if (abClear)
		memset(mpRasterOut, 0, sizeof(uint32_t) * 
			msImgDetailsOut.width * msImgDetailsOut.height);

	return 0;
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
tcImgGlitch::writeImgOut(const char* apFilename)
{
//TODO: Begin Debug to be removed
	memcpy(mpRasterOut, mpRasterIn, msImgDetailsOut.width
		* msImgDetailsOut.height * sizeof(uint32_t));
//TODO: End Debug

	// Open output file
	TIFF* lpTiffOut = NULL;	
	uint32_t lnLineBytes = 0;
	uint32_t lnNumPixels = 0;

	// Make sure there is data to write
	if (NULL == mpRasterOut)
	{
		printf("Output raster buffer is NULL. Nothing to write.\n");
		return -1;
	}

	// Attempt to open input file 
	lpTiffOut = TIFFOpen(apFilename, "w");	
	if (NULL == lpTiffOut)
	{
		printf("Error opening output file \'%s\'.\n", apFilename);
		return -1;
	}

	// Setup the new image size
	lnNumPixels = msImgDetailsOut.width * msImgDetailsOut.height;

	// Set parameter for the output file
	lnLineBytes = 4 * msImgDetailsOut.width;
	TIFFSetField(lpTiffOut, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(
		lpTiffOut, lnLineBytes));
	TIFFSetField(lpTiffOut, TIFFTAG_IMAGEWIDTH, msImgDetailsOut.width);
	TIFFSetField(lpTiffOut, TIFFTAG_IMAGELENGTH, msImgDetailsOut.height);
	TIFFSetField(lpTiffOut, TIFFTAG_SAMPLESPERPIXEL, 4);   
	TIFFSetField(lpTiffOut, TIFFTAG_BITSPERSAMPLE, 8); 
	TIFFSetField(lpTiffOut, TIFFTAG_ORIENTATION, 
		ORIENTATION_TOPLEFT); 
	TIFFSetField(lpTiffOut, TIFFTAG_PLANARCONFIG, 
		PLANARCONFIG_CONTIG);
	TIFFSetField(lpTiffOut, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	// Write the data to the output file
	for(uint32_t row_cnt = 0; row_cnt < msImgDetailsOut.height; ++row_cnt)	
	{
		if (TIFFWriteScanline(lpTiffOut, 
			&mpRasterOut[(msImgDetailsOut.height - row_cnt - 1)
			* msImgDetailsOut.width], row_cnt, 0)<0)
		{
			printf("Error writing scanline.\n");
			break;
		}
	}

	TIFFClose(lpTiffOut);
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

