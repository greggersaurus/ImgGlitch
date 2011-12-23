/////////////////////////////////////////////////////////////////////////////
// \file 	ImgGlitch.h
// \author 	Gregory Gluszek
// \date	12/10/11
// \brief	Class for messing up a perfectly good image.
///////////////////////////////////////////////////////////////////////////////

#ifndef _IMG_GLITCH_
#define _IMG_GLITCH_

#include <stdint.h>
#include <tiffio.h>

class tcImgGlitch
{
public:

	// Enumeration for different combination options
	enum teCombMethod
	{
		eeOr, //!< Bitwise OR new and old data
		eeXor, //!< Bitwise XOR new and old data
		eeAnd, //!< Bitwise AND new and old data
		eeOverwrite, //!< New data overwrites any old data
	};

	/**
	 * Constructor.
	 */
	tcImgGlitch();

	/**
	 * Desctructor.
	 */
	~tcImgGlitch();

	/**
	 * Specify the input image to be messed with.
	 * 	
	 * @param apFilename The name of the image file to open.
	 * @param abClear If true wipe the output raster to 0. If false
	 *	either allocate memory for palette and don't clear it or if
	 *	palette is open from before leave it unmolested.
	 *	
	 * @return 0 on success.
	 */
	int openImgIn(const char* apFilename, bool abClear);

	/**
	 * Specify and write the raster data to the output file.	
	 *
	 * @param apFilename The name of the file to be written. If exists
	 *	file will be overwritten.
	 *
	 * @return 0 on sucess.
	 */
	int writeImgOut(const char* apFilename);

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
	int pullChunk(uint32_t anX, uint32_t anY, uint32_t anWidth, 
		uint32_t anHeight, bool abCut);
	
	/**
	 * Flip the chunk being process.
	 *
	 * @param abHoriz If true chunk is flipped horizontally.
	 * @param abVert If true chunk is flipped vertically.
	 *
	 * @return 0 on sucess.
	 */
	int flipChunk(bool abHoriz, bool abVert);

	// Can merge with file
	// Create mini-chunks and randomly scrabble them
	int corruptChunk();

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
	int pasteChunk(uint32_t anTransX, uint32_t anTransY, 
		teCombMethod aeCombMethod);

private:

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
	void pasteRow(uint32_t* apDest, uint32_t* apSource, 
		uint32_t anNumPixels, teCombMethod aeCombMethod);

	TIFFRGBAImage msImgDetailsIn; //!< Details of input image.
	uint32_t* mpRasterIn; //!< Raster data for input image.

	uint32_t* mpRasterChunk; //!< Current chunk of image currently
		//!< being processed.
	uint32_t mnChunkX; //!< X pixel location of upper left corner of where
		//!< chunk was taken from image.
	uint32_t mnChunkY; //!< Y pixel location of upper left corner of where
		//!< chunk was taken from image.
	uint32_t mnChunkWidth; //!< Width of chunk in pixels.
	uint32_t mnChunkHeight; //!< Height of chunk in pixels.

	TIFFRGBAImage msImgDetailsOut; //!< Details of output image.
	uint32_t* mpRasterOut; //!< Raster data for output image.
};

#endif /* _IMG_GLITCH_ */

