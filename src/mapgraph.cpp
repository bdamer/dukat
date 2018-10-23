#include "stdafx.h"
#include "log.h"
#include "mapgraph.h"
#include "mapshape.h"
#include "mathutil.h"
#include "vertextypes3.h"
#include "voronoi.h"

#include <map>

namespace dukat
{
    MapGraph::MapGraph(void)
    {
    }

    MapGraph::~MapGraph(void)
    {
    }

    void MapGraph::reset(void)
    {
        points.clear();
        centers.clear();
        corners.clear();
        edges.clear();
    }

    MapGraph::Center* MapGraph::create_center(void)
    {
        auto center = std::make_unique<Center>();
        center->index = centers.size();
        auto res = center.get();
        centers.push_back(std::move(center));
        return res;
    }

    MapGraph::Corner* MapGraph::create_corner(void)
    {
        auto corner = std::make_unique<Corner>();
        corner->index = corners.size();
        auto res = corner.get();
        corners.push_back(std::move(corner));
        return res;
    }

    MapGraph::Edge* MapGraph::create_edge(void)
    {
        auto edge = std::make_unique<Edge>();
        edge->index = edges.size();
        auto res = edge.get();
        edges.push_back(std::move(edge));
        return res;
    }

    std::vector<MapGraph::Corner*> MapGraph::get_land_corners(void)
    {
        std::vector<Corner*> res;
        for (const auto& c : corners)
        {
            if (!c->ocean && !c->coast)
            {
                res.push_back(c.get());
            }
        }
        return res;
    }

    MapGraph::Edge* MapGraph::lookup_edge_from_center(Center* p, Center* r)
    {
        for (auto edge : p->borders)
        {
            if (edge->d0 == r || edge->d1 == r)
                return edge;
        }
        return nullptr;
    }

    MapGraph::Edge* MapGraph::lookup_edge_from_corner(Corner* q, Corner* s)
    {
        for (auto edge : q->protrudes)
        {
            if (edge->v0 == s || edge->v1 == s)
                return edge;
        }
        return nullptr;
    }

    void MapGraph::from_points(const std::vector<Vector2>& points)
    {
        reset();

        // map to keep track of original cell -> center so we
        // look up neighboring cells
        std::map<VoronoiDiagram::Cell*,Center*> center_map;
        std::map<VoronoiDiagram::Point*,Corner*> corner_map;

        AABB2 bb{Vector2{ -1.0f, -1.0f }, Vector2{ 1.0f, 1.0f }};
        VoronoiDiagram vd(points, bb);
        vd.compute(2);

        // First create centers for each cell
        const auto& cells = vd.get_cells();
        for (auto c : cells)
        {
            // Create a new center for cell's site
            auto center = create_center();
            center->pos = *c->site;
            // register new cell 
            center_map[c] = center;

            // Loop over edges and create corner for each vertex
            for (auto it = c->edges.begin(); it != c->edges.end(); ++it)
            {
                auto point = (*it)->v0;
                Corner* corner;
                if (corner_map.count(point) == 0)
                {
                    corner = create_corner();
                    corner->pos = *point;
                    corner->border = (point->x == bb.min.x || point->x == bb.max.x || point->y == bb.min.y || point->y == bb.max.y);
                    corner_map[point] = corner;
                }
                else
                {
                    corner = corner_map[point];
                }
                // establish corner <--> center relationships
                corner->touches.push_back(center);
                center->corners.push_back(corner);
            }
        }

        // Next, perform another pass to created edges between cells
        for (auto i = 0u; i < cells.size(); i++)
        {
            for (auto it = cells[i]->edges.begin(); it != cells[i]->edges.end(); ++it)
            {
                auto this_center = center_map[(*it)->cell];
                auto corner0 = corner_map[(*it)->v0];
                auto corner1 = corner_map[(*it)->v1];

                // edge belongs only to this cell
                if ((*it)->twin == nullptr)
                {
                    auto edge = create_edge();
                    edge->v0 = corner0;
                    edge->v1 = corner1;
                    edge->midpoint = (corner0->pos + corner1->pos) * 0.5f;
                    edge->d0 = this_center;
                    edge->d1 = nullptr;

                    this_center->borders.push_back(edge);
                    // Check for duplicates before adding corner
                    if (std::find(this_center->corners.begin(), this_center->corners.end(), corner0) == this_center->corners.end())
                    {
                        this_center->corners.push_back(corner0);
                        corner0->touches.push_back(this_center);
                    }
                    if (std::find(this_center->corners.begin(), this_center->corners.end(), corner1) == this_center->corners.end())
                    {
                        this_center->corners.push_back(corner1);
                        corner1->touches.push_back(this_center);
                    }

                    corner0->protrudes.push_back(edge);
                    corner1->protrudes.push_back(edge);
                    corner0->adjacent.push_back(corner1);
                    corner1->adjacent.push_back(corner0);

                }
                // edge is shared with another cell
                else
                {
                    // need to test if the other cell has been processed already - if so nothing to do
                    auto other_center = center_map[(*it)->twin->cell];
                    if (this_center->index < other_center->index)
                    {
                        auto edge = create_edge();
                        edge->v0 = corner0;
                        edge->v1 = corner1;
                        edge->midpoint = (corner0->pos + corner1->pos) * 0.5f;
                        edge->d0 = this_center;
                        edge->d1 = other_center;

                        this_center->neighbors.push_back(other_center);
                        other_center->neighbors.push_back(this_center);
                        
                        this_center->borders.push_back(edge);
                        other_center->borders.push_back(edge);
                        
                        // Check for duplicates before adding corner
                        if (std::find(this_center->corners.begin(), this_center->corners.end(), corner0) == this_center->corners.end())
                        {
                            this_center->corners.push_back(corner0);
                            corner0->touches.push_back(this_center);
                        }
                        if (std::find(this_center->corners.begin(), this_center->corners.end(), corner1) == this_center->corners.end())
                        {
                            this_center->corners.push_back(corner1);
                            corner1->touches.push_back(this_center);
                        }
                        if (std::find(other_center->corners.begin(), other_center->corners.end(), corner0) == other_center->corners.end())
                        {
                            other_center->corners.push_back(corner0);
                            corner0->touches.push_back(other_center);
                        }
                        if (std::find(other_center->corners.begin(), other_center->corners.end(), corner1) == other_center->corners.end())
                        {
                            other_center->corners.push_back(corner1);
                            corner1->touches.push_back(other_center);
                        }

                        corner0->protrudes.push_back(edge);
                        corner1->protrudes.push_back(edge);
                        corner0->adjacent.push_back(corner1);
                        corner1->adjacent.push_back(corner0);
                    }
                }
            }
        }
    }

    void MapGraph::generate(void)
    {
        //
        // Elevation stage
        //

        // Determine the elevations and water at Voronoi corners.
        assign_corner_elevation();
        // Determine polygon and corner type: ocean, coast, land.
        assign_ocean_coast_and_land();
        // Rescale elevations so that the highest is 1.0, and they're
        // distributed well. We want lower elevations to be more common
        // than higher elevations, in proportions approximately matching
        // concentric rings. That is, the lowest elevation is the
        // largest ring around the island, and therefore should more
        // land area than the highest elevation, which is the very
        // center of a perfectly circular island.
        redistribute_elevations();
        // Assign elevations to non-land corners
        for (auto& c : corners)
        {
            if (c->ocean || c->coast)
            {
                c->elevation = 0.0f;
            }
        }
        // Polygon elevations are the average of their corners
        assign_polygon_elevations();

        // 
        // Moisture stage
        //

        // Determine downslope paths.
        calculate_downslopes();
        // Determine watersheds: for every corner, where does it flow out into the ocean? 
        calculate_watersheds();
        // Create rivers.
        create_rivers();
        // Determine moisture at corners, starting at rivers and lakes, but not oceans. 
        // Then redistribute moisture to cover the entire range evenly from 0.0
        // to 1.0. Then assign polygon moisture as the average of the corner moisture.
        assign_corner_moisture();
        redistribute_moisture();
        assign_polygon_moisture();

        //
        // Biome stage
        //
        assign_biomes();
    }

    // Determine elevations and water at Voronoi corners. By construction, we have no local minima. 
    // This is important for the downslope vectors later, which are used in the river construction 
    // algorithm. Also by construction, inlets/bays push low elevation areas inland, which means 
    // many rivers end up flowing out through them. Also by construction, lakes often end up 
    // on river paths because they don't raise the elevation as much as other terrain does.
    void MapGraph::assign_corner_elevation(void)
    {
        IslandShape shape;

        std::queue<Corner*> queue;

        for (auto& q : corners)
        {
            // Determine map shape
            q->water = !shape.contains(q->pos);
            if (q->border)
            {
                q->elevation = 0.0f;
                queue.push(q.get());
            }
            else
            {
                q->elevation = big_number;
            }
        }

        // Traverse the graph and assign elevations to each point. As we
        // move away from the map border, increase the elevations. This
        // guarantees that rivers always have a way down to the coast by
        // going downhill (no local minima).
        while (!queue.empty()) 
        {
            auto q = queue.front();
            queue.pop();

            for (auto s : q->adjacent)
            {
                // Every step up is epsilon over water or 1 over land. The
                // number doesn't matter because we'll rescale the
                // elevations later.
                auto new_elev = 0.01f + q->elevation;
                if (!q->water && !s->water)
                {
                    new_elev += 1.0f;
                    // TODO: add more randomness                    
                }
                // If this point changed, we'll add it to the queue so
                // that we can process its neighbors too.
                if (new_elev < s->elevation) 
                {
                    s->elevation = new_elev;
                    queue.push(s);
                }
            }
        }
    }

    void MapGraph::assign_ocean_coast_and_land(void)
    {
        // Compute polygon attributes 'ocean' and 'water' based on the
        // corner attributes. Count the water corners per
        // polygon. Oceans are all polygons connected to the edge of the
        // map. In the first pass, mark the edges of the map as ocean;
        // in the second pass, mark any water-containing polygon
        // connected an ocean as ocean.
        std::queue<Center*> queue;

        const float lake_threshold = 0.3f; // 0 to 1, fraction of water corners for water polygon
        for (const auto& p : centers)
        {
            auto num_water = 0;
            for (auto q : p->corners)
            {
                if (q->border)
                {
                    p->border = true;
                    p->ocean = true;
                    q->water = true;
                    queue.push(p.get());
                }
                if (q->water)
                {
                    num_water++;
                }
            }
            p->water = (p->ocean || num_water >= p->corners.size() * lake_threshold);
        }

        while (!queue.empty())
        {
            auto p = queue.front();
            queue.pop();
            for (auto r : p->neighbors)
            {
                if (r->water && !r->ocean)
                {
                    r->ocean = true;
                    queue.push(r);
                }
            }
        }
      
        // Set the polygon attribute 'coast' based on its neighbors. If
        // it has at least one ocean and at least one land neighbor,
        // then this is a coastal polygon.
        for (const auto& p : centers)
        {
            auto num_ocean = 0;
            auto num_land = 0;
            for (auto r : p->neighbors)
            {
                if (r->ocean)
                    num_ocean++;
                if (!r->water)
                    num_land++;
            }
            p->coast = (num_ocean > 0) && (num_land > 0);
        }

        // Set the corner attributes based on the computed polygon
        // attributes. If all polygons connected to this corner are
        // ocean, then it's ocean; if all are land, then it's land;
        // otherwise it's coast.
        for (const auto& q : corners) 
        {
            auto num_ocean = 0;
            auto num_land = 0;
            for (auto p : q->touches)
            {
                if (p->ocean)
                    num_ocean++;
                if (!p->water)
                    num_land++;
            }
            q->ocean = (num_ocean == q->touches.size());
            q->coast = (num_ocean > 0) && (num_land > 0);
            q->water = q->border || ((num_land != q->touches.size()) && !q->coast);            
        }
    }

    void MapGraph::redistribute_elevations(void)
    {
        // SCALE_FACTOR increases the mountain area. At 1.0 the maximum
        // elevation barely shows up on the map, so we set it to 1.1.
        const auto scale_factor = 1.1f;

        auto locations = get_land_corners();
        std::sort(locations.begin(), locations.end(), [](Corner* a, Corner *b) -> bool {
            return a->elevation < b->elevation;
        });

        float x, y;
        for (auto i = 0u; i < locations.size(); i++)
        {
            // Let y(x) be the total area that we want at elevation <= x.
            // We want the higher elevations to occur less than lower
            // ones, and set the area to be y(x) = 1 - (1-x)^2.
            y = (float)i / (float)(locations.size() - 1);
            // Now we have to solve for x, given the known y.
            //  *  y = 1 - (1-x)^2
            //  *  y = 1 - (1 - 2x + x^2)
            //  *  y = 2x - x^2
            //  *  x^2 - 2x + y = 0
            // From this we can use the quadratic equation to get:
            x = std::sqrt(scale_factor) - std::sqrt(scale_factor * (1.0f - y));
            if (x > 1.0f) 
                x = 1.0f;
            locations[i]->elevation = x;
        }
    }

    void MapGraph::assign_polygon_elevations(void)
    {
        for (auto& p : centers)
        {
            auto sum = 0.0f;
            for (const auto& q : p->corners)
            {
                sum += q->elevation;
            }
            p->elevation = sum / (float)p->corners.size();
        }        
    }

    void MapGraph::calculate_downslopes(void)
    {
        for (auto& q : corners)
        {
            auto r = q.get();
            for (auto& s : q->adjacent)
            {
                if (s->elevation <= r->elevation)
                {
                    r = s;
                }
            }
            q->downslope = r;
        }
    }

    void MapGraph::calculate_watersheds(void)
    {
        // Initially the watershed pointer points downslope one step.      
        for (auto& q : corners) 
        {
            q->watershed = q.get();
            if (!q->ocean && !q->coast) 
            {
                q->watershed = q->downslope;
            }
        }
        // Follow the downslope pointers to the coast. Limit to 100
        // iterations although most of the time with numPoints==2000 it
        // only takes 20 iterations because most points are not far from
        // a coast.  TODO: can run faster by looking at
        // p.watershed.watershed instead of p.downslope.watershed.
        for (int i = 0; i < 100; i++) 
        {
            bool changed = false;
            for (auto& q : corners)
            {
                if (!q->ocean && !q->coast && !q->watershed->coast) 
                {
                    auto r = q->downslope->watershed;
                    if (!r->ocean)
                    {
                        q->watershed = r;
                        changed = true;
                    }
                }
            }
            if (!changed) 
                break;
        }
        // How big is each watershed?
        for (auto& q : corners) 
        {
            auto r = q->watershed;
            r->watershed_size = 1 + r->watershed_size;
        }
    }

    void MapGraph::create_rivers(void)
    {
        // TODO: size is width / height of rasterized image - this will
        // have to use something else, based on no# of points used?
        const auto size = 100;
        for (int i = 0; i < size / 2; i++)
        {
            auto q = corners[std::rand() % (corners.size() - 1)].get();
            if (q->ocean || q->elevation < 0.3f || q->elevation > 0.9f)
                continue;
            while (!q->coast)
            {
                if (q == q->downslope)
                {
                    break;
                }
                auto edge = lookup_edge_from_corner(q, q->downslope);
                edge->river = edge->river + 1;
                q->river = q->river + 1;
                q->downslope->river = q->downslope->river + 1; 
                q = q->downslope;
            }
        }
    }

    void MapGraph::assign_corner_moisture(void)
    {
        std::queue<Corner*> queue;
        // Fresh water
        for (auto& q : corners)
        {
            if ((q->water || q->river > 0) && !q->ocean)
            {
                q->moisture = q->river > 0 ? std::min(3.0f, 0.2f * (float)q->river) : 1.0f;
                queue.push(q.get());                
            }
            else
            {
                q->moisture = 0.0f;
            }
        }
        while (!queue.empty())
        {
            auto q = queue.front();
            queue.pop();
            for (auto r : q->adjacent)
            {
                auto new_moisture = q->moisture * 0.9f;
                if (new_moisture > r->moisture)
                {
                    r->moisture = new_moisture;
                    queue.push(r);
                }
            }
        }
        // Salt water
        for (auto& q : corners)
        {
            if (q->ocean || q->coast)
            {
                q->moisture = 1.0f;
            }
        }
    }

    void MapGraph::redistribute_moisture(void)
    {
        auto locations = get_land_corners();
        std::sort(locations.begin(), locations.end(), [](Corner* a, Corner *b) -> bool {
            return a->moisture < b->moisture;
        });
        for (auto i = 0u; i < locations.size(); i++)
        {
            locations[i]->moisture = (float)i / (float)(locations.size() - 1);
        }
    }

    void MapGraph::assign_polygon_moisture(void)
    {
        for (auto& p : centers)
        {
            auto sum = 0.0f;
            for (auto q : p->corners)
            {
                if (q->moisture > 1.0f)
                    q->moisture = 1.0f;
                sum += q->moisture;
            }
            p->moisture = sum / (float)p->corners.size();
        }
    }

    void MapGraph::assign_biomes(void)
    {
        for (auto& p : centers)
        {
            if (p->ocean)
            {
                p->biome = Ocean;
            } 
            else if (p->water)
            {
                if (p->elevation < 0.1f)
                    p->biome = Marsh;
                else if (p->elevation > 0.8f)
                    p->biome = Ice;
                else
                    p->biome = Lake;
            } 
            else if (p->coast)
            {
                p->biome = Beach;
            } 
            else if (p->elevation > 0.8f)
            {
                if (p->moisture > 0.5f)
                    p->biome = Snow;
                else if (p->moisture > 0.33f)
                    p->biome = Tundra;
                else if (p->moisture > 0.16f)
                    p->biome = Bare;
                else 
                    p->biome = Scorched;
            } 
            else if (p->elevation > 0.6f) 
            {
                if (p->moisture > 0.66f)
                    p->biome = Taiga;
                else if (p->moisture > 0.33f)
                    p->biome = Shrubland;
                else 
                    p->biome = TemperateDesert;
            } 
            else if (p->elevation > 0.3f) 
            {
                if (p->moisture > 0.83f)
                    p->biome = TemperateRainForest;
                else if (p->moisture > 0.50f) 
                    p->biome = TemperateDeciduousForest;
                else if (p->moisture > 0.16f) 
                    p->biome = Grassland;
                else 
                    p->biome = TemperateDesert;
            } 
            else 
            {
                if (p->moisture > 0.66f)
                    p->biome = TropicalRainForest;
                else if (p->moisture > 0.33f)
                    p->biome = TropicalSeasonalForest;
                else if (p->moisture > 0.16f)
                    p->biome = Grassland;
                else 
                    p->biome = SubtropicalDesert;
            }
        }
    }

    std::vector<MapGraph::Center*> MapGraph::get_centers(void) const
    {
        std::vector<Center*> res(centers.size());
        for (auto i = 0u; i < centers.size(); i++)
        {
            res[i] = centers[i].get();
        }
        return res;
    }

    std::vector<MapGraph::Edge*> MapGraph::get_edges(void) const
    {
        std::vector<Edge*> res(edges.size());
        for (auto i = 0u; i < edges.size(); i++)
        {
            res[i] = edges[i].get();
        }
        return res;
    }
}