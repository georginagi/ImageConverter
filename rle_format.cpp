#include "rle_format.h"
#include "Image.h"
#include "vec2.h"
#include "Block.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace imaging;

//helper function to read binary data
static unsigned short readUnsignedShortBinary(istream &in) {
    unsigned short * buffer = new unsigned short[1];
    in.read((char*) buffer, 2);
    unsigned short result = *buffer;
    delete[] buffer;
    return result;
}

//compression
static vector<Component> * compress(Block * bl, Component err) {
    vector<Component> * result = new vector<Component>();
    Component current = *(bl->begin());
    int count = 0;
    Block::iterator end = bl->end();
    
    int i = 0;
    for (Block::iterator iter = bl->begin(); iter != end; ++iter) {
        Component c = *iter;
        if (abs( ((int) c) - ((int) current) ) <= err ) {
            ++count;
        } else {
            result->push_back(current);
            result->push_back(count);
            count = 1;
            current = c;
        }
        i++;
    }
//    cout << "block len " << i << endl;
    result->push_back(current);
    result->push_back(count);
    
    return result;
}

namespace imaging {

    //implementation of the rle image writer
    void RLEImageWriter::write(std::string filename, const Image & src) {
        ofstream cpiImageOut(filename, ios_base::out | ios_base::binary);
        if (cpiImageOut) {
            unsigned char v = 2;
            unsigned short e = 258;
            unsigned short w = src.getWidth();
            unsigned short h = src.getHeight();
            unsigned short b = block_length;

            //write out the header
            cpiImageOut << 'C' << 'P' << 'I';
            cpiImageOut.put(v);
            cpiImageOut.write((char*) &e, 2);
            cpiImageOut.write((char*) &w, 2);
            cpiImageOut.write((char*) &h, 2);
            cpiImageOut.write((char*) &b, 2);
            //write out image data 
            //arranged in data blocks
            int n_blocks = (int) ceil ( src.getWidth() / (float) block_length );
            int b_last = src.getWidth() - (n_blocks-1) * block_length;
            ivec2 size(src.getWidth(), src.getHeight());
            
            vector<Image::channel_t> chanels = {Image::RED,Image::GREEN, Image::BLUE};
            
            for (Image::channel_t chanel : chanels) {
                ivec2 pos(0, 0);
                bvec2 cmp = pos < size;
                while (cmp.y) {
                    int curr_block = 0;
                    for (int i = 1; i < n_blocks; ++i) {
                        Block * b = Block::copyFromImage((Image&) src, chanel, pos, block_length);
                        pos.x += block_length;
                        vector<Component> * compr = compress(b, threshold);
                        cpiImageOut.write((char*) compr->data(), compr->size());
                        delete compr;
                        delete b;
                        
                    }
                    Block * b = Block::copyFromImage((Image&) src, chanel, pos, b_last);
                    vector<Component> * compr = compress(b, threshold);
                    cpiImageOut.write((char*) compr->data(), compr->size());
                    delete compr;
                    delete b;
                    
                    pos.x = 0;
                    pos.y++;
                    cmp = pos < size;
                }
            }
            cout << "w: " << src.getWidth() << " h: " << src.getHeight() << endl;
        } else {
            cout << "Cannot open file.\n" << endl;
            addLogEntry("Cannot open file " + filename);
        }
    }

    //implemetation of the rle image reader 
    Image * RLEImageReader::read(std::string filename) {
        ifstream rleImageIn(filename);

        if (rleImageIn) {
            Image *img;
            unsigned short w, h;
            unsigned short endian;
            unsigned short maxBlockSize;
            long sizeOfFile;

            //get file size
            rleImageIn.seekg(0, ifstream::end);
            sizeOfFile = rleImageIn.tellg();
            rleImageIn.seekg(0, ifstream::beg);

            //read format info
            char * headerChar = new char[4];
            rleImageIn.read(headerChar, 4);
            endian = readUnsignedShortBinary(rleImageIn);
            if (!(
                    (headerChar[0] == 'C') && (headerChar[1] == 'P') && (headerChar[2] == 'I') &&
                    (headerChar[3] == 2) && (endian == 258)
                    )) {
                cout << "Wrong CPI Format" << endl;
                addLogEntry("False CPI image");
                delete[] headerChar;
                return nullptr;
            }
            //read metadata
            w = readUnsignedShortBinary(rleImageIn);
            h = readUnsignedShortBinary(rleImageIn);
            maxBlockSize = readUnsignedShortBinary(rleImageIn);

            //read image data
            long size = w * h * 3;

            unsigned char* imageChar = new unsigned char[sizeOfFile - rleImageIn.tellg()];
            unsigned char* decodedImageChar = new unsigned char[size];

            
            unsigned int dataSize = sizeOfFile - rleImageIn.tellg();
            rleImageIn.read((char*) imageChar, dataSize);

            unsigned int decodedCharLength = 0;
            unsigned int charCount;
            unsigned int j = 0;
            //decode rle data
            for (unsigned int i = 1; i < dataSize; i += 2) {
                charCount = (int) imageChar[i]; //to stoiheio pou mas deixnei poses fores na epanalifthei o arithmos
                while ((decodedCharLength < charCount) && (j < size)) {
                    decodedImageChar[j] = imageChar[i - 1]; //dwse to proigoumeno stoixeo ston kainourgio pinaka
                    j++;
                    decodedCharLength++;
                }
                decodedCharLength = 0;
            }

            //create image object
            img = new Image(w, h, decodedImageChar, false);

            delete[] headerChar;
            delete[] imageChar;
            delete[] decodedImageChar;
            return img;

        } else {
            cout << "Cannot open rle image file.\n" << "" << filename << endl;
            addLogEntry("Cannot open rle image file " + filename);
            return nullptr;
        }
    }
}
