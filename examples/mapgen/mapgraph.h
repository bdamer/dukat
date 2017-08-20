#pragma once

#include <vector>
#include <dukat/meshdata.h>
#include <dukat/vector2.h>

namespace dukat
{
    enum Biome
    {
        Ocean,
        Marsh,
        Ice,
        Lake,
        Beach,
        Snow,
        Tundra,
        Bare,
        Scorched,
        Taiga,
        Shrubland,
        TemperateDesert,
        TemperateRainForest,
        TemperateDeciduousForest,
        Grassland,
        TropicalRainForest,
        TropicalSeasonalForest,
        SubtropicalDesert
    };

    class MapGraph
    {
    public:
        struct Edge; 
        struct Corner;

        // The center of a map polygon.
        struct Center
        {
            int index;
            Vector2 pos;
            bool ocean;
            bool water;
            bool coast;
            bool border;
            Biome biome;
            float elevation; // [0..1]
            float moisture; // [0..1]
            
            std::vector<Center*> neighbors;
            std::vector<Edge*> borders;
            std::vector<Corner*> corners;
            
            Center(void) : index(-1), ocean(false), water(false), 
                coast(false), border(false), elevation(0.0f), moisture(0.0f) { }
        };

        // A corner of a map polygon.
        struct Corner 
        {
            int index;
            Vector2 pos;
            bool ocean;
            bool water;
            bool coast;
            bool border;
            float elevation;
            float moisture;
            int river; // 0 if no river or volume of water in river
            int watershed_size;
            Corner* downslope; // points to adjacent corner most downhill
            Corner* watershed; // points to coastal corner or null
            
            std::vector<Center*> touches;
            std::vector<Edge*> protrudes;
            std::vector<Corner*> adjacent;

            Corner(void) : index(-1), ocean(false), water(false), 
                coast(false), border(false), elevation(0.0f), 
                moisture(0.0f), river(0), watershed_size(0),
                downslope(nullptr), watershed(nullptr) { }
        };
    
        // The edge between two map polygons.
        struct Edge 
        {
            int index; 
            Center *d0, *d1; // delaunay edge
            Corner *v0, *v1; // voronoi edge
            Vector2 midpoint; // halfway between v0, v1
            int river; // volume of water or 0
            
            Edge(void) : index(-1), d0(nullptr), d1(nullptr), 
                v0(nullptr), v1(nullptr), river(0) { }
        };

    private:
        std::vector<Vector2> points;
        std::vector<std::unique_ptr<Center>> centers;
        std::vector<std::unique_ptr<Corner>> corners;
        std::vector<std::unique_ptr<Edge>> edges; 

        Center* create_center(void);
        Corner* create_corner(void);
        Edge* create_edge(void);
        
        // Create an array of corners that are on land only, for use by algorithms that work only on land.
        std::vector<Corner*> get_land_corners(void);
        // Look up a Voronoi Edge object given two adjacent Voronoi
        // polygons, or two adjacent Voronoi corners
        Edge* lookup_edge_from_center(Center* p, Center* r);
        Edge* lookup_edge_from_corner(Corner* q, Corner* s);

        void assign_corner_elevation(void);
        // Determine polygon and corner types: ocean, coast, land.
        void assign_ocean_coast_and_land(void);
        // Change the overall distribution of elevations so that lower elevations are more common than higher
        // elevations. Specifically, we want elevation X to have frequency (1-X).  
        // To do this we will sort the corners, then set each corner to its desired elevation.
        void redistribute_elevations(void);
        // Polygon elevations are the average of the elevations of their corners.
        void assign_polygon_elevations(void);
        
        // Calculate downslope pointers.  At every point, we point to the
        // point downstream from it, or to itself.  This is used for
        // generating rivers and watersheds.
        void calculate_downslopes(void);
        // Calculate the watershed of every land point. The watershed is
        // the last downstream land point in the downslope graph. TODO:
        // watersheds are currently calculated on corners, but it'd be
        // more useful to compute them on polygon centers so that every
        // polygon can be marked as being in one watershed.
        void calculate_watersheds(void);
        // Create rivers along edges. Pick a random corner point, then
        // move downslope. Mark the edges and corners as rivers.
        void create_rivers(void);
        // Calculate moisture. Freshwater sources spread moisture: rivers
        // and lakes (not oceans). Saltwater sources have moisture but do
        // not spread it (we set it at the end, after propagation).
        void assign_corner_moisture(void);
        // Change the overall distribution of moisture to be evenly distributed.
        void redistribute_moisture(void);
        // Polygon moisture is the average of the moisture at corners
        void assign_polygon_moisture(void);

        // Assign a biome type to each polygon. If it has
        // ocean/coast/water, then that's the biome; otherwise it depends
        // on low/high elevation and low/medium/high moisture. This is
        // roughly based on the Whittaker diagram but adapted to fit the
        // needs of the island map generator.
        void assign_biomes(void);

        // Resets internal state
        void reset(void);

    public:
        MapGraph(void);
        ~MapGraph(void);
        
        bool show_delaunay;
        bool show_voronoi;
        
        // Creates a graph based on a regular grid. The points will be normalized [-1..1].
        void from_grid(int size, int seed);
        // Creates a graph for a list of input points. The points are assumed to be normalized [-1..1].
        void from_points(const std::vector<Vector2>& points);

        void generate(void);

        // Generates water / land mesh.
        void create_water_land_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh indicating elevation of each map cell. 
        void create_elevation_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh indicating moisture of each map cell.
        void create_moisture_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh to showcase biome of each map cell.
        void create_biomes_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh of polygon outlines
        void create_edge_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh of rivers
        void create_river_mesh(MeshData* mesh, float z_scale = 1.0f);
    };
}
