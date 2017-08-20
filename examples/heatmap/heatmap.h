#pragma once

#include <memory>
#include <vector>

#include <dukat/buffers.h>
#include <dukat/game3.h>
#include <dukat/meshdata.h>
#include <dukat/mesh.h>
#include <dukat/heightmapgenerator.h>
#include <dukat/heightmap.h>
#include <dukat/texturecache.h>

namespace dukat
{
	class ShaderProgram;

    class HeatMap : public Mesh
    {
    public:
        struct Cell
        {
            float t; // Temperature
            float v; // Vegetation
            float delta;

            Cell(void) : t(0.0f), v(0.0f), delta(0.0f) { }
        };

        struct Emitter
        {
            int x, y; // cell index
            bool active; // state
            float max_emission; // max outflow
            float phase; // current phase value
            float period; // duration of emission period

            Emitter(void) : x(-1), y(-1), active(false) { }
            Emitter(int x, int y) : x(x), y(y), max_emission(2.0f), phase(0.0f), period(1.0f), active(true) { }
        };

    private:
        // Simulation constants
        // Amount of heat lost due to dissipation.
        static const float dissipation_rate;
        // Factor of heat transfer. If set to 0, all heat is transferred, otherwise the ratio 
        // is dependent on transfer_factor * slope.
        static const float transfer_factor;
        // Elevation increase relative to dissipation rate.
        static const float elevation_increase;
        // Growth rate for vegetation.
        static const float growth_rate;
        // Rate at which vegetation is "burned" when temperature is greater than veg threshold.
        static const float burn_rate;
        // Min threshold for heat transmission.
        static const float transmission_threshold;
        // Max threshold for vegetation growth.
        static const float vegetation_threshold;
        // Range of heat emission.
        static const float min_emission;
        static const float max_emission;
        // Length of period of emission - affects how wide a lava flow spreads
        static const float min_period;
        static const float max_period;

        Game3* game;
        int map_size; // width / height of heat map
        int tile_spacing; // size of each map tile
        std::vector<Emitter> emitters; // heat emitters
        std::vector<Cell> cells; // cells of heatmap
        std::unique_ptr<HeightMap> heightmap; // elevation data

		ShaderProgram* program; // used to render elevation mesh
        std::unique_ptr<MeshData> grid_mesh; // elevation mesh
        std::unique_ptr<Texture> heightmap_texture; // 1-channel GL_R32F texture used for elevation data
		std::unique_ptr<Texture> heatmap_texture; // 3-channel GL_RGB8 texture used to update the heat map
		std::unique_ptr<Texture> terrain_texture; // RGB texture array for texture splatting.

        ShaderProgram* normal_program; // used to generate normal sampler
        std::unique_ptr<FrameBuffer> fb_normal; // frame buffer to update heat sampler 
        std::unique_ptr<MeshData> quad_normal; // quad mesh used to update normal sampler
        
        void emitter_phase(float delta);
        void compute_phase(float delta);
        void update_phase(float delta);
        void update_textures(void);

    public:
        HeatMap(Game3* game, int map_size);
        ~HeatMap(void) { }

        // Resets the state of the heat map, preserving existing emitters.
        void reset(void);
        // Loads terrain from file.
		void load(const std::string& filename);
        // Saves terrain to file.
		void save(const std::string& filename) const;
        // TODO: maybe just use a setter for heightmap?
		void generate(const HeightMapGenerator& generator);

        // Adds a new emitter cluster to heat map. 
        void add_emitters(int num_emitters, int radius);
        // Turns all emitters on or off.
        void toggle_emitters(void);

        // Updates the state of the heat map.
        void update(float delta);
        // Renders this heat map.
        void render(Renderer* renderer);

        void set_scale_factor(float scale_factor) { heightmap->set_scale_factor(scale_factor); }
        float get_scale_factor(void) const { return heightmap->get_scale_factor(); }
        HeightMap* get_height_map(void) const { return heightmap.get(); }
        void set_tile_spacing(int tile_spacing) { this->tile_spacing = tile_spacing; }
        int get_tile_spacing(void) const { return tile_spacing; }
    };
}