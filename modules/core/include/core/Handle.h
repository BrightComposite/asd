//---------------------------------------------------------------------------

#ifndef HANDLE_H
#define HANDLE_H

//---------------------------------------------------------------------------

#include <core/addition/Singleton.h>
#include <core/addition/Wrapper.h>
#include <core/memory/allocator/DefaultAllocator.h>
#include <core/Hash.h>
#include <core/container/Container.h>

//---------------------------------------------------------------------------

namespace Rapture
{
	template<class ... A>
	class Handle;

	template<class ... A>
	class UniqueHandle;

	//---------------------------------------------------------------------------

	template<class T>
	struct PointerDeleter
	{
		void operator()(T * ptr)
		{
			delete ptr;
		}
	};

	//---------------------------------------------------------------------------

//#define ATOMIC_REFERENCES

#ifdef ATOMIC_REFERENCES
	using ref_counter_t = atomic<size_t>;
#else
	using ref_counter_t = size_t;
#endif

    /**
     *  @brief
     *  The Shared class is used to store the reference counter which can be
	 *	accessed by the Handle class.
	 *
	 *	The Shared class by itself is only used by "shareable" objects.
	 *	A shareable object is an object of a class T which inherits the Shared
	 *	class. Such thing allows to create handles to sheareables without
	 *	wrapping them into SharedWrappers. Create sheareable classes when you
	 *	know that objects of these classes will be handled (you will almost
	 *	always want that).
     */
	struct Shared : protected DefaultAllocator
	{
		template<class ...> friend class Handle;
		template<class ...> friend class UniqueHandle;
		template<class>		friend struct PointerDeleter;

	protected:
		mutable ref_counter_t _refs = 1;
	};

	template<class T>
	class UniquePtr : public std::unique_ptr<T, PointerDeleter<T>>
	{
	public:
		using Base = std::unique_ptr<T, PointerDeleter<T>>;
		using Base::unique_ptr;

		UniquePtr & operator = (nullptr_t)
		{
			Base::operator = (nullptr);
			return (*this);
		}

		template<class U>
		UniquePtr & operator = (U && right)
		{
			Base::operator = (forward<U>(right));
			return (*this);
		}

		operator T * ()
		{
			return Base::get();
		}

		operator const T * () const
		{
			return Base::get();
		}
	};

	/**
	 *  @brief
	 *	Pointer-wrapping kind of handle. It is used with the objects of
	 *	non-shareable classes.
	 */
	template<class T>
	struct SharedWrapper : public Shared
	{
		template<class, bool>
		friend struct SharedCast;

		SharedWrapper(T * obj) : _inner(obj) {}

		template<class ... A>
		SharedWrapper(A &&... args) : _inner(new T(forward<A>(args)...)) {}

		~SharedWrapper()
		{
			PointerDeleter<T> deleter;
			deleter(_inner);
		}

	protected:
		T * _inner;
	};

	struct SharedEmpty : Empty, Shared {};

	template<class T>
	using is_shareable = is_base_of<Shared, T>;

//---------------------------------------------------------------------------

	/**
	 *  @brief
	 *	Template struct which provides casts for different kinds of handles.
	 */
	template<class T, bool = is_shareable<T>::value>
	struct SharedCast
	{
		using SharedType = SharedWrapper<T>;

		static T * toObj(SharedType * shared)
		{
			return shared->_inner;
		}

		template<class U>
		static SharedType * cast(SharedWrapper<U> * shared)
		{
			return reinterpret_cast<SharedType *>(shared);
		}

		static void toShared(T & x)
		{
			static_assert(is_same<T, SharedWrapper<T>>::value, "RaptureCore error: Can't make this object shared!");
		}
	};

	template<class T>
	struct SharedCast<T, true>
	{
		using SharedType = T;

		static T * toObj(SharedType * shared)
		{
			return shared;
		}

		template<class U>
		static SharedType * cast(U * shared)
		{
			return static_cast<SharedType *>(shared);
		}

		template<class U>
		static void cast(SharedWrapper<U> * shared)
		{
			static_assert(is_same<T, SharedWrapper<T>>::value, "RaptureCore error: Can't cast SharedWrapper to shareable object!");
		}

		static SharedType * toShared(T & x)
		{
			return &x;
		}
	};

//---------------------------------------------------------------------------

	template<class T>
	struct is_handle;

	template<class T, class H>
	struct is_same_handle;

	template<class T, class ... A>
	struct is_handle_init;

//---------------------------------------------------------------------------

	/**
	 *  @brief
	 *  The Handle class is the very important component in
	 *	memory-management of Rapture State Toolkit. It is used to fully
	 *	control life-time of objects.
	 *  It contains functions for allocating, deallocating, referencing and
	 *  releasing of objects of the type T.
	 *  The objects of this class store only pointer to the object.
	 *  They may be statically casted to object-pointer or shared-pointer.
	 *  Use operator *() to get object.
	 *  Use this class with pointers to objects (or shared objects, of
	 *	course).
	 *	The common version of the Handle class contains Owner template
	 *	argument. Assignment of objects of this class is denied, and only
	 *	Owner can do that.
	 */
	template<class ... A>
	class Handle {};

	/**
	 *  @brief
	 *  Disowned version of the Handle class
	 */
	template<class T>
	class Handle<T> : public Handle<T, Empty>
	{
		using Base = Handle<T, Empty>;
		using CastType = typename Base::CastType;
		using SharedType = typename Base::SharedType;

	public:
		Handle() : Base() {}

		Handle(const Handle & h) : Base(h) {}
		Handle(Handle && h) : Base(forward<Handle>(h)) {}

		Handle(SharedType * shared) : Base(shared) {}
		Handle(nullptr_t) : Base(nullptr) {}

		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle(const Handle<U, A...> & h) : Base(h) {}
		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle(Handle<U, A...> && h) : Base(forward<Handle<U, A...>>(h)) {}

		template<class U = T, useif <can_construct<U>::value> endif>
		Handle(Empty) : Base(nothing) {}

		template<class ... A, selectif(0) <can_construct<T, A...>::value, (sizeof...(A) > 0)> endif>
		explicit Handle(A &&... args) : Base(forward<A>(args)...) {}

		template<class ... A, selectif(1) <is_abstract<T>::value, !is_handle_init<T, A...>::value> endif>
		explicit Handle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		Handle & operator = (const Handle & h)
		{
			Base::operator = (h);
			return *this;
		}

		Handle & operator = (Handle && h)
		{
			Base::operator = (forward<Handle<T>>(h));
			return *this;
		}

		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle & operator = (const Handle<U, A...> & h)
		{
			Base::operator = (h);
			return *this;
		}

		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle & operator = (Handle<U, A...> && h)
		{
			Base::operator = (forward<Handle<U, A...>>(h));
			return *this;
		}

		Handle & operator = (SharedType * shared)
		{
			Base::operator = (shared);
			return *this;
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		Handle & init(A &&... args)
		{
			Base::init(forward<A>(args)...);
			return *this;
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		static Handle create(A &&... args)
		{
			SharedType * sh = new SharedType(forward<A>(args)...);
			--sh->_refs;

			return sh;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		void init(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<class U>
		static Handle cast(const Handle<U> & handle)
		{
			return CastType::cast(handle._shared);
		}
	};

	/**
	 *  @brief
	 *  Common owned version of the Handle class
	 */
	template<class T, class Owner>
	class Handle<T, Owner> : public Wrapper<T *, Handle<T, Owner>>, protected DefaultAllocator
    {
		template<class...>
		friend class Handle;

		friend struct Shared;
		friend Owner;

		static_assert(is_determined_class<T>::value, "Class T hasn't been determined yet");

	protected:
		using Base = Wrapper<T *, Handle<T, Owner>>;
		using CastType = SharedCast<T>;
		using SharedType = typename CastType::SharedType;

		friend Base;

		SharedType * _shared;

		Handle(SharedType * shared) : _shared(shared) { keep(); }

		template<class U = T, useif <can_construct<U>::value> endif>
		Handle(Empty) : _shared(new SharedType()) {}

		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle(const Handle<U, A...> & h) : _shared(CastType::cast(h._shared)) { keep(); }
		template<class U, class ... A, useif <based_on<U, T>::value && !is_const<U>::value> endif>
		Handle(Handle<U, A...> && h) : _shared(CastType::cast(h._shared)) { h._shared = nullptr; }

		template<class ... A, selectif(0) <can_construct<T, A...>::value, (sizeof...(A) > 0)> endif>
		explicit Handle(A &&... args) : _shared(new SharedType(forward<A>(args)...)) {}

		template<class ... A, selectif(1) <is_abstract<T>::value, !is_handle_init<T, A...>::value> endif>
		explicit Handle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		T * inptr_() const
		{
			return CastType::toObj(_shared);
		}

		void keep()
		{
			if(_shared != nullptr)
				++_shared->_refs;
		}

		void release()
		{
			if(_shared != nullptr)
			{
				--_shared->_refs;

				if(_shared->_refs <= 0)
					delete _shared;
			}
		}

		Handle & operator = (const Handle & h)
		{
			if(this == &h)
				return *this;

			release();
			_shared = h._shared;
			keep();

			return *this;
		}

		Handle & operator = (Handle && h)
		{
			if(this == &h)
				return *this;

			release();
			_shared = h._shared;
			h._shared = nullptr;

			return *this;
		}

		template<class U, class ... A, useif <not_same_types<tuple<T, Owner>, tuple<U, A...>>::value && based_on<U, T>::value && !is_const<U>::value> endif>
		Handle & operator = (const Handle<U, A...> & h)
		{
			if(void_ptr(this) == void_ptr(&h))
				return *this;

			release();
			_shared = CastType::cast(h._shared);
			keep();

			return *this;
		}

		template<class U, class ... A, useif <not_same_types<tuple<T, Owner>, tuple<U, A...>>::value && based_on<U, T>::value && !is_const<U>::value> endif>
		Handle & operator = (Handle<U, A...> && h)
		{
			if(void_ptr(this) == void_ptr(&h))
				return *this;

			release();
			_shared = CastType::cast(h._shared);
			h._shared = nullptr;

			return *this;
		}

		Handle & operator = (SharedType * shared)
		{
			release();
			_shared = shared;
			keep();

			return *this;
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		Handle & init(A &&... args)
		{
			release();
			_shared = new SharedType(forward<A>(args)...);

			return *this;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		void init(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		static Handle create(A &&... args)
		{
			SharedType * sh = new SharedType(forward<A>(args)...);
			--sh->_refs;

			return sh;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

	public:
		Handle() : _shared(nullptr) {}
		Handle(nullptr_t) : _shared(nullptr) {}

		Handle(const Handle & h) : _shared(h._shared) { keep(); }
		Handle(Handle && h) : _shared(h._shared) { h._shared = nullptr; }
		~Handle() { release(); }

		bool isNull() const
		{
			return _shared == nullptr;
		}

		template<class U>
		bool operator == (const Handle<U> & h) const
		{
			return _shared == h._shared;
		}

		bool operator == (SharedType * shared) const
		{
			return _shared == shared;
		}

		bool operator == (nullptr_t) const
		{
			return _shared == nullptr;
		}

		template<class U>
		bool operator != (const U & x) const
		{
			return !operator == (x);
		}

		template<class U>
		bool operator > (const Handle<U> & h) const
		{
			return _shared > h._shared;
		}

		template<class U>
		bool operator < (const Handle<U> & h) const
		{
			return _shared < h._shared;
		}

		operator SharedType * () const
		{
			return _shared;
		}

		size_t refs() const
		{
			return _shared->_refs;
		}

		size_t hash() const
		{
			return ptr_hash<T>(inptr_());
		}

		template<class U>
		static Handle cast(const Handle<U, Owner> & h)
		{
			return CastType::cast(h._shared);
		}
	};

	template<class T>
	class Handle<const T> : public Handle<const T, Empty>
	{
		using Base = Handle<const T, Empty>;
		using CastType = typename Base::CastType;
		using SharedType = typename Base::SharedType;
		using RealSharedType = typename Base::RealSharedType;

	public:
		Handle() : Base() {}

		Handle(const Handle & h) : Base(h) {}
		Handle(Handle && h) : Base(forward<Handle>(h)) {}

		Handle(const Handle<T> & h) : Base(h) {}
		Handle(Handle<T> && h) : Base(forward<Handle<T>>(h)) {}

		Handle(SharedType * shared) : Base(shared) {}
		Handle(nullptr_t) : Base(nullptr) {}

		template<class U, class ... A, useif <based_on<U, T>::value> endif>
		Handle(const Handle<U, A...> & h) : Base(h) {}
		template<class U, class ... A, useif <based_on<U, T>::value> endif>
		Handle(Handle<U, A...> && h) : Base(forward<Handle<U, A...>>(h)) {}

		template<useif <can_construct<T>::value> endif>
		Handle(Empty) : Base(nothing) {}

		template<class ... A, selectif(0) <can_construct<T, A...>::value && (sizeof...(A) > 0)> endif>
		explicit Handle(A &&... args) : Base(forward<A>(args)...) {}
		template<class ... A, selectif(1) <is_abstract<T>::value && !is_handle_init<T, A...>::value> endif>
		explicit Handle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		Handle & operator = (const Handle & h)
		{
			Base::operator = (h);
			return *this;
		}

		Handle & operator = (Handle && h)
		{
			Base::operator = (forward<Handle>(h));
			return *this;
		}

		Handle & operator = (const Handle<T> & h)
		{
			Base::operator = (h);
			return *this;
		}

		Handle & operator = (Handle<T> && h)
		{
			Base::operator = (forward<Handle<T>>(h));
			return *this;
		}

		Handle & operator = (SharedType * shared)
		{
			Base::operator = (shared);
			return *this;
		}

		template <typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		Handle & init(A &&... args)
		{
			Base::init(forward<A>(args)...);
			return *this;
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		static Handle create(A &&... args)
		{
			auto sh = new RealSharedType(forward<A>(args)...);
			--sh->_refs;

			return sh;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		void init(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<class U>
		static Handle cast(const Handle<const U> & handle)
		{
			return CastType::cast(handle._shared);
		}
	};

	template<class T, class Owner>
	class Handle<const T, Owner> : public Wrapper<const T *, Handle<const T, Owner>>, protected DefaultAllocator
	{
		template<class...>
		friend class Handle;

		friend struct Shared;
		friend Owner;

		static_assert(is_determined_class<T>::value, "Class T hasn't been determined yet");

	protected:
		using Base = Wrapper<const T *, Handle<const T, Owner>>;
		using CastType = SharedCast<const T>;
		using SharedType = typename CastType::SharedType;
		using RealSharedType = typename SharedCast<T>::SharedType;

		friend Base;

		SharedType * _shared;

		Handle(SharedType * shared) : _shared(shared) { keep(); }

		template<class U, class ... A, useif <based_on<U, T>::value> endif>
		Handle(const Handle<U, A...> & h) : _shared(CastType::cast(h._shared)) { keep(); }
		template<class U, class ... A, useif <based_on<U, T>::value> endif>
		Handle(Handle<U, A...> && h) : _shared(CastType::cast(h._shared)) { h._shared = nullptr; }

		template<class U = T, useif <can_construct<U>::value> endif>
		Handle(Empty) : _shared(new RealSharedType()) {}

		template<class ... A, selectif(0) <can_construct<T, A...>::value && (sizeof...(A) > 0)> endif>
		explicit Handle(A && ... args) : _shared(new RealSharedType(forward<A>(args)...)) {}
		template<class ... A, selectif(1) <is_abstract<T>::value && !is_handle_init<T, A...>::value> endif>
		explicit Handle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		const T * inptr_() const
		{
			return CastType::toObj(_shared);
		}

		void keep()
		{
			if(_shared != nullptr)
				++_shared->_refs;
		}

		void release()
		{
			if(_shared != nullptr)
			{
				--_shared->_refs;

				if(_shared->_refs <= 0)
					delete _shared;
			}
		}

		Handle & operator = (const Handle & h)
		{
			if(this == &h)
				return *this;

			release();
			_shared = h._shared;
			keep();

			return *this;
		}

		Handle & operator = (Handle && h)
		{
			if(this == &h)
				return *this;

			release();
			_shared = h._shared;
			h._shared = nullptr;

			return *this;
		}

		Handle & operator = (const Handle<T> & h)
		{
			if(void_ptr(this) == void_ptr(&h))
				return *this;

			release();
			_shared = h._shared;
			keep();

			return *this;
		}

		Handle & operator = (Handle<T> && h)
		{
			if(void_ptr(this) == void_ptr(&h))
				return *this;

			release();
			_shared = h._shared;
			h._shared = nullptr;

			return *this;
		}

		Handle & operator = (SharedType * shared)
		{
			release();
			_shared = shared;
			keep();

			return *this;
		}

		template<typename ... A, selectif(0) <can_construct<T, A...>::value> endif>
		Handle & init(A &&... args)
		{
			release();
			_shared = new RealSharedType(forward<A>(args)...);

			return *this;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value, is_abstract<T>::value> endif>
		void init(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

		template<class ... A, selectif(0) <can_construct<T, A...>::value> endif>
		static Handle create(A &&... args)
		{
			RealSharedType * sh = new RealSharedType(forward<A>(args)...);
			--sh->_refs;

			return sh;
		}

		template<typename ... A, selectif(1) <cant_construct<T, A...>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

	public:
		Handle() : _shared(nullptr) {}
		Handle(nullptr_t) : _shared(nullptr) {}

		Handle(const Handle & h) : _shared(h._shared) { keep(); }
		Handle(Handle && h) : _shared(h._shared) { h._shared = nullptr; }

		~Handle() { release(); }

		bool isNull() const
		{
			return _shared == nullptr;
		}

		template<class U>
		bool operator == (const Handle<U> & h) const
		{
			return h._shared == _shared;
		}

		bool operator == (SharedType * shared) const
		{
			return _shared == shared;
		}

		bool operator == (nullptr_t) const
		{
			return _shared == nullptr;
		}

		template <class U>
		bool operator != (const U & x) const
		{
			return !operator == (x);
		}

		template<class U>
		bool operator > (const Handle<U> & h) const
		{
			return _shared > h._shared;
		}

		template<class U>
		bool operator < (const Handle<U> & h) const
		{
			return _shared < h._shared;
		}

		operator SharedType * () const
		{
			return _shared;
		}

		size_t refs() const
		{
			return _shared->_refs;
		}

		size_t hash() const
		{
			return ptr_hash<T>(inptr_());
		}

		template<class U>
		static Handle cast(const Handle<const U, Owner> & h)
		{
			return h._shared;
		}
	};

	typedef Handle<SharedEmpty> EmptyHandle;

	template<class T, class ... A, selectif(0) <can_construct<T, A...>::value> endif>
	Handle<T> handle(A &&... args)
	{
		return Handle<T>::create(forward<A>(args)...);
	}

	template<class T, class ... A, selectif(1) <cant_construct<T, A...>::value> endif>
	void handle(A &&... args)
	{
		static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
	}

	template<class T, class U>
	Handle<T> handle_cast(const Handle<U> & h)
	{
		return Handle<T>::cast(h);
	}

	template<class T, class U, class Owner>
	Handle<T, Owner> handle_cast(const Handle<U, Owner> & h)
	{
		return Handle<T, Owner>::cast(h);
	}

	template<class T, useif <is_shareable<T>::value> endif>
	Handle<T> share(T * x)
	{
		return x;
	}

	template<class T>
	Handle<T> share(T & x)
	{
		return SharedCast<T>::toShared(x);
	}

	template<class T>
	void share(T && x)
	{
		static_assert(!is_same<T, T>::value, "RaptureCore error: Can't share the temporary object!");
	}

	class SharedIdentifier : public EmptyHandle
	{
	public:
		SharedIdentifier() : EmptyHandle(nothing) {}
	};
}

namespace std
{
	template<class T>
	use_class_hash(Rapture::Handle<T>);
}

//---------------------------------------------------------------------------

namespace Rapture
{
	template<class T>
	struct is_uhandle;

	template<class T, class H>
	struct is_same_uhandle;

	template<class T, class ... A>
	struct is_uhandle_init;

//---------------------------------------------------------------------------

	template<class ... A>
	class UniqueHandle {};

	template<class T>
	class UniqueHandle<T> : public UniqueHandle<T, Empty>
	{
		typedef UniqueHandle<T, Empty> Base;

	public:
		UniqueHandle() : Base() {}

		UniqueHandle(const UniqueHandle & h) = delete;
		UniqueHandle(UniqueHandle && h) : Base(forward<UniqueHandle>(h)) {}

		UniqueHandle(T * shared) : Base(shared) {}
		UniqueHandle(nullptr_t) : Base(nullptr) {}

		template<class U, class ... A, useif <based_on<U, T>::value, !is_const<U>::value> endif>
		UniqueHandle(UniqueHandle<U, A...> && h) : Base(forward<UniqueHandle<U, A...>>(h)) {}

		template<class U = T, useif <can_construct<U>::value> endif>
		UniqueHandle(Empty) : Base(nothing) {}

		template<class ... A, selectif(0) <can_construct<T, A...>::value && (sizeof...(A) > 0)> endif>
		explicit UniqueHandle(A && ... args) : Base(forward<A>(args)...) {}
		template<class ... A, selectif(1) <is_abstract<T>::value && !is_uhandle_init<T, A...>::value> endif>
		explicit UniqueHandle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		UniqueHandle & operator = (const UniqueHandle & h) = delete;

		UniqueHandle & operator = (UniqueHandle && h)
		{
			Base::operator = (forward<UniqueHandle>(h));
			return *this;
		}

		template<class U, useif <
			based_on<U, T>::value && !is_const<U>::value
			> endif
		>
		UniqueHandle & operator = (const UniqueHandle<U> & h)
		{
			Base::operator = (h);
			return *this;
		}

		template<class U, useif <
			based_on<U, T>::value, !is_const<U>::value
			> endif
		>
		UniqueHandle & operator = (UniqueHandle<U> && h)
		{
			Base::operator = (forward<UniqueHandle<U>>(h));
			return *this;
		}

		UniqueHandle & operator = (T * shared)
		{
			Base::operator = (shared);
			return *this;
		}

		template<typename ... A, useif <can_construct<T, A...>::value> endif>
		UniqueHandle & init(A &&... args)
		{
			Base::init(forward<A>(args)...);
			return *this;
		}

		template<class ... A, useif <can_construct<T, A...>::value> endif>
		static UniqueHandle create(A &&... args)
		{
			return new T(forward<A>(args)...);
		}

		template<class T, class ... A, skipif <can_construct<T, A...>::value || !is_abstract<T>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}
	};

	/**
	 *  @brief
	 *  Common owned version of the Handle class
	 */
	template<class T, class Owner>
	class UniqueHandle<T, Owner> : public Wrapper<T *, UniqueHandle<T, Owner>>, protected DefaultAllocator
	{
		template<class...>
		friend class UniqueHandle;
		friend Owner;

		// Check that T has been already determined
		static_assert(is_determined_class<T>::value, "Class T hasn't been determined yet");

	protected:
		typedef Wrapper<T *, UniqueHandle<T, Owner>> Base;
		friend Base;

		T * _inner;

		UniqueHandle(UniqueHandle && h) : _inner(h._inner) { h._inner = nullptr; }

		UniqueHandle(const UniqueHandle & h) = delete;
		UniqueHandle(T * ptr) : _inner(ptr) {}

		template<class U, class ... A, useif <based_on<U, T>::value, !is_const<U>::value> endif>
		UniqueHandle(UniqueHandle<U, A...> && h) : _inner(h._inner) { h._inner = nullptr; }

		template<class U = T, useif <can_construct<U>::value> endif>
		UniqueHandle(Empty) : _inner(new T()) {}

		template<class ... A, useif <can_construct<T, A...>::value, (sizeof...(A) > 0)> endif>
		explicit UniqueHandle(A && ... args) : _inner(new T(forward<A>(args)...)) {}
		template<class ... A, selectif(1) <is_abstract<T>::value && !is_uhandle_init<T, A...>::value> endif>
		explicit UniqueHandle(A &&... args) { static_assert(!is_abstract<T>::value, "Can't construct an abstract class"); }

		T * inptr_() const
		{
			return _inner;
		}

		UniqueHandle & operator = (const UniqueHandle & h) = delete;

		UniqueHandle & operator = (UniqueHandle && h)
		{
			if(this == &h)
				return *this;

			if(_inner)
				delete _inner;

			_inner = h._inner;
			h._inner = nullptr;

			return *this;
		}

		template<class U, useif <
			based_on<U, T>::value, !is_const<U>::value
			> endif
		>
		UniqueHandle & operator = (UniqueHandle<U> && h)
		{
			if(void_ptr(this) == void_ptr(&h))
				return *this;

			if(_inner)
				delete _inner;

			_inner = h._inner;
			h._inner = nullptr;

			return *this;
		}

		UniqueHandle & operator = (T * ptr)
		{
			if(void_ptr(ptr) == void_ptr(_inner))
				return *this;

			if(_inner)
				delete _inner;

			_inner = ptr;

			return *this;
		}

		template<typename ... A, useif <can_construct<T, A...>::value> endif>
		UniqueHandle & init(A &&... args)
		{
			if(_inner)
				delete _inner;

			_inner = new T(forward<A>(args)...);

			return *this;
		}

		template<class ... A, useif <can_construct<T, A...>::value> endif>
		static UniqueHandle create(A &&... args)
		{
			return new T(forward<A>(args)...);
		}

		template<class T, class ... A, skipif <can_construct<T, A...>::value || !is_abstract<T>::value> endif>
		static void create(A &&... args)
		{
			static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
		}

	public:
		UniqueHandle() : _inner(nullptr) {}
		UniqueHandle(nullptr_t) : _inner(nullptr) {}

		~UniqueHandle() { delete _inner; }

		bool isNull() const
		{
			return _inner == nullptr;
		}

		template<class U>
		bool operator == (const UniqueHandle<U> & h) const
		{
			return _inner == h._inner;
		}

		bool operator == (nullptr_t) const
		{
			return _inner == nullptr;
		}

		template<class U>
		bool operator != (const U & x) const
		{
			return !operator == (x);
		}

		template<class U>
		bool operator > (const UniqueHandle<U> & h) const
		{
			return _inner > h._inner;
		}

		template<class U>
		bool operator < (const UniqueHandle<U> & h) const
		{
			return _inner < h._inner;
		}

		operator T * ()
		{
			return _inner;
		}

		operator const T * () const
		{
			return _inner;
		}
	};

	template<class T, class ... A, useif <can_construct<T, A...>::value> endif>
	UniqueHandle<T> unique_handle(A &&... args)
	{
		return UniqueHandle<T>::create(forward<A>(args)...);
	}

	template<class T, class ... A, skipif <can_construct<T, A...>::value> endif>
	void unique_handle(A &&... args)
	{
		static_assert(!is_abstract<T>::value, "Can't construct an abstract class");
	}

//---------------------------------------------------------------------------

	template<class T, class R, class C, class ... A>
	struct method_wrapper<Handle<T>, R(C::*)(A...)>
	{
		typedef R(__thiscall C::*MethodType)(A ...);

		R operator()(A ... args)
		{
			return (object->*method)(forward<A>(args)...);
		}

		Handle<T> object;
		MethodType method;
	};

	template<class T, class R, class C, class ... A>
	struct method_wrapper<Handle<T>, R (C::*)(A...) const>
	{
		typedef R(__thiscall C::*MethodType)(A ...) const;

		R operator()(A ... args)
		{
			return (object->*method)(forward<A>(args)...);
		}

		Handle<T> object;
		MethodType method;
	};

	template<class T, class R, class C, class ... A>
	struct method_wrapper<const Handle<T>, R(C::*)(A...)>
	{
		typedef R(__thiscall C::*MethodType)(A ...);

		R operator()(A ... args)
		{
			return (object->*method)(forward<A>(args)...);
		}

		const Handle<T> object;
		MethodType method;
	};

	template<class T, class R, class C, class ... A>
	struct method_wrapper<const Handle<T>, R(C::*)(A...) const>
	{
		typedef R(__thiscall C::*MethodType)(A ...) const;

		R operator()(A ... args)
		{
			return (object->*method)(forward<A>(args)...);
		}

		const Handle<T> object;
		MethodType method;
	};

//---------------------------------------------------------------------------

	template<class T, class Owner>
	struct VariableInitializer<Handle<T, Owner>>
	{
		static Handle<T, Owner> initialize()
		{
			return {nothing};
		}
	};

	template<class T, class Owner = Empty, class ThreadingModel = CommonThreadingModel>
	struct HandleSingleton : Shared, CopySingleton<Handle<T, Owner>, ThreadingModel> {};

//---------------------------------------------------------------------------

	namespace Internals
	{
		template<class T>
		struct is_handle0 : false_type {};

		template<class ... A>
		struct is_handle0<Handle<A...>> : true_type {};

		template<class T, class U>
		struct is_same_handle1 : false_type {};

		template<class T, class ... A>
		struct is_same_handle1<T, Handle<T, A...>> : true_type {};

		template<class T, class U>
		struct is_same_handle0 : false_type {};

		template<class T, class U, class ... A>
		struct is_same_handle0<T, Handle<U, A...>> : is_same_handle1<T, Handle<decay_t<U>, A...>> {};

		template<class T>
		struct is_uhandle0 : false_type {};

		template<class ... A>
		struct is_uhandle0<Handle<A...>> : true_type {};

		template<class T, class U>
		struct is_same_uhandle1 : false_type {};

		template<class T, class ... A>
		struct is_same_uhandle1<T, Handle<T, A...>> : true_type {};

		template<class T, class U>
		struct is_same_uhandle0 : false_type {};

		template<class T, class U, class ... A>
		struct is_same_uhandle0<T, Handle<U, A...>> : is_same_uhandle1<T, Handle<decay_t<U>, A...>> {};
	}

	template<class T>
	struct is_handle : Internals::is_handle0<decay_t<T>> {};

	template<class T, class H>
	struct is_same_handle : Internals::is_same_handle0<T, decay_t<H>> {};

	template<class T>
	struct is_uhandle : Internals::is_uhandle0<decay_t<T>> {};

	template<class T, class H>
	struct is_same_uhandle : Internals::is_same_uhandle0<T, decay_t<H>> {};

	template<class T, class ... A>
	struct is_handle_init
	{
		using FirstType = first_t<A...>;

		static const bool value =
			sizeof...(A) == 1 && (
			(based_on<FirstType, typename SharedCast<T>::SharedType>::value && std::is_pointer<FirstType>::value) ||
			is_same_handle<T, FirstType>::value ||
			same_type<FirstType, nullptr_t>::value ||
			same_type<FirstType, Empty>::value
			);
	};

	template<class T, class ... A>
	struct is_uhandle_init
	{
		using FirstType = first_t<A...>;

		static const bool value =
			sizeof...(A) == 1 && (
			(based_on<FirstType, T>::value && std::is_pointer<FirstType>::value) ||
			is_same_uhandle<T, FirstType>::value ||
			same_type<FirstType, nullptr_t>::value ||
			same_type<FirstType, Empty>::value
			);
	};
}

//---------------------------------------------------------------------------

#define friend_handle											\
	template<class ...>	friend class  Rapture::Handle;			\
	template<class ...>	friend class  Rapture::UniqueHandle;	\
	template<class>		friend struct Rapture::PointerDeleter;	\
	friend_sfinae(is_constructible)

#define friend_owned_handle(...)							\
	friend class Rapture::Handle<__VA_ARGS__>;				\
	friend class Rapture::UniqueHandle<__VA_ARGS__>;		\
	template<class>	friend struct Rapture::PointerDeleter;	\
	friend_sfinae(is_constructible)

//---------------------------------------------------------------------------
#endif
