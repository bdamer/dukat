#pragma once

namespace dukat
{
	// Circular buffer with fixed capacity.
	// Elements are inserted at the front of the buffer, overriding the 
	// oldest existing element once capacity has been reached.
	template<typename T, size_t Size>
	class CircularBuffer
	{
	private:
		int head;
		std::size_t _size;
		std::array<T, Size> elements;
		int index(int i) const { return (Size + head - i) % Size; }

	public:
		CircularBuffer(void) : head(0), _size(0), elements() { }
		~CircularBuffer(void) { }

		std::size_t size(void) const { return _size; }

		// Operators
		const T& operator[](int i) const { return elements[index(i)]; }
		T& operator[](int i) { return elements[index(i)]; }

		void push_front(const T& t)
		{
			if (_size < Size)
				_size++;
			head = (head + 1) % Size;
			elements[head] = t;
		}

		void pop_front(void)
		{
			if (_size == 0)
				return;
			_size--;
			head = (Size + head - 1) % Size;
		}
	};
}

