#pragma once

#include <boost/polygon/voronoi.hpp>

#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

#include "aabb2.h"
#include "vector2.h"

namespace dukat
{
    class VoronoiDiagram
    {
    public:
        typedef Vector2 Point;

        struct Cell;

        struct Edge
        {
            Point* v0;
            Point* v1;
            Edge* twin;
            Cell* cell;

            Edge(void) : v0(nullptr), v1(nullptr), twin(nullptr), cell(nullptr) { }
        };

        struct Cell
        {
            int id;
            Point* site;
            std::list<std::unique_ptr<Edge>> edges;
            std::unordered_set<Cell*> neighbors;

            Cell(void) : id(-1), site(nullptr) { }
        };

    private:
        // Bounding box used for clipping.
        AABB2 bb;
        // Boost voronoi only supports integer, so we need to scale site coordinates.
        // Any input coordinate wil be scale between bb-min and bb-max, using max_scale_extent
        // discrete values.
        const float max_scale_extent;
        Vector2 scale_factor;
        Vector2 inv_scale_factor;
        // Boost voronoi object used to compute diagram
        typedef boost::polygon::voronoi_diagram<double> voronoi_diagram;
        std::unique_ptr<voronoi_diagram> vd;
        // Source point sites
        std::vector<Point> sites;
        // Cell / Edge vertices
        std::list<std::unique_ptr<Point>> vertices;
        // Cells
        std::vector<std::unique_ptr<Cell>> cells;
        // List of edge vertices that lie on bounding box in clockwise order.
        std::list<Point*> border_vertices;

        // Resets internal state and frees up data structures.
        void reset(void);
        // Adds vertex to list and pointer to it.
        Point* create_vertex(const Point& p);
        // Collects cells and edges from diagram into member variables.
        void collect_cells(void);
        bool clip_vector(const Vector2& inv, const Vector2& dir, Vector2& outv);
        // Clips an edge against the bounding box and returns the resulting edge or
        // nullptr if the edge could not be clipped.
        std::unique_ptr<Edge> clip_edge(const voronoi_diagram::edge_type& src_edge);
        // Inserts links between border vertices from and to.
        void insert_link(Point* from, Point* to, Cell* cell, std::list<std::unique_ptr<Edge>>::iterator& it);
        // Returns the side of the bounding box a point lies on.
        int check_side(Point* point) const;
        void add_border_vertex(Point* point);
        // Adds mising edges to cells that contained clipped edges
        void close_cells(void);
        // Walks the bounding border in clock-wise order from a starting point
        // to the next point, which is then returned. Will return nullptr if 
        // no next point can be found.
        Point* border_walk(Point* start);

    public:
        // Generates the voronoi diagram for the given sites within the provided bounding box.
        VoronoiDiagram(const std::vector<Vector2>& sites, const AABB2& bb);
        ~VoronoiDiagram(void) { }
        // If iterations > 1, Lloyd relaxation will be performed between each pass.
        void compute(int iterations = 1);
        const std::vector<std::unique_ptr<Cell>>& get_cells(void) const { return cells; }
    };
}