#pragma once

#include <array>
#include <memory>
#include <vector>
#include "aabb3.h"
#include "buffers.h"
#include "color.h"
#include "game3.h"
#include "plane.h"
#include "mesh.h"
#include "texturecache.h"
#include "vector2.h"

namespace dukat
{
	class ShaderProgram;
    struct VertexAttribute;
    class HeightMap;

    struct ClipMapLevel
    {
        static const uint8_t bottom_right = 0;
        static const uint8_t top_right = 1;
        static const uint8_t bottom_left = 2;
        static const uint8_t top_left = 3;

        const int index;
        Vector2 origin; // current origin of this level
        int u, v; // origin in texture space 
        Vector2 last_shift; // last change to origin to this level
        uint8_t orientation; // flag indicating position of this level within next coarser level
        bool is_dirty;

        // Commonly used values
        float scale;
        float width;
        float half_width;

        // Precomputed bounding boxes for each level 
        static constexpr int bb_inner_idx = 0;
        static constexpr int bb_block_idx = 4;
		std::array<AABB3, 16> bounding_boxes;

        ClipMapLevel(int index) : index(index), is_dirty(true), origin(0.0f, 0.0f), u(0), v(0), last_shift(0.0f, 0.0f) { }
        ~ClipMapLevel(void) { }

        // Shifts origin and attached bounding boxes
        void translate(const Vector2& offset);

		// Orientation helpers
		inline bool is_left(void) const { return (orientation & 0x2) == 0x2; }
		inline bool is_right(void) const { return (orientation & 0x2) == 0x0; }
		inline bool is_top(void) const { return (orientation & 0x1) == 0x1; }
		inline bool is_bottom(void) const { return (orientation & 0x1) == 0x0; }
    };

    class ClipMap : public Mesh
    {
    private:
		const int num_levels;
		const int level_size;
		const int texture_size;
		Game3* game;
		HeightMap* height_map; // Height map data
		std::vector<ClipMapLevel> levels;
		int min_level; // min level to render - based on height of observer

		// Clipmap meshes
        std::unique_ptr<MeshData> inner_mesh;
        std::unique_ptr<MeshData> block_mesh;
        std::unique_ptr<MeshData> ring_mesh;
        std::array<std::unique_ptr<MeshData>, 4> fill_mesh;
        std::unique_ptr<MeshData> perimeter_mesh;
		std::array<Vector2, 4> inner_offsets; // Offsets for <I> blocks
		std::array<Vector2, 12> block_offsets; // Offsets for <B> blocks

		ShaderProgram* program; // used to render final terrain
		std::unique_ptr<Texture> elevation_maps; // 1-channel GL_R32F texture array for elevation data
        std::unique_ptr<Texture> normal_maps; // 2-channel GL_RG16F texture array for normal data
		std::unique_ptr<Texture> color_map; // 1-dimensional RGB texture used to color terrain

        ShaderProgram* update_program; // used to update elevation sampler 
		std::vector<GLfloat> buffer; // Buffer used to update elevation samplers.
        std::unique_ptr<FrameBuffer> fb_update; // frame buffer to update elevation sampler 
        std::unique_ptr<MeshData> quad_update; // quad mesh used to update elevation sampler
		std::unique_ptr<Texture> update_texture; // 1-channel GL_R32F texture used to update elevation maps.

		ShaderProgram* normal_program; // used to generate normal maps
		std::unique_ptr<FrameBuffer> fb_normal; // frame buffer to generate normal textures
		std::unique_ptr<MeshData> quad_normal; // quad mesh used to update normal shader

		void build_levels(void);
        void build_color_sampler(void);
        void build_buffers(void);
        void build_ring_buffer(int block_size);
        void build_fill_buffer(int block_size);
        void build_perimeter_buffer(void);

        void update_levels(void);
        // Updates elevation samplers and returns index of coarsest level that was updated.
        int update_elevation_maps(void);
        // Updates normal samplers starting at max_index to finest grained level.
        void update_normal_maps(int max_index);
        void render_level(const Camera3& cam, int i);

    public:
        bool wireframe;
        bool culling;
        bool stitching;
		bool blending;
        bool lighting;
        // Observer (i.e., camera) position in world space
		Vector3 observer_pos;
        
        // Creates a new clipmap.
        ClipMap(Game3* game, int num_levels, int level_size, HeightMap* height_map);
        ~ClipMap(void) { }

		// Sets clipmap shader.
		void set_program(ShaderProgram* program) { this->program = program; }
		// Sets the color palette used to shade the terrain.
        void set_palette(const std::vector<Color>& palette);
		void update(float delta);
        void render(Renderer* renderer);
        
        // Testing
        Texture* get_elevation_map(void) { return elevation_maps.get(); }
        Texture* get_normal_map(void) { return normal_maps.get(); }
    };
}