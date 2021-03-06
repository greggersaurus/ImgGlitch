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
	// Clean up any raster data we may have allocated
	if (NULL != mpRasterIn)
		_TIFFfree(mpRasterIn);
	if (NULL != mpRasterChunk)
		delete [] mpRasterChunk;
	if (NULL != mpRasterOut)
		_TIFFfree(mpRasterOut);
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
 * @param anX The X pixel location of the bottom left corner of the
 * 	image chunk to process.
 * @param anY The Y pixel location of hte bottom left corner of the 
 * 	image chunk to process.
 * @param anWidth The width, in pixels, of the chunk to process. Set to
 * 	max size to grab entire input image.
 * @param anHeight The height, in pixels of hte chunk to process. Set to
 * 	max size to grab entire input image
 * @param abCut Set to true if you want the chunk to be removed
 * 	from the original image as opposed to just being copied.
 * 
 * @return 0 on success.
 */
int 
tcImgGlitch::pullChunk(uint32_t anX, uint32_t anY, uint32_t anWidth, 
	uint32_t anHeight, bool abCut)
{
	uint32_t* lpImgData = mpRasterIn;
	uint32_t* lpChunkData = NULL;

	// Check input arguments
	if (anX >= msImgDetailsIn.width)
		return -1;
	if (anY >= msImgDetailsIn.height)
		return -1;

	// Make sure we have an image opened
	if (NULL == mpRasterIn)
		return -1;

	// Delete old chunk if we've got it
	if (NULL != mpRasterChunk)
		delete mpRasterChunk;

	// Mark original location and size of chunk
	mnChunkX = anX;
	mnChunkY = anY;
	mnChunkWidth = anWidth;
	if (anX + anWidth >= msImgDetailsIn.width)
		mnChunkWidth -= (anX + anWidth) - msImgDetailsIn.width ;
	mnChunkHeight = anHeight; 
	if (anY + anHeight >= msImgDetailsIn.height)
		mnChunkHeight -= (anY + anHeight) - msImgDetailsIn.height ;

	// Allocate space for the chunk data
	mpRasterChunk = new uint32_t[mnChunkWidth * mnChunkHeight];
	// Set local pointer for ease of access
	lpChunkData = mpRasterChunk;

	// Clear the chunk in case it's too large
	memset(mpRasterChunk, 0, sizeof(uint32_t) * mnChunkWidth 
		* mnChunkHeight);

	// Adjust for x offset
	lpImgData += anX;
	// Copy/cut in chunk data
	for (uint32_t row = 0; row < mnChunkHeight; row++)
	{
		// Copy row data
		memcpy(lpChunkData, lpImgData, sizeof(uint32_t)*mnChunkWidth);
		// Clear row if this was a cut
		if (abCut)
			memset(lpImgData, 0, sizeof(uint32_t)*mnChunkWidth);
		// Advance to next row of image data
		lpImgData += msImgDetailsIn.width;	
		// Advance to next row of chunk data
		lpChunkData += mnChunkWidth;
	}
	
	return 0;
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
	uint32_t* lpRasterOld = mpRasterChunk;
	uint32_t* lpOldChunk = mpRasterChunk;
	uint32_t* lpNewChunk = NULL;
	// The number of pixels to jump when proceeding to the next row
	int32_t lnRowOffset = mnChunkWidth;

	// Make sure we have a valid chunk in memory
	if (NULL == mpRasterChunk)
		return -1;

	// Assume it's not a mistake if we've been told to do nothing
	if (false == abHoriz && false == abVert)
		return 0;

	// Create new chunk to store flipped data
	mpRasterChunk = new uint32_t[mnChunkWidth * mnChunkHeight];
	lpNewChunk = mpRasterChunk;

	// Make sure we had enough heap space for another chunk
	if (NULL == mpRasterChunk)
		return -1;

	// If vertical flip we work backwards from bottom to top
	if (abVert)
	{
		// Work backwards from bottom of old data to top
		lnRowOffset = -lnRowOffset;
		// Start at last row in new chunk data
		lpOldChunk += mnChunkWidth * (mnChunkHeight-1);
	}

	// Loop for row-wise copying
	for (uint32_t row = 0; row < mnChunkHeight; row++)
	{
		// Copy the row data
		if (abHoriz)
		{
			// Copy row backwards if flipped horizontally
			for (uint32_t col=0; col < mnChunkWidth; col++)
				lpNewChunk[col] = 
					lpOldChunk[mnChunkWidth-col-1];
		}
		else
		{
			memcpy(lpNewChunk, lpOldChunk, sizeof(uint32_t)
				* mnChunkWidth);
		}

		// Increment the image data pointers
		lpOldChunk += lnRowOffset;
		lpNewChunk += mnChunkWidth;	
	}

	// Delete the old chunk
	delete [] lpRasterOld;

	return 0;
}

int 
tcImgGlitch::corruptChunk()
{
	// Make sure we have a valid chunk in memory
	if (NULL == mpRasterChunk)
		return -1;

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
tcImgGlitch::pasteChunk(uint32_t anTransX, uint32_t anTransY, 
	teCombMethod aeCombMethod)
{
	// Number of pixels to paste per row in case less than chunk width
	uint32_t lnPasteWidth = mnChunkWidth;
	// Number of rows to paste in case less than chunk height
	uint32_t lnPasteHeight = mnChunkHeight;
	uint32_t* lpImgData = mpRasterOut;
	uint32_t* lpChunkData = mpRasterChunk;

	// Make sure we have a valid chunk in memory
	if (NULL == mpRasterChunk)
		return -1;

	// Make sure an output image is open
	if (NULL == mpRasterChunk)
		return -1;

	// Readjust paste width if chunk runs outside of output image
	if (anTransX + mnChunkX + mnChunkWidth >= msImgDetailsOut.width)
	{
		lnPasteWidth -=  (anTransX + mnChunkX + mnChunkWidth)
			- msImgDetailsOut.width;
	}

	// Readjust paste height if chunk runs outside of output image
	if (anTransY + mnChunkY + mnChunkHeight >= msImgDetailsOut.height)
	{
		lnPasteHeight -=  (anTransY + mnChunkY + mnChunkHeight)
			- msImgDetailsOut.height;
	}

	lpImgData += anTransX + mnChunkX;
	// Copy the chunk to the output raster 
	for (uint32_t row = 0; row < lnPasteHeight; row++)
	{
		// Copy over a row
		pasteRow(lpImgData, lpChunkData, lnPasteWidth, aeCombMethod);
		// Increment chunk pointer by a row
		lpChunkData += mnChunkWidth;
		// Increment output raster point by a row
		lpImgData += msImgDetailsOut.width;
	}

	return 0;
}

/**
 * Take the given row of data and paste is using the specified 
 * combination method.
 *
 * @param apDest Pointer to desitation.
 * @param apSource Pointer to source data.
 * @param anNumPixels Number of pixels to copy. 
 * @param aeCombMethod Combine method.
 *
 * @return None.
 */
void 
tcImgGlitch::pasteRow(uint32_t* apDest, uint32_t* apSource,
	uint32_t anNumPixels, teCombMethod aeCombMethod)
{
	switch (aeCombMethod)
	{
	case eeOr:
		for (uint32_t cnt=0; cnt < anNumPixels; cnt++)
			apDest[cnt] |= apSource[cnt];
		break;
	case eeXor:
		for (uint32_t cnt=0; cnt < anNumPixels; cnt++)
			apDest[cnt] ^= apSource[cnt];
		break;
	case eeAnd:
		for (uint32_t cnt=0; cnt < anNumPixels; cnt++)
			apDest[cnt] &= apSource[cnt];
		break;
	case eeAvg:
		for (uint32_t cnt=0; cnt < anNumPixels; cnt++)
		{
			apDest[cnt] = ((apDest[cnt]>>1)&0xFF7F7F7F)
				+ ((apSource[cnt]>>1)&0xFF7F7F7F);
		}
		break;
	case eeOverwrite:
		memcpy(apDest, apSource, sizeof(uint32_t)*anNumPixels);
		break;
	}
}

