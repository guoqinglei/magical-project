#include "types.h"

/*
 * Edge implementation
 */

Edge::Edge(Vertex *v, double w, Edge *e)
{
    successor = v;
    weight = w;
    link = e;
}

Edge* Edge::get_next() const { return link; }

bool Edge::has_next() const { return (link == 0); }

Vertex* Edge::get_successor() const { return successor; }

Vertex* Edge::get_origin() const { return origin; }

double Edge::get_weight() const { return weight; }

void Edge::set_weight(double w) { weight = w; }

/*
 * Vertex implementation
 */

Vertex::Vertex(unsigned long k)
{
    key = k;
    indegree = outdegree = 0;
    adjacencies = 0;
}


Vertex::~Vertex()
{
    Edge *e = adjacencies;
    while (e)
    {
        adjacencies = e;
        e = adjacencies->get_next();
        delete adjacencies;
    }
}

void Vertex::addEdge(Vertex *v, double w)
{
    // new edge is inserted as the head of the list
    Edge *e = new Edge(v, w, adjacencies);
    adjacencies = e;
    outdegree++;
    v->indegree++;
    e->origin = this;
}

Edge* Vertex::isEdge(Vertex *v) const
{
    Edge *e = adjacencies;
    while (e)
    {
        if (e->get_successor() == v)
            return e;
        else
            e = e->get_next();
    }

    return 0;
}

Edge* Vertex::removeEdge(Vertex *v)
{
    Edge *e = adjacencies;
    Edge *previous = e;

    // first edge is the one we're looking for
    if (e && e->get_successor() == v)
    {
        // remove from adjacency list (friendship allows) and adjusts degrees
        adjacencies = e->get_next();
        outdegree--;
        e->get_successor()->indegree--;
        return e;
    }

    while (e)
    {
        if (e->get_successor() == v)
        {
            // remove from adjacency list (friendship allows) and adjusts degrees
            previous->link = e->get_next();
            outdegree--;
            e->get_successor()->indegree--;

            return e;
        }
        else
        {
            previous = e;
            e = e->get_next();
        }
    }

    return 0;   // edge doesn't exist
}

unsigned long Vertex::get_key() const { return key; }

unsigned long Vertex::get_indegree() const { return indegree; }

unsigned long Vertex::get_outdegree() const { return outdegree; }

Edge* Vertex::get_adjacencies() const { return adjacencies; }
