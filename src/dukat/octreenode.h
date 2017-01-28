#pragma once

#include <memory>
#include "vector3.h"
#include "log.h"

namespace dukat
{
    template <typename T>
    class OctreeNode
    {
    private:
        std::unique_ptr<T> data;
        std::unique_ptr<OctreeNode<T>> nodes[8];

        // Returns the index of the octant that contains a point.
        int octant_index(const Vector3& point) const;
        // Returns first node that is intersected by ray.
        int first_node(float tx0, float ty0, float tz0, float txm, float tym, float tzm) const;
        // Returns the next node that is intersected by ray.
        int next_node(float txm, int x, float tym, int y, float tzm, int z) const;

    public:
        const Vector3 origin;
        const float half_size;

        OctreeNode(const Vector3& origin, float half_size);
        ~OctreeNode(void) { }

        // Checks if this is a leaf node
        bool is_leaf(void) const { return nodes[0] == nullptr; }
        // Returns a child at a give index.
        OctreeNode<T>* get_child(int idx) const { return nodes[idx].get(); }
        // Returns the data element for this node.
        T* get_data(void) const { return data.get(); }
        // Sets the data element for this node.
        void set_data(std::unique_ptr<T> data) { this->data = std::move(data); }
        // Returns the count of all nodes in this tree.
        int count(void) const;

        // Splits the current node into an interior node with 8 children.
        // Any data associated with this node is lost.
        void split(void);
        // Joins the interior nodes. Any data associted with the interior 
        // nodes is lost.
        void join(void);
        // Inserts a new data point at a provided position into the tree.
        void insert(const Vector3& pos, std::unique_ptr<T> new_data);
        // Removes redundant tree nodes.
        void reduce(void);

        // Parametric sampling of this node.
        T* sample(float tx0, float ty0, float tz0, float tx1, float ty1, float tz1, char oidx) const;
    };

    template <typename T>
    OctreeNode<T>::OctreeNode(const Vector3& origin, float half_size) 
        : data(nullptr), origin(origin), half_size(half_size)
    {
        for (int i = 0; i < 8; i++)
        {
            nodes[i] = nullptr;
        }
    }

    template <typename T>
    int OctreeNode<T>::octant_index(const Vector3& point) const 
    {
        int idx = 0;
        if (point.x >= origin.x)
            idx |= 4;
        if (point.y >= origin.y)
            idx |= 2;
        if (point.z >= origin.z)
            idx |= 1;
        return idx;
    }

    template <typename T>
    int OctreeNode<T>::first_node(float tx0, float ty0, float tz0, float txm, float tym, float tzm) const
    {
        int idx = 0;
        // Set index based on which plane the ray hits first
        if (tx0 > ty0)
        {
            if (tx0 > tz0)	// YZ Plane
            {
                if (tym < tx0) 
                    idx |= 2;
                if (tzm < tx0) 
                    idx |= 1;
                return idx;
            }
        }
        else 
        {
            if (ty0 > tz0)	// XZ Plane
            {
                if (txm < ty0)
                    idx |= 4;
                if (tzm < ty0) 
                    idx |= 1;
                return idx;
            }
        }
        // PLANE XY
        if (txm < tz0) 
            idx |= 4;
        if (tym < tz0) 
            idx |= 2;
        return idx;
    }

    template <typename T>
    int OctreeNode<T>::next_node(float txm, int x, float tym, int y, float tzm, int z) const
    {
        if (txm < tym)
        {
            if (txm < tzm)
            {
                return x;	// YZ plane
            }  
        }
        else
        {
            if (tym < tzm) 
            {
                return y;	// XZ plane
            }
        }
        return z; // XY plane;
    }

    template <typename T>
    int OctreeNode<T>::count(void) const
    {
        int res = 0;
        if (!is_leaf())
        {
            for (int i = 0; i < 8; i++)
            {
                res += nodes[i]->count();
            }
        }
        else
        {
            res = 1;
        }
        return res;
    }

    template <typename T>
    void OctreeNode<T>::split(void)
    {
        assert(nodes[0] == nullptr); // don't split if we already have nodes
        data = nullptr;

        auto new_origin = origin;
        auto new_half_size = half_size * 0.5f;
        for (int i = 0; i < 8; i++)
        {
            Vector3 new_origin = {
                origin.x + (i & 4 ? new_half_size : -new_half_size),
                origin.y + (i & 2 ? new_half_size : -new_half_size),
                origin.z + (i & 1 ? new_half_size : -new_half_size)
            };
            nodes[i] = std::make_unique<OctreeNode<T>>(new_origin, new_half_size);
        }
    }

    template <typename T>
    void OctreeNode<T>::join(void)
    {
        assert(nodes[0] != nullptr); // don't join if we don't have interior nodes
        for (int i = 0; i < 8; i++)
        {
            nodes[i] = nullptr;
        }
    }

    template <typename T>
    void OctreeNode<T>::insert(const Vector3& pos, std::unique_ptr<T> new_data)
    {
        if (is_leaf())
        {
            // if we are at the lowest available resolution, set new data on this node
            if (half_size < 1.0f)
            {
                // node already contained data - this is bad. issue warning,
                // and recommend to check resolution. alternatively, we could 
                // compute a new value as a average of the two.
                if (data != nullptr)
                {
                    logger << "Warning: node already contains data." << std::endl;
                }
                data = std::move(new_data);
            }
            // not at terminal resolution yet, so split up and insert data into
            // the appropriate child node.
            else
            {
                split();
                nodes[octant_index(pos)]->insert(pos, std::move(new_data));
            }
        }
        // the node is an interiror node to the tree - check which child the
        // data belongs to and call insert on it.
        else
        {
            nodes[octant_index(pos)]->insert(pos, std::move(new_data));
        }
    }

    template <typename T>
    void OctreeNode<T>::reduce(void)
    {
        // check if we have anything to do
        if (is_leaf())
            return;
        // check that all child nodes are ready
        bool only_leaves = true;
        for (int i = 0; i < 8; i++)
        {
            nodes[i]->reduce();
            only_leaves &= nodes[i]->is_leaf();
        }
        if (only_leaves)
        {
            // all children are leaves, check if they all have the same data
            for (int i = 1; i < 8; i++)
            {
                if (nodes[0]->data != nodes[i]->data)
                    return;
            }
            // all children are equal, so reduce 
            data = std::move(nodes[0]->data);
            join();
        }
    }

    template <typename T>
    T* OctreeNode<T>::sample(float tx0, float ty0, float tz0, float tx1, float ty1, float tz1, char oidx) const
    {
        if (tx1 < 0 || ty1 < 0 || tz1 < 0) 
        {
            return nullptr;
        } 
        else if (is_leaf()) 
        {
            return data.get();
        }

        auto txm = 0.5f * (tx0 + tx1);
        auto tym = 0.5f * (ty0 + ty1);
        auto tzm = 0.5f * (tz0 + tz1);
        auto cur_node = first_node(tx0, ty0, tz0, txm, tym, tzm);
        T* res = nullptr;
        while (cur_node < 8)
        {
            switch (cur_node)
            {
            case 0:
                res = nodes[oidx]->sample(tx0, ty0, tz0, txm, tym, tzm, oidx);
                cur_node = next_node(txm, 4, tym, 2, tzm, 1);
                break;
            case 1:
                res = nodes[1^oidx]->sample(tx0, ty0, tzm, txm, tym, tz1, oidx);
                cur_node = next_node(txm, 5, tym, 3, tz1, 8);
                break;
            case 2: 
                res = nodes[2^oidx]->sample(tx0, tym, tz0, txm, ty1, tzm, oidx);
                cur_node = next_node(txm, 6, ty1, 8, tzm, 3);
                break;
            case 3:
                res = nodes[3^oidx]->sample(tx0, tym, tzm, txm, ty1, tz1, oidx);
                cur_node = next_node(txm, 7, ty1, 8, tz1, 8);
                break;
            case 4:
                res = nodes[4^oidx]->sample(txm, ty0, tz0, tx1, tym, tzm, oidx);
                cur_node = next_node(tx1, 8, tym, 6, tzm, 5);
                break;
            case 5:
                res = nodes[5^oidx]->sample(txm, ty0, tzm, tx1, tym, tz1, oidx);
                cur_node = next_node(tx1, 8, tym, 7, tz1, 8);
                break;
            case 6:
                res = nodes[6^oidx]->sample(txm, tym, tz0, tx1, ty1, tzm, oidx);
                cur_node = next_node(tx1, 8, ty1, 8, tzm, 7);
                break;
            case 7:
                res = nodes[7^oidx]->sample(txm, tym, tzm, tx1, ty1, tz1, oidx);
                cur_node = 8;
                break;
            }

            if (res != nullptr)	// hit
                break;
        }
        return res;
    }    
}