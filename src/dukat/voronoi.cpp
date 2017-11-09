#include "stdafx.h"
#include "voronoi.h"
#include "log.h"
#include "mathutil.h"
#include "ray2.h"

namespace dukat
{
    VoronoiDiagram::VoronoiDiagram(const std::vector<Vector2>& sites, const AABB2& bb) : sites(sites), bb(bb),
        max_scale_extent((float)std::numeric_limits<uint16_t>::max())
    {
        scale_factor = bb.max - bb.min;
        inv_scale_factor = Vector2{1.0f / scale_factor.x, 1.0f / scale_factor.y};
    }

    void VoronoiDiagram::reset(void)
    {
        border_vertices.clear();
        cells.clear();
        vertices.clear();
    }

    void VoronoiDiagram::compute(int iterations)
    {
        for (int iteration = 0; iteration < iterations; iteration++)
        {
            if (!cells.empty())
            {
                // perform lloyd relaxation on input sites
                std::vector<Vector2> new_sites;
                new_sites.reserve(cells.size());
                for (const auto& it : cells)
                {
                    // compute midpoint of cell
                    Vector2 mid;
                    for (const auto& edge : it.second->edges)
                    {
                        mid += *edge->v0;
                    }
                    mid /= (float)it.second->edges.size();
                    new_sites.push_back(mid);
                }
                this->sites = new_sites;
            }

            reset();

            // seed border list
            vertices.push_back(std::make_unique<Point>(bb.min));
            vertices.push_back(std::make_unique<Point>(bb.max.x, bb.min.y));
            vertices.push_back(std::make_unique<Point>(bb.max));
            vertices.push_back(std::make_unique<Point>(bb.min.x, bb.max.y));
            for (auto it = vertices.begin(); it != vertices.end(); ++it)
                add_border_vertex(it->get());

            // Need to scale points and convert to int
            const auto num_sites = this->sites.size();
            std::vector<boost::polygon::point_data<int>> in_sites(num_sites);
            for (auto i = 0u; i < num_sites; i++)
            {
                // normalize input values [0..1]
                auto tx = (this->sites[i].x - bb.min.x) * inv_scale_factor.x;
                auto ty = (this->sites[i].y - bb.min.y) * inv_scale_factor.y;
                // scale to [0..max_val]
                in_sites[i] = boost::polygon::point_data<int>{ 
                    (int)std::round(tx * max_scale_extent), (int)std::round(ty * max_scale_extent) };
            }

            // Generate diagram
            vd = std::make_unique<voronoi_diagram>();
            construct_voronoi(in_sites.begin(), in_sites.end(), vd.get());
            collect_cells();
            // at this point we can free up memory used by boost diagram
            vd = nullptr;

            // Finish up cells and identify neighbors
            close_cells();
        }

        // logger << "Diagram contains " << cells.size() << " cells." << std::endl;
    }

    void VoronoiDiagram::collect_cells(void)
    {
        // Build up cells & edges
        for (auto it = vd->cells().begin(); it != vd->cells().end(); ++it)
        {
            auto cell = std::make_unique<Cell>();
            cell->id = it->source_index();
            cell->site = &sites[cell->id];
			auto c_ptr = cell.get();
			// Add cell to cell map here, since it will be needed during edge clipping
			cells[cell->id] = std::move(cell);

            // Iterate over cell's edges
            auto edge = it->incident_edge();
            do 
            {
                if (edge->is_curved())
                {
                    logger << "Warning - found curved edge." << std::endl;
                }
                else if (!edge->is_primary())
                {
                    logger << "Skipping secondary edge." << std::endl;
                }
                else
                {
                    auto new_edge = clip_edge(*edge);
                    if (new_edge != nullptr)
                    {
                        // insert new edge
                        new_edge->cell = c_ptr;
						c_ptr->edges.push_back(std::move(new_edge));
                    }
                }
                edge = edge->next();
            }
            while (edge != it->incident_edge());
        }
    }

    void VoronoiDiagram::insert_link(Point* from, Point* to, Cell* cell, std::list<std::unique_ptr<Edge>>::iterator& it)
    {
        //logger << "Insert link from " << *from << " to " << *to << std::endl;
        auto last_v = from;
        Point* next_v;
        do
        {
            next_v = border_walk(last_v);
            // quit if no next point can be found or we're in a loop
            if (next_v == nullptr || next_v == from)
            {
                throw std::runtime_error("Error closing cell.");
            }
            auto edge = std::make_unique<Edge>();
            edge->v0 = last_v;
            edge->v1 = next_v;
            edge->cell = cell;
            cell->edges.insert(it, std::move(edge));
            last_v = next_v;
        }
        while (next_v != to);
    }

    void VoronoiDiagram::close_cells(void)
    {
        for (auto cit = cells.begin(); cit != cells.end(); ++cit)
        {
            auto cell = cit->second.get();

            // Check that edges form a loop
            Edge* start = nullptr;
            Edge* prev = nullptr;
            Edge* cur = nullptr;
            auto eit = cell->edges.begin();
            for ( ; eit != cell->edges.end(); ++eit)
            {
                cur = eit->get();
                if (prev == nullptr) // 1st edge
                {
                    start = prev = cur;
                }
                else if (prev->v1 == cur->v0) // last edge is connected to this one
                {
                    prev = cur; 
                }
                else // missing connection 
                {
                    insert_link(prev->v1, cur->v0, cell, eit);
                    prev = cur;
                }
            }

            if (cur == nullptr)
            {
                // something went wrong - a cell without edges?
                throw std::runtime_error("Error - cell without edges.");
            }

            // check that cell loops back to the beginning
            if (cur->v1 != start->v0)
            {
                insert_link(cur->v1, start->v0, cell, eit);
            }
        }
    }

    int VoronoiDiagram::check_side(Point* point) const
    {
        if (point->y == bb.min.y)
            return 0; // bottom
        else if (point->x == bb.max.x)
            return 1; // left
        else if (point->y == bb.max.y)
            return 2; // top
        else
            return 3; // right
    }

    void VoronoiDiagram::add_border_vertex(Point* point)
    {
        // logger << "Add border vertex " << *point << std::endl;
        auto side = check_side(point);
        auto it = border_vertices.begin();
        for (; it != border_vertices.end(); ++it)
        {
            // safe guard against point already on list
            if (point == *it)
                return;

            auto side2 = check_side(*it);
            if (side > side2)
            {
                // skip vertices until we find the another
                // vertex on the same border side.
                continue;
            }
            else if (side == side2)
            {
                if (side == 0 && point->x <= (*it)->x)
                    break;
                else if (side == 1 && point->y <= (*it)->y)
                    break;
                else if (side == 2 && point->x >= (*it)->x)
                    break;
                else if (side == 3 && point->y >= (*it)->y)
                    break;
            }
            else
            {
                // we've moved beyond other vertices on our side
                // so stop and insert here.
                break;
            }
        }
        border_vertices.insert(it, point);
    }

    VoronoiDiagram::Point* VoronoiDiagram::border_walk(Point* start)
    {
        auto it = std::find(border_vertices.begin(), border_vertices.end(), start);
        if (it == border_vertices.end())
        {
            return nullptr;
        }

        ++it; // move to next point

        // we've hit the end of the list, move to beginning
        if (it == border_vertices.end())
        {
            it = border_vertices.begin();
        }
        return (*it != start) ? *it : nullptr;
    }

    VoronoiDiagram::Point* VoronoiDiagram::create_vertex(const Point& p)
    {
        auto it = vertices.begin();
        for ( ; it != vertices.end(); ++it)
        {
            if (p.x < (*it)->x && p.y < (*it)->y)
                break;
            else if (p.x == (*it)->x && p.y == (*it)->y)
                return it->get();
        }
        //logger << "Adding vertex " << p << std::endl;
        auto point = std::make_unique<Point>(p);
        auto ptr = point.get();
        vertices.insert(it, std::move(point));
        return ptr;
    }

    bool VoronoiDiagram::clip_vector(const Vector2& inv, const Vector2& dir, Vector2& outv)
    {
        Ray2 ray(inv, dir);
        auto t = bb.intersect_ray(ray, 0.0f, 1000.0f);
        if (t == no_intersection)
        {
            return false;
        }
        else
        {
            outv = inv + dir * t;
            const float min_delta = 0.0001f;
            if (std::abs(outv.x - bb.min.x) < min_delta)
                outv.x = bb.min.x;
            if (std::abs(outv.y - bb.min.y) < min_delta)
                outv.y = bb.min.y;
            if (std::abs(outv.x - bb.max.x) < min_delta)
                outv.x = bb.max.x;
            if (std::abs(outv.y - bb.max.y) < min_delta)
                outv.y = bb.max.y;
            return true;
        }
    }

    std::unique_ptr<VoronoiDiagram::Edge> VoronoiDiagram::clip_edge(const voronoi_diagram::edge_type& src_edge)
    {
        Point tmpv0, tmpv1;
        // Flags to keep track of clipped vertices
        bool is_clipped0 = false, is_clipped1 = false;

        auto vs0 = src_edge.vertex0();
        if (vs0 != nullptr)
        {
            tmpv0 = bb.min + Vector2{ ((float)vs0->x() / max_scale_extent) * scale_factor.x, 
                ((float)vs0->y() / max_scale_extent) * scale_factor.y };
        }
        auto vs1 = src_edge.vertex1();
        if (vs1 != nullptr)
        {
            tmpv1 = bb.min + Vector2{ ((float)vs1->x() / max_scale_extent) * scale_factor.x, 
                ((float)vs1->y() / max_scale_extent) * scale_factor.y };
        }

        // Get cells on each side of the edge
        auto cell1 = src_edge.cell();
        auto cell2 = src_edge.twin()->cell();

        // One of the vertices is not defined - need to clip against the bounding box
        Vector2 dir;   
        if (src_edge.is_infinite())
        {
            // Case #1: Edge is between two cells containing point sites 
            // The edge in this case is perpendicular to the vector connecting the
            // two point sites.
            if (cell1->contains_point() && cell2->contains_point())
            {
                // Look up point sites
                const auto& p1 = sites[cell1->source_index()];
                const auto& p2 = sites[cell2->source_index()];
                // Compute edge direction (in original space)
                dir.x = p2.y - p1.y; 
                dir.y = p1.x - p2.x;
            }
            // Case #2: Edge is between segment and point sites
            // Only needed if we support segments as input, thus just give up
            // See http://www.boost.org/doc/libs/1_58_0/libs/polygon/example/voronoi_visualizer.cpp 
            // for a working solution.
            else
            {
                throw std::runtime_error("Unsupported edge type.");
            }

            // Based on end point and dir, find start point
            if (vs0 == nullptr)
            {
                if (!clip_vector(tmpv1, -dir, tmpv0))
                    return nullptr;
                is_clipped0 = true;
            }

            // Based on start point and -dir, find end point
            if (vs1 == nullptr)
            {
                if (!clip_vector(tmpv0, dir, tmpv1))
                    return nullptr;
                is_clipped1 = true;
            }
        }

        // At this point, we have both vertices of the edge. In case of vertices 
        // that we got directly from the source edge, we need to check if they fall 
        // outside of the bounding box.
        if (!is_clipped0 && !bb.contains(tmpv0))
        {
            if (!clip_vector(tmpv0, tmpv1 - tmpv0, tmpv0))
                return nullptr;
            is_clipped0 = true;
        }
        if (!is_clipped1 && !bb.contains(tmpv1))
        {
            if (!clip_vector(tmpv1, tmpv0 - tmpv1, tmpv1))
                return nullptr;
            is_clipped1 = true;
        }

        // Clipping can lead to invalid edges - just skip them
        if (tmpv0 == tmpv1)
        {
            return nullptr;
        }

        // Add vertices to internal list
        auto res = std::make_unique<Edge>();
        res->v0 = create_vertex(tmpv0);
        res->v1 = create_vertex(tmpv1);
        // Track clipped vertices
        if (is_clipped0)
            add_border_vertex(res->v0);
        if (is_clipped1)
            add_border_vertex(res->v1);

        // Establish neighbor relationship. If other cell is null,
        // assume that this relationship will get created when it is
        // processed.
		auto this_idx = cell1->source_index();
		auto other_idx = cell2->source_index();
		auto this_cell = cells[this_idx].get();
		auto other_cell = cells[other_idx].get();
        if (other_cell != nullptr)
        {
            this_cell->neighbors.insert(other_cell);
            other_cell->neighbors.insert(this_cell);
            //logger << "Cell " << *this_cell->site << " is next to " << *other_cell->site << std::endl;

            // Find this edge's twin
            auto& edges = other_cell->edges;
            auto twin_it = std::find_if(edges.begin(), edges.end(), 
                [&res](const std::unique_ptr<Edge>& e) { return e->v0 == res->v1 && e->v1 == res->v0; } );
            if (twin_it != edges.end())
            {
                (*twin_it)->twin = res.get();
                res->twin = twin_it->get();
                // logger << "Edge " << *res->v0 << "-" << *res->v1 << " links to " << *(*twin_it)->v0 << "-" << *(*twin_it)->v1 << std::endl;
            }
        }

        return std::move(res);
    }

	std::vector<VoronoiDiagram::Cell*> VoronoiDiagram::get_cells(void) const
	{
		std::vector<Cell*> res;
		res.reserve(cells.size());
		for (auto& it : cells) 
		{
			res.push_back(it.second.get());
		}
		return res;
	}
}