#pragma once

#include "Image.h"
#include "vec2.h"

//
// Bonus!
//
// Implement and use the block iterator below to exhaustively read the sequence of 
// data in a block.

#define USE_BLOCK_ITERATOR

#ifdef USE_BLOCK_ITERATOR
	#include <iterator>
#endif

namespace imaging
{
	class Block 
	{
	#ifdef USE_BLOCK_ITERATOR
	public:
		// Block iterator (optional). Traverses the block left to right
		typedef class iterator; // local class declaration
	#endif

	protected:
		Component * data;       // Holds the data of the block of type Component 
		size_t size;            // size of (length of) block
		Component error_margin; // The allowed difference when comparing the cells of two blocks. 
		                        // This is used in the == operator and allows a "relaxed" equality.

	public:
		// Constructors 
		Block(const size_t & block_size);   // Create an emlty block. There is no default constructor. It doesn't make sense.
		Block(const Block & src);           // Copy constructor: remember, you need a deep copy of the buffer
		
		// Destructor
		~Block();

		// Generates and returns a block of appropriate size according to a user-defined region on an NON-INTERLACED
		// Image and copies the data from there. The channel variable decides which image channel will the block be copied from. The size 
		// of the returned block depends on the size of the image and the position pos of the selected region in it.
		// Returns nullptr if the estimated size is zero. This can happen for various reasons: a) the size argument was 0, b)
		// there were no pixels left form the requested position to the right margin of the image, c) the y coordinate is 
		// greater than image height-1, d) The requested block size cannot be allocated.
		// If size > image.width - pos.x, then the block actual size becomes image.width - pos.x
		static Block * copyFromImage(Image & src, Image::channel_t channel, const ivec2 & pos, const size_t & size); 
		
		// Size accessors
		size_t getSize() const;
		
		// Obtain the pointer to the internal data. Useful for fast data copies from the block
		Component * getDataPtr() const;

		// Copies the size Component tokens from the provided array to the internal buffer 
		void setData(const Component * src);

		// Creates a COPY of the current block and reverses the order of its elements.
		Block reverse() const;

		// Specifies the maximum allowed difference between the i-th cells of two blocks, i=0...size-1, so that
		// the two blocks can be considered equal.
		void setErrorMargin(Component err);

		// Return the index-th element of the block. No bounds are checked, for speed.
		// Caution: Since no bounds are checked, a segmentation fault may occur for invalid index values.
		Component & operator[] (const size_t index) const;

		// Same as [], but with bounds checking. If out of bounds return the last element
		Component & operator() (const size_t & index) const;

		// "equals" operator. Should return true if ALL components of a block are equal to those of the 
		// rhs block. Remember equality is checked using the error_margin: |data[i]-rhs.data[i]|<=error_margin.
		// Caution: In the above equation the quantities are "unsigned". Use appropriate casting to make the formula work!
		// Hint: You can implement it with a "fast compute path", when the error_margin is 0 (no abs() function needs 
		// to be called), just check for equality of elements. Also, you can have an early exit mechanism if at least one
		// element pair doesn't satisfy the above equality criteria. 
		bool operator==(const Block& rhs) const;

		// The "not equal" operator. Should return true if ANY of the elements in the two blocks differ.
		// You can implement this as "!equal"
		bool operator!=(const Block& rhs) const; 

		// Assignment operator. Be careful: You need to to perform a deep copy of the buffer.
		Block & operator=(const Block & src);
		
		
	#ifdef USE_BLOCK_ITERATOR
		// Optional
		// Returns an iterator to the first element of the block (leftmost)
		Block::iterator begin() const;
		// Returns an iterator "beyond" the last element of the block (i.e. invalid, out of range)
		Block::iterator end() const;
	#endif

	};


#ifdef USE_BLOCK_ITERATOR		
	// Optional

	// Our "Block" iterator is compliant with the c++ (stl) iterator:
	class Block::iterator : public std::iterator<std::forward_iterator_tag, Component>
	{
		Block *block_ptr;          // Pointer to the Block where this iterator operates on (we need access to the data)
		size_t iter;               // maintain current sequential position 
		size_t block_size;         // size of the block also maintained here for speed 

	public:
		// Constructors
		iterator()                             // default. Not really useful...
			: block_ptr(nullptr), iter(0) {}   
		iterator(Block *p_data);               // Ah..  a practical one. Should start at element 0.
		iterator(const Block::iterator& src);        // copy constructor     
		
		Block::iterator& operator++();               // iterator forward operation
		Block::iterator operator++(int);             // 
		bool operator==(const Block::iterator& rhs); // equality check
		bool operator!=(const Block::iterator& rhs); // inequality check 
		Component & operator*() const;               // content retrieval operator. Remenber, 
		                                             // should return a reference to the original data     
		                                             // so that you can actually read/write from/to the 
		                                             // block the iterator runs on.
		
	protected:
		// make this constructor protected and allow only the using class (Block)
		// to initialize the iterator with a position argument (to implement Block's
		// begin() and end() member functions )
		iterator(Block *p_data, const size_t position);
		
		// use this so that you can access the protected constructor above. 
		friend class Block;
	};

#endif

} // namespace imaging
