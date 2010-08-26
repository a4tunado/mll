/*
* Copyright (C) Sergey P. Derevyago, 2003-2004.
*
* Permission to copy, use, modify, sell and distribute this software is granted
* provided this copyright notice appears in all copies.
* This software is provided "as is" without express or implied warranty, and
* with no claim as to its suitability for any purpose.
*
*/

#ifndef __SH_PTR_HPP__
#define __SH_PTR_HPP__
#include "fix_alloc.h"

//! Implementation of shared smart pointer
template <class T>
class sh_ptr {
	struct Rep {
		T* ptr;
		size_t refs;

		Rep(T* ptr_) : ptr(ptr_), refs(1) {}

		~Rep() { delete ptr; }

		void* operator new(size_t)
		{
			return fixed_alloc<Rep>::alloc();
		}

		void operator delete(void* ptr, size_t)
		{
			fixed_alloc<Rep>::free(ptr);
		}
	};

	Rep* rep;

public:
	explicit sh_ptr(T* ptr=0)
	{
		try { rep=new Rep(ptr); }
		catch (...) {
			delete ptr;
			throw;
		}
	}

	sh_ptr(const sh_ptr& shp)
	{
		rep=shp.rep;
		rep->refs++;
	}
	~sh_ptr() { if (--rep->refs==0) delete rep; }

	sh_ptr& operator=(const sh_ptr& shp)
	{
		shp.rep->refs++;
		if (--rep->refs==0) delete rep;
		rep=shp.rep;

		return *this;
	}

	void swap(sh_ptr& shp)
	{
		Rep* tmp=rep;
		rep=shp.rep;
		shp.rep=tmp;
	}

	T& operator*() const { return *rep->ptr; }

	T* operator->() const { return rep->ptr; }

	T* get() const { return rep->ptr; }

	void set(T* ptr) { rep->ptr=ptr; }

	size_t refs() const { return rep->refs; }
};

template <class T>
class sh_array {
	struct Rep {
		T* ptr;
		size_t refs;

		Rep(T* ptr_) : ptr(ptr_), refs(1) {}

		~Rep() { delete [] ptr; }

		void* operator new(size_t)
		{
			return fixed_alloc<Rep>::alloc();
		}

		void operator delete(void* ptr, size_t)
		{
			fixed_alloc<Rep>::free(ptr);
		}
	};

	Rep* rep;

public:
	explicit sh_array(T* ptr=0)
	{
		try { rep=new Rep(ptr); }
		catch (...) {
			delete [] ptr;
			throw;
		}
	}

	sh_array(const sh_array& sha)
	{
		rep=sha.rep;
		rep->refs++;
	}

	~sh_array() { if (--rep->refs==0) delete rep; }

	sh_array& operator=(const sh_array& sha)
	{
		sha.rep->refs++;
		if (--rep->refs==0) delete rep;
		rep=sha.rep;

		return *this;
	}

	void swap(sh_array& sha)
	{
		Rep* tmp=rep;
		rep=sha.rep;
		sha.rep=tmp;
	}

	T& operator[](size_t i) const { return rep->ptr[i]; }

	T* get() const { return rep->ptr; }

	void set(T* ptr) { rep->ptr=ptr; }

	size_t refs() const { return rep->refs; }
};

#endif
