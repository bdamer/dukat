#pragma once

#include <stdint.h>
#include "octreenode.h"

namespace dukat
{
    struct VoxHeader
    {
        uint32_t id;
        uint32_t version;
        uint32_t node_offset;
        uint32_t node_count;
        float_t origin[3];
        float_t dimension;
    };

    struct VoxNode
    {
        uint32_t flags;
        uint32_t children[8];
    };

    class VoxModel
    {
    private:
        static const uint32_t vox_id = 0x6d786f76; // voxm
        static const uint32_t vox_version = 1;
        VoxHeader header;
        std::unique_ptr<OctreeNode<SDL_Color>> octree;
        // Helper function to recursively load octree nodes.
        void load_node(OctreeNode<SDL_Color>* cur_node, const std::vector<VoxNode>& nodes, int idx);

    public:
        VoxModel(void) : header(), octree(nullptr) { }
        ~VoxModel(void) { }

        // Data accessor
        std::unique_ptr<OctreeNode<SDL_Color>> get_data(void) { return std::move(octree); }
        void set_data(std::unique_ptr<OctreeNode<SDL_Color>> octree) { this->octree = std::move(octree); }

        // Stream I/O
        friend std::ostream& operator<<(std::ostream& os, const VoxModel& m);
        friend std::istream& operator>>(std::istream& is, VoxModel& m);
    };
}