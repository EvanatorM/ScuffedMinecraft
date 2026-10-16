#pragma once

#include <wv/core.h>

template <typename T>
class ChunkRefOwner
{
public:
	ChunkRefOwner() : _ptr(nullptr) {}
	explicit ChunkRefOwner(T* ptr) : _ptr(ptr) {}

	ChunkRefOwner(ChunkRefOwner&& other) noexcept
		: _ptr(other._ptr), _refCount(other._refCount.load())
	{
		other._ptr = nullptr;
		other._refCount = 0;
	}

	ChunkRefOwner& operator=(ChunkRefOwner&& other) noexcept
	{
		if (this != &other)
		{
			delete _ptr;

			_ptr = other._ptr;
			_refCount = other._refCount.load();

			other._ptr = nullptr;
			other._refCount = 0;
		}
		return *this;
	}

	// Disable copying
	ChunkRefOwner(const ChunkRefOwner&) = delete;
	ChunkRefOwner& operator=(const ChunkRefOwner&) = delete;

	~ChunkRefOwner()
	{
		if (_refCount.load() > 0)
			LOG_WARN("Chunk Ref Owner deleted while still having references! _ptr: %p", (void*)_ptr);
		delete _ptr;
		//LOG_INFO("Deleted chunk ref owner");
	}

	T* operator->() { return _ptr; }
	const T* operator->() const { return _ptr; }

	template <typename... Args>
	static ChunkRefOwner<T> CreateRef(Args&&... args)
	{
		T* ptr = new T(std::forward<Args>(args)...);
		return ChunkRefOwner<T>(ptr);
	}

	void IncRefCount() const
	{
		++_refCount;
		//LOG_INFO("Added to Ref: %d", _refCount.load());
	}

	void DecRefCount() const
	{
		--_refCount;
		//LOG_INFO("Subtracted from Ref: %d", _refCount.load());
	}

	uint32_t GetRefCount() const { return _refCount.load(); }

	T* Get() const
	{
		return _ptr;
	}

	T* _ptr = nullptr;
	mutable std::atomic<uint32_t> _refCount = 0;
};

template <typename T>
class ChunkRef
{
public:
	ChunkRef() = default;
	ChunkRef(ChunkRefOwner<T>* owner)
		: _owner(owner)
	{
		_owner->IncRefCount();
	}
	ChunkRef(ChunkRefOwner<T>& owner)
		: _owner(&owner)
	{
		_owner->IncRefCount();
	}

	ChunkRef(const ChunkRef& other)
	{
		_owner = other._owner;
		_owner->IncRefCount();
	}

	ChunkRef& operator=(const ChunkRef& other)
	{
		_owner = other._owner;
		_owner->IncRefCount();
	}

	~ChunkRef()
	{
		_owner->DecRefCount();
		//LOG_INFO("Deleted chunk ref");
	}

	T* operator->()
	{
		return _owner->_ptr;
	}

	const T* operator->() const
	{
		return _owner->_ptr;
	}

	T* Get() const
	{
		return _owner->_ptr;
	}

private:
	ChunkRefOwner<T>* _owner;
};