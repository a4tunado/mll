/*
* Copyright (C) Sergey P. Derevyago, 2003-2004.
*
* Permission to copy, use, modify, sell and distribute this software is granted
* provided this copyright notice appears in all copies.
* This software is provided "as is" without express or implied warranty, and
* with no claim as to its suitability for any purpose.
*
*/

#include "fix_alloc.h"

namespace {

	const size_t CHUNK_SIZE=4*1024;

	const size_t SVP=sizeof(void*);

	const size_t MAX_SIZE=128;

	const size_t HEADS_NUM=(MAX_SIZE+SVP-1)/SVP;

	void* heads[HEADS_NUM];

}

void fixed_alloc_private::get_mem(void*& head, size_t type_sz)
{
	size_t n =( CHUNK_SIZE > type_sz ) ? CHUNK_SIZE / type_sz : 1;
	head = operator new(n*type_sz);

	char* last=(char*)head+(n-1)*type_sz;
	for (char* ptr=(char*)head; ; ptr+=type_sz) {
		if (ptr!=last) *(void**)ptr=ptr+type_sz;
		else {
			*(void**)ptr=0;
			break;
		}
	}
}

void* sized_alloc::alloc(size_t size)
{
	size_t index=(size+SVP-1)/SVP;
	if (index>=HEADS_NUM) return operator new(size);

	void*& head=heads[index];
	if (!head ) 
		fixed_alloc_private::get_mem( head, index * SVP );

	void* ret=head;
	head=*(void**)head;

	return ret;
}

void sized_alloc::free(void* ptr, size_t size)
{
	size_t index=(size+SVP-1)/SVP;
	if( index >= HEADS_NUM ){ 
		operator delete(ptr);
	} else {
		void*& head=heads[index];
		*(void**)ptr=head;
		head=ptr;
	}
}
