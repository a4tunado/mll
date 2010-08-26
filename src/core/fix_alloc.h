/*
* Copyright (C) Sergey P. Derevyago, 2003-2004.
*
* Permission to copy, use, modify, sell and distribute this software is granted
* provided this copyright notice appears in all copies.
* This software is provided "as is" without express or implied warranty, and
* with no claim as to its suitability for any purpose.
*
*/

#ifndef __FIX_ALLOC_HPP__
#define __FIX_ALLOC_HPP__

#include <stddef.h>

namespace fixed_alloc_private {

	void get_mem(void*& head, size_t type_sz);

	template <size_t SIZE>
	class void_alloc {
		static void* head;

	public:
		static void* alloc()
		{
			if (!head) get_mem(head, SIZE);

			void* ret=head;
			head=*(void**)head;

			return ret;
		}

		static void free(void* ptr)
		{
			*(void**)ptr=head;
			head=ptr;
		}
	};

	template <size_t SIZE>
	void* void_alloc<SIZE>::head;

}

template <class T>
class fixed_alloc {
public:
	enum { SIZE=(sizeof(T)+sizeof(void*)-1)/sizeof(void*)*sizeof(void*) };

	static T* alloc()
	{
		return (T*)fixed_alloc_private::void_alloc<SIZE>::alloc();
	}
	static void free(void* ptr)
	{
		fixed_alloc_private::void_alloc<SIZE>::free(ptr);
	}
};

class sized_alloc {
public:
	static void* alloc(size_t size);

	static void free(void* ptr, size_t size);
};

#endif
