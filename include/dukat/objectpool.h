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
			while (capacity > 0u)
			{
				auto& e = data[free_index];
				if (!alloc.is_alive(e))
				{
					alloc.init(e);
					capacity--;
					return &e;
				}
				if (++free_index == N)
					free_index = 0;
			}
			return nullptr;
		}

		void release(T* t) { release(*t); }
		void release(T& t) { alloc.free(t); capacity++; }
		void clear(void) { std::for_each(data.begin(), data.end(), [&](T& t) { alloc.free(t); }); free_index = 0u; capacity = N; }
	};

	// Object pool that can increase in size.
	template<class T, std::size_t N, class Allocator = ObjectAllocator<T> >
	struct GrowingObjectPool
	{
		std::list<std::array<T, N>> blocks;
		std::size_t free_index;
		std::size_t capacity;
		Allocator alloc;

		GrowingObjectPool(std::size_t num_blocks = 1L) : free_index(0u), capacity(N * num_blocks) 
		{  
			for (auto i = 0u; i < num_blocks; i++)
				blocks.push_back(std::array<T, N>{});
		}
		~GrowingObjectPool(void) { }

		T* acquire(void)
		{
			// Get iterator for initial block
			auto cur_block_index = free_index / N;
			auto it = blocks.begin();
			std::advance(it, cur_block_index);
			while (capacity > 0u)
			{
				auto& arr = *it;
				auto& e = arr[free_index % N];
				if (!alloc.is_alive(e))
				{
					alloc.init(e);
					capacity--;
					return &e;
				}

				++free_index;
				const auto block_index = free_index / N;
				if (free_index == N)
				{
					free_index = 0;
					it = blocks.begin();
					cur_block_index = 0;
				}
				else if (cur_block_index != block_index)
				{
					++it;
					cur_block_index = block_index;
				}
			}

			// grow pool
			blocks.push_back(std::array<T, N>{});
			capacity = N - 1;

			// allocate next element
			auto& e = blocks.back()[0];
			alloc.init(e);
			return &e;
		}

		void release(T* t) { release(*t); }
		void release(T& t) { alloc.free(t); capacity++; }

		void clear(void) 
		{ 
			free_index = 0u;
			blocks.resize(1);
			capacity = N; 
		}
	};
}