//---------------------------------------------------------------------------

#pragma once

#ifndef COM_EXCEPTION_H
#define COM_EXCEPTION_H

//---------------------------------------------------------------------------

#include <comdef.h>
#include <core/Exception.h>

//---------------------------------------------------------------------------

namespace asd
{
	class ComException : public Exception
	{
	public:
		ComException(HRESULT hr) : Exception(getError(hr)) {}
		template<class H, class ... T>
		ComException(HRESULT hr, H && head, T &&... tail) : Exception(getError(hr), " | ", forward<H>(head), forward<T>(tail)...) {}
		ComException(const ComException & except) : Exception(static_cast<const Exception &>(except)) {}
		ComException(ComException && except) : Exception(forward<Exception>(except)) {}

		virtual ~ComException() {}

		static WideString getError(HRESULT hr)
		{
			_com_error err(hr);
			MessageBoxW(0, err.ErrorMessage(), L"COM exception", MB_ICONERROR);
			return err.ErrorMessage();
		}
	};

	template<typename ... A>
	inline void com_assert(HRESULT hr, A &&... args)
	{
		if(FAILED(hr))
			throw ComException(hr, forward<A>(args)...);
	}
}

//---------------------------------------------------------------------------
#endif