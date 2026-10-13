#pragma once

#include <algorithm>
#include <memory>
#include <vector>
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
		std::vector<T*> values;
		std::unique_ptr<QuadTree<T>> children[4];

	public:
		const Vector2 min_v;
		const Vector2 max_v;
		const Vector2 center;

		QuadTree(const Vector2& min_v, const Vector2& max_v, int max_depth, int depth = 0)
			: max_depth(max_depth), depth(depth), min_v(min_v), max_v(max_v), center(min_v + (max_v - min_v) * 0.5f)
		{
			children[0] = nullptr;
			children[1] = nullptr;
			children[2] = nullptr;
			children[3] = nullptr;
		}
		~QuadTree(void) { }

		void insert(T* value);
		void remove(T* value);
		void clear(void);

		int get_index(T* value) const;
		bool has_child(int index) const { return children[index] != nullptr; }
		QuadTree* child(int index) const { return (children[index] == nullptr) ? nullptr : children[index].get(); }
		const std::vector<T*>& get_values(void) const { return values; }
		// Returns true if neither this node nor any of its descendants hold values.
		bool empty(void) const;
	};

	template<class T>
	void QuadTree<T>::insert(T* value)
	{
		const auto idx = get_index(value);
		if (idx > -1 && depth < max_depth)
		{
			// split if necessary
			if (children[idx] == nullptr)
			{
				switch (idx)
				{
				case 0:
					children[0] = std::make_unique<QuadTree<T>>(Vector2{ center.x, min_v.y }, Vector2{ max_v.x, center.y }, max_depth, depth + 1);
					break;
				case 1:
					children[1] = std::make_unique<QuadTree<T>>(Vector2{ center.x, center.y }, Vector2{ max_v.x, max_v.y }, max_depth, depth + 1);
					break;
				case 2:
					children[2] = std::make_unique<QuadTree<T>>(Vector2{ min_v.x, center.y }, Vector2{ center.x, max_v.y }, max_depth, depth + 1);
					break;
				case 3:
					children[3] = std::make_unique<QuadTree<T>>(Vector2{ min_v.x, min_v.y }, Vector2{ center.x, center.y }, max_depth, depth + 1);
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
	void QuadTree<T>::remove(T* value)
	{
		const auto idx = get_index(value);
		if (idx > -1 && depth < max_depth)
		{
			if (children[idx] != nullptr)
				children[idx]->remove(value);
		}
		else
		{
			values.erase(std::remove(values.begin(), values.end(), value), values.end());
		}
	}

	template<class T>
	inline void QuadTree<T>::clear(void)
	{
		// Keep child nodes and value capacity around so that the tree can be
		// rebuilt each frame without reallocating.
		values.clear();
		for (auto& c : children)
		{
			if (c != nullptr)
				c->clear();
		}
	}

	template<class T>
	bool QuadTree<T>::empty(void) const
	{
		if (!values.empty())
			return false;
		for (const auto& c : children)
		{
			if (c != nullptr && !c->empty())
				return false;
		}
		return true;
	}

	template<class T>
	int QuadTree<T>::get_index(T* value) const
	{
		auto res = -1;
		if (value->bb.max().x < center.x) // value is in left quadrants
		{
			if (value->bb.max().y < center.y) // value is in top-left quadrant
			{
				res = 3;
			}
			else if (value->bb.min().y >= center.y) // value is in bottom-left quadrant
			{
				res = 2;
			}
		}
		else if (value->bb.min().x >= center.x) // value is in right quadrants
		{
			if (value->bb.max().y < center.y) // value is in top-right quadrant
			{
				res = 0;
			}
			else if (value->bb.min().y >= center.y) // value is in bottom-right quadrant
			{
				res = 1;
			}
		}
		return res;
	}
}