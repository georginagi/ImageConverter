//------------------------------------------------------------
//
// C++ course assignment code 
//
// G. Papaioannou, 2013
// www.aueb.gr
//
//-------------------------------------------------------------

#pragma once
#include "Image.h"

namespace imaging
{
	// The RLE Image Writer breaks up the image into blocks of maximum size 1 X "block_length" (row blocks, i.e. line segments)
	// and encodes each one using run length encoding with an error "threshold" (see algorithm description and format specification 
	// in assignment document). Therefore, the class is extended with the setBlockDimension and setThreshold to configure these
	// parameters
	//
	class RLEImageWriter : public ImageWriter
	{
	protected:
		unsigned short block_length;     
		Component threshold;

	public:
		void setBlockDimension(unsigned int dim) {block_length = dim>2 ? dim : 2; }
		void setThreshold(Component value) {threshold = value;}
		virtual void write(std::string filename, const Image & src);
		RLEImageWriter(std::string extension = "rle") 
			: ImageWriter(extension), block_length(32), threshold(0) {}
	};

	class RLEImageReader : public ImageReader
	{
	public:
		virtual Image * read(std::string filename);
		RLEImageReader(std::string extension = "rle")
			: ImageReader(extension) {}
	};

} //namespace imaging
