#include "stdafx.h"
#include <dukat/voxmodel.h>
#include <dukat/log.h>
#include <queue>

namespace dukat
{
    void VoxModel::load_node(OctreeNode<SDL_Color>* cur_node, const std::vector<VoxNode>& nodes, int idx)
    {
        cur_node->split();
        for (int i = 0; i < 8; i++)
        {
            auto child = cur_node->get_child(i);
            uint32_t flag = 1 << i;
            if ((nodes[idx].flags & flag) == flag)
            {
                load_node(child, nodes, nodes[idx].children[i]);
            }
            else
            {
                SDL_Color* src = (SDL_Color*)&nodes[idx].children[i];
                if (src->a != 0)
                {
                    auto data = std::make_unique<SDL_Color>();
                    data->r = src->r;
                    data->g = src->g;
                    data->b = src->b;
                    data->a = src->a;
                    child->set_data(std::move(data));
                }
            }
        }
    }

    std::ostream& operator<<(std::ostream& os, const VoxModel& m)
    {
        VoxHeader header;
        header.id = m.vox_id;
        header.version = m.vox_version;
        header.node_offset = sizeof(VoxHeader);
        header.origin[0] = m.octree->origin.x;
        header.origin[1] = m.octree->origin.y;
        header.origin[2] = m.octree->origin.z;
        header.dimension = m.octree->half_size;
        header.node_count = 0;

        std::vector<VoxNode> nodes;
        std::queue<OctreeNode<SDL_Color>*> queue;
        queue.push(m.octree.get());

        while (!queue.empty())
        {
            auto cur = queue.front();
            queue.pop();
            if (cur->is_leaf())
                continue; // shouldn't happen unless root is a leaf
            
            nodes.push_back(VoxNode());
            auto& node = nodes[nodes.size() - 1];
            node.flags = 0;

            for (int i = 0; i < 8; i++)
            {
                auto child = cur->get_child(i);
                if (child->is_leaf())
                {
                    // set color element
                    auto data = child->get_data();
                    node.children[i] = (data == nullptr ? 0 : *(reinterpret_cast<uint32_t*>(data)));
                }
                else
                {
                    // set node address
                    header.node_count++;
                    node.flags |= (1 << i);
                    node.children[i] = header.node_count;
                    queue.push(child);
                }
            }
        }

        header.node_count++;

        // write header
        os.write(reinterpret_cast<const char*>(&header.id), sizeof(uint32_t));
        os.write(reinterpret_cast<const char*>(&header.version), sizeof(uint32_t));
        os.write(reinterpret_cast<const char*>(&header.node_offset), sizeof(uint32_t));
        os.write(reinterpret_cast<const char*>(&header.node_count), sizeof(uint32_t));
        os.write(reinterpret_cast<const char*>(&header.origin), 3 * sizeof(float_t));
        os.write(reinterpret_cast<const char*>(&header.dimension), sizeof(float_t));
        // write nodes
        os.write(reinterpret_cast<const char*>(nodes.data()), sizeof(VoxNode) * nodes.size());

        return os;
    }

    std::istream& operator>>(std::istream& is, VoxModel& m)
    {
        is.read(reinterpret_cast<char*>(&m.header.id), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&m.header.version), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&m.header.node_offset), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&m.header.node_count), sizeof(uint32_t));
		is.read(reinterpret_cast<char*>(&m.header.origin), 3 * sizeof(float_t));
		is.read(reinterpret_cast<char*>(&m.header.dimension), sizeof(float_t));
		if (m.header.id != VoxModel::vox_id || m.header.version != VoxModel::vox_version)
		{
			throw std::runtime_error("Invalid model format or version!");
		}

        is.seekg(m.header.node_offset);

        std::vector<VoxNode> nodes(m.header.node_count);
        is.read(reinterpret_cast<char*>(nodes.data()), m.header.node_count * sizeof(VoxNode));

        Vector3 origin(m.header.origin[0], m.header.origin[1], m.header.origin[2]);
        m.octree = std::make_unique<OctreeNode<SDL_Color>>(origin, m.header.dimension);
        m.load_node(m.octree.get(), nodes, 0);

        return is;
    }
}