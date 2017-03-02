#pragma once

#include <vector>
#include "mesh.h"

namespace dukat
{
    class Mesh;
    struct VertexBuffer;

    class BlockBuilder
    {
    private:
        const int block_size;
        // Vertices contain x/y position on the grid - no need for GLfloat precision
        std::vector<GLshort> vertex_data;
        std::vector<GLushort> index_data;
        std::vector<VertexAttribute> attributes;

    public:
        BlockBuilder(int block_size = 1);
        ~BlockBuilder(void) { }

        // Adds a new block with a given with and height and optional origin.
        void add_block(int width, int height, int xoffset = 0, int yoffset = 0);
        // Binds the vertex and index data to the provided buffers.
        void bind_data(VertexBuffer& vertex_buffer, int vertex_pos = 0, int index_pos = 1);
        // Creates a new mesh from this buffer.
        std::unique_ptr<Mesh> create_mesh(void);

        // Resets all state associated with this block builder.
        void clear(void);

        int get_vertex_count(void) const { return (int)vertex_data.size() / 2;  }
        int get_index_count(void) const { return (int)index_data.size(); }
    };
}