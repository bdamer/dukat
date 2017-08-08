#pragma once

#include "aabb3.h"
#include "material.h"
#include "sysutil.h"
#include "transform3.h"

#include <memory>

namespace dukat
{
    class GameBase;
    class MeshGroup;

    class Model3
    {
    public:
        static constexpr size_t string_length = 256;
        static constexpr uint32_t model_id = mc_const('d','o','m','d');
        static constexpr uint32_t model_version = 1;

        enum VertexFormat
        {
            PosNorTex,  // position, normal, texture coordinates
            PosNorClr   // position, normal, color
        };

        struct Vertex
        {
            GLfloat pos[3]; // position data
            GLfloat nor[3]; // normal data
            GLfloat tex[2]; // TODO: either 4 float RGBA or 2 float UV coordinates
        };

        struct Header
        {
            uint32_t id;
            uint32_t version;
            char name[string_length];
            uint32_t mesh_offset;
            uint32_t mesh_count;
            uint32_t index_offset;
            uint32_t index_count;
            uint32_t vertex_offset;
            uint32_t vertex_count;
            
            Header(void) : id(0), version(0), mesh_offset(0), mesh_count(0),
                index_offset(0), index_count(0), vertex_offset(0), vertex_count(0)
            {
                memset(name, 0, string_length);
            }
        };
        
        struct Mesh
        {
            uint32_t id;
            char name[string_length];
            char texture[string_length];
            Material material;
            Transform3 transform;
            uint32_t index_offset;
            uint32_t index_count;
            uint32_t vertex_offset;
            uint32_t vertex_count;
            
            Mesh(void) : id(0), index_offset(0), index_count(0), vertex_offset(0), vertex_count(0)
            {
                memset(name, 0, string_length);
                memset(texture, 0, string_length);
            }
        };
    
    private:
        Header header;
        std::vector<Mesh> meshes;
        std::vector<GLushort> indices;
        std::vector<Vertex> vertices;
        
        // Updates internal offsets.
        void update_headers(void);

    public:
        Model3(void);
        ~Model3(void);
        // Sets name of this model.
        void set_name(const std::string& name);
        std::string get_name(void) const { return std::string(header.name); }
        // Adds a new mesh to this model.
        void add_mesh(const std::string& name, const dukat::Material& material, const std::string& texture,
            const dukat::Transform3& transform, const std::vector<GLushort>& indices, const std::vector<Vertex>& vertices);
        // Accesors
        const std::vector<Mesh>& get_meshes(void) const { return meshes; }
        const std::vector<GLushort>& get_indices(void) const { return indices; }
        const std::vector<Vertex>& get_vertices(void) const { return vertices; }
        // Creates AABB3 from this model.
        dukat::AABB3 create_aabb(void) const;
        // Stream input / output
        friend std::ostream& operator<<(std::ostream& os, const Model3& v);
        friend std::istream& operator>>(std::istream& is, Model3& v);
    };

	// Utility method to generate mesh group from model
    extern std::unique_ptr<MeshGroup> build_mesh_group(GameBase* game, const Model3& model);
}