#pragma once

#include <array>

namespace dukat
{
	template<typename T>
	struct ObjectAllocator
	{
		void init(T& t) { t.alive = true; }
		bool is_alive(T& t) { return t.alive; }
		void free(T& t) { t.alive = false; }
	};

	// Simple object pool.
	template<class T, std::size_t N, class Allocator = ObjectAllocator<T> >
	struct ObjectPool
	{
		std::array<T, N> data;
		std::size_t free_index;
		std::size_t capacity;
		Allocator alloc;

		ObjectPool(void) : free_index(0u), capacity(N) { }
		~ObjectPool(void) { }

		T* acquire(void)
		{
			while (free_index < N && capacity > 0u)
			{
				auto& e = data[free_index];
				if (!alloc.is_alive(e))
				{
					alloc.init(e);
					capacity--;
					return &e;
				}
				free_index++;
			}
			return nullptr;
		}

		void release(T* t) { release(*t); }
		void release(T& t) { alloc.free(t); capacity++; }
		void clear(void) { std::for_each(data.begin(), data.end(), [&](T& t) { alloc.free(t); }); free_index = 0u; capacity = N; }
	};
}