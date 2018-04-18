#pragma once

#include <memory>
#include "vector2.h"

namespace dukat
{
	// Quadtree used to partition space during collision detection.
	template<class T>
	class QuadTree
	{
	private:
		const int max_depth;
		const int depth;
		std::list<T*> values;
		std::unique_ptr<QuadTree<T>> children[4];

	public:
		const Vector2 min;
		const Vector2 max;
		const Vector2 center;

		QuadTree(const Vector2& min, const Vector2& max, int max_depth, int depth = 0)
			: min(min), max(max), max_depth(max_depth), depth(depth), center(min + (max - min) * 0.5f)
		{
			children[0] = nullptr;
			children[1] = nullptr;
			children[2] = nullptr;
			children[3] = nullptr;
		}
		~QuadTree(void) { }

		void insert(T* value);
		void clear(void);

		int get_index(T* value) const;
		bool has_child(int index) const { return children[index] != nullptr; }
		QuadTree* child(int index) const { return (children[index] == nullptr) ? nullptr : children[index].get(); }
		const std::list<T*>& get_values(void) const { return values; }
	};

	template<class T>
	void QuadTree<T>::insert(T* value)
	{
		auto idx = get_index(value);
		if (idx > -1 && depth < max_depth)
		{
			// split if necessary
			if (children[idx] == nullptr)
			{
				switch (idx)
				{
				case 0:
					children[0] = std::make_unique<QuadTree<T>>(Vector2{ center.x, min.y }, Vector2{ max.x, center.y }, max_depth, depth + 1);
					break;
				case 1:
					children[1] = std::make_unique<QuadTree<T>>(Vector2{ center.x, center.y }, Vector2{ max.x, max.y }, max_depth, depth + 1);
					break;
				case 2:
					children[2] = std::make_unique<QuadTree<T>>(Vector2{ min.x, center.y }, Vector2{ center.x, max.y }, max_depth, depth + 1);
					break;
				case 3:
					children[3] = std::make_unique<QuadTree<T>>(Vector2{ min.x, min.y }, Vector2{ center.x, center.y }, max_depth, depth + 1);
					break;
				}
			}
			children[idx]->insert(value);
		}
		else
		{
			values.push_back(value);
		}
	}

	template<class T>
	inline void QuadTree<T>::clear(void)
	{
		children[0] = nullptr;
		children[1] = nullptr;
		children[2] = nullptr;
		children[3] = nullptr;
		values.clear();
	}

	template<class T>
	int QuadTree<T>::get_index(T* value) const
	{
		auto res = -1;
		if (value->bb.max.x < center.x) // value is in left quadrants
		{
			if (value->bb.max.y < center.y) // value is in top-left quadrant
			{
				res = 3;
			}
			else if (value->bb.min.y >= center.y) // value is in bottom-left quadrant
			{
				res = 2;
			}
		}
		else if (value->bb.min.x >= center.x) // value is in right quadrants
		{
			if (value->bb.max.y < center.y) // value is in top-right quadrant
			{
				res = 0;
			}
			else if (value->bb.min.y >= center.y) // value is in bottom-right quadrant
			{
				res = 1;
			}
		}
		return res;
	}
}