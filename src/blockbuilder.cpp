#include "stdafx.h"
#include <dukat/blockbuilder.h>
#include <dukat/buffers.h>
#include <dukat/renderer3.h>

namespace dukat
{
    BlockBuilder::BlockBuilder(int block_size) : block_size(block_size)
    { 
        attributes.push_back(VertexAttribute(Renderer::at_pos, 2, 0, GL_SHORT));
    }

    void BlockBuilder::add_block(int width, int height, int xoffset, int yoffset)
    {
        // add primitive restart unless this is the first block
        auto idx = vertex_data.size() / 2;
        if (idx > 0)
        {
            index_data.push_back(-1);
            // alternative for connected blocks is to introduce degenerate triangles:
            // index_data.push_back(idx);
            // index_data.push_back(idx + 1);
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                vertex_data.push_back(static_cast<GLshort>(xoffset + block_size * x));
                vertex_data.push_back(static_cast<GLshort>(yoffset + block_size * y));
            }
        }

        for (int y = 0; y < height - 1; y++)
        {
            for (int x = 0; x < width - 1; x++)
            {
                index_data.push_back(static_cast<GLushort>(idx));
                index_data.push_back(static_cast<GLushort>(idx + width));
                index_data.push_back(static_cast<GLushort>(idx + 1));
                index_data.push_back(static_cast<GLushort>(idx + width + 1));
                idx++;
            }

            // degenerate triangle for all but bottom row
            if (y < (height - 2))
            {
                index_data.push_back(static_cast<GLushort>(idx + width));
                idx++;
                index_data.push_back(static_cast<GLushort>(idx));
            }
        }
    }

    void BlockBuilder::bind_data(VertexBuffer& vertex_buffer, int vertex_pos, int index_pos)
    {
        assert(vertex_pos < vertex_buffer.buffer_count);
        assert(index_pos < vertex_buffer.buffer_count);
        vertex_buffer.counts[vertex_pos] = get_vertex_count();
        vertex_buffer.strides[vertex_pos] = 2 * sizeof(GLshort);
        auto size = vertex_buffer.counts[vertex_pos] * vertex_buffer.strides[vertex_pos];
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.buffers[vertex_pos]);
        glBufferData(GL_ARRAY_BUFFER, size, vertex_data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        vertex_buffer.counts[index_pos] = get_index_count();
        vertex_buffer.strides[index_pos] = sizeof(GLushort);
        size = vertex_buffer.counts[index_pos] * vertex_buffer.strides[index_pos];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer.buffers[index_pos]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index_data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    std::unique_ptr<MeshData> BlockBuilder::create_mesh(void)
    {
        auto mesh = std::make_unique<MeshData>(GL_TRIANGLE_STRIP, get_vertex_count(), 
            get_index_count(), attributes);
        mesh->set_vertices(vertex_data.data());
        mesh->set_indices(index_data.data());
        return mesh;
    }

    void BlockBuilder::clear(void)
    {
        vertex_data.clear();
        index_data.clear();
    }
}