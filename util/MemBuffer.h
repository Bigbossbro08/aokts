/**
 * MemBuffer.h, defines class MemBuffer
 * AOKTS, GPL, all that.
 */

#include "Buffer.h"

/**
 * An implementation of Buffer that uses a statically-sized chunk of memory as
 * its backing store.
 */
class MemBuffer : public Buffer
{
public:
	MemBuffer(char *b, int l);
//	Buffer(const char *b, int l);
	MemBuffer(int l);
	~MemBuffer();

	void read(void *dest, size_t length);
	void reads(char *dest, size_t lensize);
	void skip(size_t offset);

	void write(const void *source, size_t length);
	void writes(const char *source, size_t lensize);
	void fill(int value, size_t length);


	const char *get() const	{ return buffer; }

private:
	bool mine;	//did i create memory?
	bool writeable;
	char *buffer;
	char *pos;
	int blen;

	// throws std::out_of_range if the specified length won't fit in the buffer
	void checkFits(size_t length) const;
};
