
#include "Image.h"
#include "vec2.h"
#include "Block.h"
#include <iostream>

#ifdef USE_BLOCK_ITERATOR
#include <iterator>
#endif

using namespace std;

namespace imaging {

    // Constructors 
    
    Block::Block(const size_t & block_size)
        : size(block_size) {
        data = new Component[size];
    }

    Block::Block(const Block & src) {
        size = src.size;
        data = new Component[size];
        setData(src.data);
    }

    // Destructor

    Block::~Block() {
        delete[] data;
    }

    // Generates and returns a block of appropriate size according to a user-defined region on an NON-INTERLACED
    // Image and copies the data from there. The channel variable decides which image channel will the block be copied from. The size 
    // of the returned block depends on the size of the image and the position pos of the selected region in it.
    // Returns nullptr if the estimated size is zero. This can happen for various reasons: a) the size argument was 0, b)
    // there were no pixels left form the requested position to the right margin of the image, c) the y coordinate is 
    // greater than image height-1, d) The requested block size cannot be allocated.
    // If size > image.width - pos.x, then the block actual size becomes image.width - pos.x
    Block * Block::copyFromImage(Image & src, Image::channel_t channel,
           const vecmath::ivec2 & pos, const size_t & size) {
        unsigned int chanel_size = src.getWidth() * src.getHeight();
        unsigned int chanel_offset = channel * chanel_size;
        unsigned int pos_offset = pos.y * src.getWidth() + pos.x;
        const Component * pos_address = src.getRawDataPtr() + chanel_offset + pos_offset;
        
        if (size == 0 || pos.y >= src.getHeight() || pos.x >= src.getWidth()) {
            return nullptr;
        }
        
        Block * result = new Block(size);
//        Component * data =  new (nothrow) Component[size];
//        if (data == nullptr) {
//            return nullptr;
//        }
//        copy(pos_address, pos_address+size, data);
        result->setData(pos_address);
        
        return result;
        
    }

    // Size accessors
    size_t Block::getSize() const {
        return size;
    }

    // Obtain the pointer to the internal data. Useful for fast data copies from the block
    Component * Block::getDataPtr() const {
        return data;
    }

    // Copies the size Component tokens from the provided array to the internal buffer 
    void Block::setData(const Component * src) {
        copy(src, src + size, data);
    }

    // Creates a COPY of the current block and reverses the order of its elements.
    Block Block::reverse() const {
        Block result(size);
        
        for (int i = 0; i < size/2; ++i ) {
            result.data[i] = data[size-i];
        }
        
        return result;
    }

    // Specifies the maximum allowed difference between the i-th cells of two blocks, i=0...size-1, so that
    // the two blocks can be considered equal.
    void Block::setErrorMargin(Component err) {
        error_margin = err;
    }

    // Return the index-th element of the block. No bounds are checked, for speed.
    // Caution: Since no bounds are checked, a segmentation fault may occur for invalid index values.
    Component & Block::operator[] (const size_t index) const {
        return data[index];
    }

    // Same as [], but with bounds checking. If out of bounds return the last element
    Component & Block::operator() (const size_t & index) const {
        size_t index2 = index;
        if (index2 >= size) {
            index2 = size -1;
        }
        
        return data[index2];
    }

    // "equals" operator. Should return true if ALL components of a block are equal to those of the 
    // rhs block. Remember equality is checked using the error_margin: |data[i]-rhs.data[i]|<=error_margin.
    // Caution: In the above equation the quantities are "unsigned". Use appropriate casting to make the formula work!
    // Hint: You can implement it with a "fast compute path", when the error_margin is 0 (no abs() function needs 
    // to be called), just check for equality of elements. Also, you can have an early exit mechanism if at least one
    // element pair doesn't satisfy the above equality criteria. 
    bool Block::operator==(const Block& rhs) const {
        if (size != rhs.size) {
            return false;
        }
        if (error_margin == 0) {
            for (int i = 0; i < size; i++) {
                if (data[i] != rhs.data[i]) {
                    return false;
                }
            }
        } else {
            for (int i = 0; i < size; i++) {
                if (abs( ((int) data[i]) - ((int) rhs.data[i]) ) <= error_margin ) {
                    return false;
                }
            }
        }
        
        return true;
    }

    // The "not equal" operator. Should return true if ANY of the elements in the two blocks differ.
    // You can implement this as "!equal"
    bool Block::operator!=(const Block& rhs) const {
        return ! (*this == rhs);
    }

    // Assignment operator. Be careful: You need to to perform a deep copy of the buffer.
    Block & Block::operator=(const Block & src) {
        if (size != src.size) {
            delete[] data;
            size = src.size;
            data = new Component[size];
        }
        
        setData(src.data);
    }


#ifdef USE_BLOCK_ITERATOR
    // Optional
    // Returns an iterator to the first element of the block (leftmost)
    Block::iterator Block::begin() const {
        iterator result((Block*) this, 0);
        return result;
    }
    // Returns an iterator "beyond" the last element of the block (i.e. invalid, out of range)
    Block::iterator Block::end() const {
        iterator result((Block*) this, size);
        return result;
    }
#endif



#ifdef USE_BLOCK_ITERATOR		
    // Optional

    // Our "Block" iterator is compliant with the c++ (stl) iterator:

    // Constructors

    Block::iterator::iterator(Block *p_data)
        : iterator(p_data, 0) {
        
    }
    Block::iterator::iterator(const Block::iterator & src) {
        block_ptr = src.block_ptr;
        iter = src.iter;
        block_size = src.block_size;
    }

    Block::iterator& Block::iterator::operator++() {
        ++iter;
        return *this;
    }
    
    Block::iterator Block::iterator::operator++(int) {
        Block::iterator result(*this);
        result.iter++;
        return result;
    }
    
    bool Block::iterator::operator==(const Block::iterator& rhs) {
        return ! (*this != rhs);
    }
    
    bool Block::iterator::operator!=(const Block::iterator& rhs) {
        
        return iter != rhs.iter || block_ptr != rhs.block_ptr;
    }
    
    // should return a reference to the original data     
    // so that you can actually read/write from/to the 
    // block the iterator runs on.
    Component & Block::iterator::operator*() const {
        return (*block_ptr)[iter];
    }
    
    // make this constructor protected and allow only the using class (Block)
    // to initialize the iterator with a position argument (to implement Block's
    // begin() and end() member functions )
    Block::iterator::iterator(Block *p_data, const size_t position) {
        block_ptr = p_data;
        block_size = p_data->size;
        iter = position;
    }

#endif

} // namespace imaging
