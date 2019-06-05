#pragma once

#include <array>

namespace dukat
{
	template<typename T>
	struct ObjectAllocator
	{
		void init(T& t) { t.flags = T::Alive; }
		bool is_alive(T& t) { return (t.flags & T::Alive) == T::Alive; }
		void free(T& t) { t.flags = ~T::Alive; }
	};

	// Simple object pool.
	template<class T, std::size_t N, class Allocator = ObjectAllocator<T> >
	struct ObjectPool
	{
		std::array<T, N> data;
		std::size_t free_index;
		Allocator alloc;

		ObjectPool(void) : free_index(0) { }
		~ObjectPool(void) { }

		T* acquire(void)
		{
			while (free_index < N)
			{
				auto& e = data[free_index];
				if (!alloc.is_alive(e))
				{
					alloc.init(e);
					return &e;
				}
				free_index++;
			}
			return nullptr;
		}

		void release(T* t) { alloc.free(*t); }
		void release(T& t) { alloc.free(t); }
	};
}