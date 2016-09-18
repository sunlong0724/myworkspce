/*
 * NoLockCBuffer.cpp
 *
 *  Created on: Jun 23, 2014
 *      Author: ddjpol
 */

#include "NoLockCBuffer.h"
#include <iostream>
#include <string.h>
NoLockCBuffer::NoLockCBuffer() {

	_Buffer = NULL;


	m_head = 0;

	m_tail = 0;
}

NoLockCBuffer::~NoLockCBuffer() {

	unInit();
}

bool NoLockCBuffer::init(int nInitCount, size_t nDataSize)
{

	nMaxCount = nInitCount;
	nELementSize = nDataSize ;

	_Buffer  = new char[nMaxCount * nELementSize ];

	memset(_Buffer, 0x00, nMaxCount * nELementSize) ;

	m_head = 0;

	m_tail = 0;

	return true ;
}

void NoLockCBuffer::clear() {
	m_head = m_tail = 0;
}

void NoLockCBuffer::unInit()
{
	m_head = 0;

	m_tail = 0;

	if( _Buffer )
		delete[] _Buffer ;
	_Buffer = NULL ;
}

bool NoLockCBuffer::push(void * _Element, void** pDirectdUse = NULL)
{

	if( m_head == ((m_tail + 1)% nMaxCount) ) return false ;

	char * p_pos = _Buffer + m_tail * nELementSize;
	if( pDirectdUse == NULL)
		memcpy(p_pos, _Element,nELementSize) ;
	else
	{
		*pDirectdUse = p_pos;

		if(_Element == NULL)
			m_tail = (m_tail + 1)% nMaxCount ;
		return true ;
	}

	m_tail = (m_tail + 1)% nMaxCount ;

	return true ;
}

void* NoLockCBuffer::pop( )
{
	if( m_tail == m_head) return NULL ;

	char * p_pos = _Buffer + m_head * nELementSize;

	m_head = (m_head + 1)% nMaxCount ;

	return p_pos ;
}

void * NoLockCBuffer::get_top()
{
	return _Buffer;
}

RingBuffer *RingBuffer_create(int elem_size, int count) {
	if (count == 1) {
		count = 2;
	}
	RingBuffer* buffer = new RingBuffer();
	buffer->nolock_buffer.init(count, elem_size);
	return buffer;
}

void RingBuffer_destroy(RingBuffer * buffer) {
	delete buffer;
}

int RingBuffer_read(RingBuffer * buffer, char *target, int amount) {
	if (buffer->nolock_buffer.nELementSize != amount) {
		return -1;
	}
	void* p = buffer->nolock_buffer.pop();
	if (p == NULL) {
		return -1;
	}
	memcpy(target, p, amount);
	return amount;
}

int RingBuffer_write(RingBuffer * buffer, char *data, int length) {
	if (buffer->nolock_buffer.nELementSize != length) {
		return -1;
	}

	bool ret = buffer->nolock_buffer.push(data);
	if (ret == false) {
		return -1;
	}
	return length;
}

void RingBuffer_clear(RingBuffer * buffer) {
	buffer->nolock_buffer.clear();
}