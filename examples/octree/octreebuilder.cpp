#include "stdafx.h"
#include "octreebuilder.h"

namespace dukat
{
    std::unique_ptr<OctreeNode<SDL_Color>> OctreeBuilder::build_empty(int size)
    {
        int half_size = size / 2;
        // make sure size is a power of 2
        assert(size == half_size * 2);
        return std::make_unique<OctreeNode<SDL_Color>>(Vector3::origin, (float)half_size);
    }

    std::unique_ptr<OctreeNode<SDL_Color>> OctreeBuilder::build_cube(int size)
    {
        logger << "Building cube with side: " << size << std::endl;
        size = next_pow_two(size);
        auto root = build_empty(size);
    	auto half_size = size / 2;
        for (int x = 0; x < size; x++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int z = 0; z < size; z++)
                {
                    if (x == 0 || x == size - 1 || 
                        y == 0 || y == size - 1 ||
                        z == 0 || z == size - 1)
                    {
                        auto data = std::make_unique<SDL_Color>();
                        data->r = (Uint8)(255.0f * (float)x / (float)size);
                        data->g = (Uint8)(255.0f * (float)y / (float)size); 
                        data->b = (Uint8)(255.0f * (float)z / (float)size);
                        data->a = 0xff;

                        root->insert(
                            Vector3(0.5f + (float)(x - half_size), 0.5f + (float)(y - half_size), 0.5f + (float)(z - half_size)),
                            std::move(data)
                        );
                    }
                }
            }
        }

        logger << "Size before reduction: " << root->count() << std::endl;
        root->reduce();
        logger << "Size after reduction: " << root->count() << std::endl;

        return std::move(root);
    }

    std::unique_ptr<OctreeNode<SDL_Color>> OctreeBuilder::build_sphere(int radius)
    {
        logger << "Building sphere with radius: " << radius << std::endl;
        auto size = next_pow_two(2 * radius);
        auto root = build_empty(size);

        auto rad2 = ((float)radius - 0.5f) * ((float)radius - 0.5f);
        for (int x = -radius; x < radius; x++)
        {
            for (int y = -radius; y < radius; y++)
            {
                for (int z = -radius; z < radius; z++)
                {
                    auto px = 0.5f + (float)x;
                    auto py = 0.5f + (float)y;
                    auto pz = 0.5f + (float)z;
                    auto len = (px * px + py * py + pz * pz);
                    if (abs(len - rad2) < radius)	// how odd - what's the relation between the 
                                                    // squared distances?
                    {
                        auto data = std::make_unique<SDL_Color>();
                        data->r = (Uint8)(255.0f * (float)x / (float)size);
                        data->g = (Uint8)(255.0f * (float)y / (float)size); 
                        data->b = (Uint8)(255.0f * (float)z / (float)size);
                        data->a = 0xff;

                        root->insert(
                            Vector3(px, py, pz),
                            std::move(data)
                        );
                    }
                }
            }
        }
        return std::move(root);
    }

#ifdef NOISE_ENABLED
    std::vector<SDL_Color> terrain_palette;

    SDL_Color rgb(float r, float g, float b)
    {
        return { (Uint8)(255.0f * r), (Uint8)(255.0f * g), (Uint8)(255.0f * b), 0xff };
    }

    /**
    * Builds a palette for M-class terrain. 
    * Given height-map values between 0..1, the following colors are applied:
    * [0..0.5]		Water (blue)
    * [0.5..0.55]	Beach (yellow)
    * [0.55..0.75]	Plains (green)
    * [0.75..0.90] Low mountains (brown)
    * [0.90..1]    High mountains (gray -> white)
    */
    void build_palette()
    {
        terrain_palette.resize(256);

        int sea_size = (int)(terrain_palette.size() * 0.5f);
        int beach_size = (int)(terrain_palette.size() * 0.05f) + sea_size;
        int plain_size = (int)(terrain_palette.size() * 0.2f) + beach_size;
        int low_mnt_size = (int)(terrain_palette.size() * 0.1f) + plain_size;
        int high_mnt_size = (int)(terrain_palette.size() * 0.1f) + low_mnt_size;
        int i;

        /* sea colors (blue) */
        for(i = 0; i <= sea_size; i++) {
            float perc = (float)i / (float)sea_size;
            terrain_palette[i] = rgb(0.0f, 0.0f, 105.0f/256.0f * perc + 150.0f/256.0f);
        }

        /* yellow */
        for(i = sea_size+1; i <= beach_size; i++) {
            float perc = (float)(i - sea_size - 1) / (float)(beach_size - sea_size);
            terrain_palette[i] = rgb(55.0f/256.0f * perc + 200.0f/256.0f,
                    40.0f/256.0f * perc + 170.0f/256.0f,
                    0.0f);
        }

        /* plain size (green) */
        for(i = beach_size+1; i <= plain_size; i++) {
            float perc = (float)(i - beach_size - 1)/ (float)(plain_size - beach_size);
            terrain_palette[i] = rgb(0.0f, 135.0f/256.0f * perc + 120.0f/256.0f, 0.0f);
        }

        /* low_mnt size (brown) */
        for(i = plain_size+1; i <= low_mnt_size; i++) {
            float perc = (float)(i - plain_size - 1) / (float)(low_mnt_size - plain_size);
            terrain_palette[i] = rgb(100.0f/256.0f * perc + 130.0f/256.0f,
                    70.0f/256.0f * perc + 80.0f/256.0f,
                    0.0f);
        }

        /* hight_mnt_size size (gray -> white) */
        /*printf("%d/%d\n", high_mnt_size, _size);*/
        for(i = low_mnt_size+1; i < (int)terrain_palette.size(); i++) {
            float perc = (float)(i - low_mnt_size - 1) / (float)(terrain_palette.size() - low_mnt_size);
            terrain_palette[i] = rgb(0.5f * perc + 0.5f, 0.5f * perc + 0.5f, 0.5f * perc + 0.5f);
        }
    }

	void calculate_min_max(utils::NoiseMap& noise_map, float& min, float& max)
	{
		min = 1.0f;
		max = -1.0f;
		for (int row = 0; row < noise_map.GetHeight(); row++)
		{
			float* ptr = noise_map.GetSlabPtr(row);
			for (int col = 0; col < noise_map.GetWidth(); col++)
			{
				min = std::min(min, *ptr);
				max = std::max(max, *ptr);
				ptr++;
			}
		}
	}

    std::unique_ptr<OctreeNode<SDL_Color>> OctreeBuilder::build_planetoid(int radius, int surface_height)
    {
        if (terrain_palette.size() == 0)
            build_palette();

        logger << "Building planetoid with radius: " << radius << std::endl;
        auto size = next_pow_two(2 * (radius + surface_height));
        auto root = build_empty(size);
        
        // Generate terrain modules - do this once somewhere
        // from http://libnoise.sourceforge.net/tutorials/tutorial5.html
        module::Perlin perlin_terrain;
        module::RidgedMulti mountain_terrain;
        module::Billow base_flat_terrain;
        base_flat_terrain.SetFrequency(2.0);
        module::ScaleBias flat_terrain;
        flat_terrain.SetSourceModule(0, base_flat_terrain);
        flat_terrain.SetScale(0.125);
        flat_terrain.SetBias(-0.75);
        module::Perlin terrain_type;
        terrain_type.SetFrequency(0.5);
        terrain_type.SetPersistence(0.25);
        module::Select final_terrain;
        final_terrain.SetSourceModule(0, flat_terrain);
        final_terrain.SetSourceModule(1, mountain_terrain);
        final_terrain.SetControlModule(terrain_type);
        final_terrain.SetBounds(0.0, 1000.0);
        final_terrain.SetEdgeFalloff(0.125);

        // Generate a height_map to color our sphere
        utils::NoiseMap height_map;
        utils::NoiseMapBuilderSphere height_map_builder;
        height_map_builder.SetSourceModule(perlin_terrain);
        height_map_builder.SetDestNoiseMap(height_map);
        height_map_builder.SetDestSize(4 * radius, 2 * radius);
        height_map_builder.SetBounds(-90.0, 90.0, -180.0, 180.0);
        height_map_builder.Build();
        float low, high;
		calculate_min_max(height_map, low, high);

        auto rad2 = ((float)radius - 0.5f) * ((float)radius - 0.5f);
        for (int x = -radius; x < radius; x++)
        {
            for (int y = -radius; y < radius; y++)
            {
                for (int z = -radius; z < radius; z++)
                {
                    auto px = 0.5f + (float)x;
                    auto py = 0.5f + (float)y;
                    auto pz = 0.5f + (float)z;
                    auto len = (px * px + py * py + pz * pz);
                    if (abs(len - rad2) < radius)	// how odd - what's the relation between the 
                                                    // squared distances?
                    {
                        // Generated UV coordinates based on longitude and latitude
                        auto u = (int)(((std::atan2(pz, px) + pi) / two_pi) * height_map.GetWidth());
                        auto v = (int)(((std::asin(py / (float)radius) + pi_over_two) / pi) * height_map.GetHeight());

                        auto value = height_map.GetValue(u, v);
                        // normalize value to 0..1
                        value = std::min(0.9999999f, (value - low) / (high - low));
                        auto& c = terrain_palette[(int)(value * terrain_palette.size())];

						// Add voxels from position on radius to position + height
						Vector3 pos(px, py, pz);
						Vector3 step = pos / std::sqrt(len);
						int steps = (int)(surface_height * value);
						for (int i = 0; i <= steps; i++)
						{
							root->insert(pos + step * (float)i, std::make_unique<SDL_Color>(c));
						}
                    }
                }
            }
        }
        return std::move(root);
    }
#endif
}