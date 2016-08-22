//---------------------------------------------------------------------------

#pragma once

#ifndef CONTAINER_H
#define CONTAINER_H

//---------------------------------------------------------------------------

#include <meta/Meta.h>

//---------------------------------------------------------------------------

namespace Rapture
{
	type_checker(is_iterable, iterator);
	type_checker(is_const_iterable, const_iterator);

	template<class Container, useif<is_iterable<Container>::value>>
	typename Container::iterator erase(Container & container, int pos)
	{
		return container.erase(std::next(container.begin(), pos));
	}

	template<class Container, typename T, useif<is_iterable<Container>::value>>
	typename Container::iterator erase(Container & container, const T & value)
	{
		return container.erase(std::remove(container.begin(), container.end(), value), container.end()); //move all values equal to the 'value' to the end of the 'container', then erase the end
	}

	template<class Container, useif<is_iterable<Container>::value>>
	bool check(typename Container::iterator & iterator, const Container & container)
	{
		return iterator != container.end();
	}

	template<class Container, useif<is_const_iterable<Container>::value>>
	bool check(typename Container::const_iterator & iterator, const Container & container)
	{
		return iterator != container.cend();
	}

	template<class T, class A, A T::*member>
	struct MemberSort
	{
		bool operator() (const T & a, const T & b)
		{
			return a.*member < b.*member;
		}

		bool operator() (const T * a, const T * b)
		{
			return a->*member < b->*member;
		}
	};
}

//---------------------------------------------------------------------------
#endif