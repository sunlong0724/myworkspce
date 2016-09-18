/*
 * NoLockCBuffer.h
 *
 *  Created on: Jun 23, 2014
 *      Author: ddjpol
 */

#ifndef NOLOCKCBUFFER_H_
#define NOLOCKCBUFFER_H_

#include <stddef.h>


class NoLockCBuffer {
public:
	NoLockCBuffer( );
	virtual ~NoLockCBuffer();

public:

	bool init( int nInitCount, size_t nDataSize );

	bool push(void *, void** pDirectdUse);

	void* pop( );

	void * get_top();

	void unInit() ;

	void clear();

public:
	char * _Buffer;

	int nMaxCount ;
	int nELementSize;

	long m_head;

	long m_tail;

};
typedef struct {
	NoLockCBuffer nolock_buffer;
} RingBuffer;

RingBuffer *RingBuffer_create(int elem_size, int count);

void RingBuffer_destroy(RingBuffer * buffer);

int RingBuffer_read(RingBuffer * buffer, char *target, int amount);

int RingBuffer_write(RingBuffer * buffer, char *data, int length);

void RingBuffer_clear(RingBuffer * buffer);
#endif /* NOLOCKCBUFFER_H_ */
