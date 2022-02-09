#pragma once

namespace dukat
{
	template<typename T>
	class MemoryPool
	{
	private:
		// Chunk holding metadata for unallocated objects.
		struct Chunk
		{
			Chunk* next;
		};

		const std::size_t capacity;
		Chunk* base;
		Chunk* free_list;

	public:
		/// <summary>
		/// Creates a new memory pool with a fixed capacity.
		/// </summary>
		/// <param name="capacity">The max number of objects the pool can hold</param>
		MemoryPool(std::size_t capacity) : capacity(capacity)
		{
			// allocate memory
			base = reinterpret_cast<Chunk*>(malloc(capacity * sizeof(T)));
			free_list = base;

			// chain up free chunks
			auto ptr = free_list;
			for (auto i = 0u; i < capacity - 1; i++)
			{
				ptr->next = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(ptr) + sizeof(T));
				ptr = ptr->next;
			}
			ptr->next = nullptr;
		}
		
		~MemoryPool(void) 
		{
			::free(base);
			base = nullptr;
			free_list = nullptr;
		}

		/// <summary>
		/// Allocates a new object from the pool
		/// </summary>
		/// <param name="size">Size of the object to allocate.</param>
		/// <returns>Pointer to allocated object.</returns>
		void* allocate(std::size_t size)
		{
			if (size != sizeof(T))
				return ::operator new(size);
			if (free_list == nullptr)
				throw std::bad_alloc();
			auto res = free_list;
			free_list = free_list->next;
			return reinterpret_cast<void*>(res);
		}

		/// <summary>
		/// Releases object back into the pool.
		/// </summary>
		/// <param name="ptr">The object to be released.</param>
		/// <param name="size">Size of the object to release.</param>
		void free(void* ptr, std::size_t size)
		{
			if (size != sizeof(T))
			{
				::operator delete(ptr, size);
			}
			else
			{
				auto chunk = reinterpret_cast<Chunk*>(ptr);
				chunk->next = free_list;
				free_list = chunk;
			}
		}
	};
}