#ifndef __TYPES_H__
#define __TYPES_H__

#include <vector>
#include <map>
#include <exception>
#include <cstdio>      // for sprintf
#include <utility>
#include <string>
#include <sstream>     // for stringstream

using namespace std;

// defined below
class Vertex;

/**
 * Edge: class implementing each node of the adjacency list. Can be extended
 * to store more information.
 */
class Edge
{
public:
    // constructor
    Edge(Vertex*, double, Edge*);

    // operations
    virtual Edge* get_next() const;

    // structure access (get/set)
    virtual bool has_next() const;
    virtual Vertex* get_successor() const;
    virtual double get_weight() const;
    virtual void set_weight(double);

private:
    Vertex *successor;
    Edge *link;
    double weight;

    friend class Vertex;
};


/**
 * Vertex: class implementing each vertex in the graph. Can be extended
 * to store more information.
 */
class Vertex
{
public:
    // constructors and destructor
    Vertex(unsigned long);
    virtual ~Vertex();

    // operations
    virtual void addEdge(Vertex*, double);
    virtual Edge* isEdge(Vertex*) const;
    virtual Edge* removeEdge(Vertex*);

    // structure access (get/set)
    virtual unsigned long get_key() const;
    virtual unsigned long get_indegree() const;
    virtual unsigned long get_outdegree() const;
    virtual Edge* get_adjacencies() const;

private:
    unsigned long key;
    unsigned long indegree, outdegree;
    Edge *adjacencies;
};


/**
 * NoSuchVertexException: default exception, thrown when attempting to use a
 * vertex which does not exist in the current graph.
 */
class NoSuchVertexException : public exception
{
public:
    NoSuchVertexException(unsigned long k) { vertex_key = k; }

    virtual const char* what() const throw() {
        char *buffer = new char[50];
        sprintf (buffer, "vertex #%lu does not exist", vertex_key);
        return buffer;
    }

private:
    unsigned long vertex_key;
};


/**
 * AdjacencyList: graph representation through an adjacency list. The template
 * parameters allow to use specific vertex and/or edge implementations, but is
 * set to use current implementation as default
 */
template <class V = Vertex, class E = Edge>
class AdjacencyList
{
public:
    // constructors and destructor
    AdjacencyList()
    {
        vertex_count = 0;
        vertices.push_back(0);  // dummy node
    }

    AdjacencyList(unsigned long num_vertices)
    {
        vertices.push_back(0);  // dummy node

        if (num_vertices<=0)
            vertex_count = 0;
        else
        {
            vertex_count = (unsigned long) num_vertices;
            for (unsigned long i=1; i<=vertex_count; ++i)
            {
                V *v = new V(i);
                vertices.push_back(v);
            }
        }
    }

    virtual ~AdjacencyList()
    {
        clearList();
        delete vertices[0];  // dummy node
    }

    virtual void clearList()
    {
        for (unsigned long i=1; i<=vertex_count; ++i)
            delete vertices[i];
    }

    // operations
    virtual void addVertices(unsigned long num_vertices)
    {
        if (num_vertices<=0)
            return;

        for (unsigned long i=1; i<=num_vertices; ++i)
        {
            V *v = new V(vertex_count+i);
            vertices.push_back(v);
        }

        vertex_count += num_vertices;
    }

    virtual void addEdge(unsigned long from, unsigned long to, double weight)
    throw (NoSuchVertexException)
    {
        if (from>vertex_count)
             throw NoSuchVertexException(from);

        if (to>vertex_count)
            throw NoSuchVertexException(to);

        vertices[from]->addEdge(vertices[to], weight);
    }

    /* returns pointer to edge, if it exists; otherwise, returns 0 */
    virtual E* isEdge(unsigned long from, unsigned long to) const
    throw (NoSuchVertexException)
    {
        if (from>vertex_count)
             throw NoSuchVertexException(from);

        if (to>vertex_count)
            throw NoSuchVertexException(to);

         return vertices[from]->isEdge(vertices[to]);
    }

    /* returns pointer to edge, if it exists; otherwise, returns 0 */
    virtual E* removeEdge(unsigned long from, unsigned long to)
    throw (NoSuchVertexException)
    {
        if (from>vertex_count)
             throw NoSuchVertexException(from);

        if (to>vertex_count)
            throw NoSuchVertexException(to);

         return vertices[from]->removeEdge(vertices[to]);
    }

    /* does NOT traverse graph checking for arcs to the specified vertex
     * if vertex exists: returns true if it is isolated; otherwise, returns false
     */
    virtual bool removeIfIsolatedVertex(unsigned long key)
    throw (NoSuchVertexException)
    {
        if (key>vertex_count)
            throw NoSuchVertexException(key);

        if (vertices[key]->get_outdegree() > 0 || vertices[key]->get_indegree() > 0)
            return false;

        // remove vertex: delete object, erase vector position and adjust counter
        delete vertices[key];
        vertices.erase(vertices.begin()+key);
        --vertex_count;
        return true;
    }

    /* removes vertex and traverse graph removing arcs to it */
    virtual void removeVertex(unsigned long key)
    throw (NoSuchVertexException)
    {
        if (key>vertex_count)
            throw NoSuchVertexException(key);

        // remove arcs from all vertices to the specified one
        V *v = vertices[key];
        for (unsigned long u = 1; u<=vertex_count; ++u)
            vertices[u]->removeEdge(v);

        // remove vertex: delete object, erase vector position and adjust counter
        delete vertices[key];
        vertices.erase(vertices.begin()+key);
        --vertex_count;
    }

    // structure access (get/set)
    virtual unsigned long get_vertex_count() const
    {
        return vertex_count;
    }

    virtual V* get_vertex(unsigned long v) const
    throw (NoSuchVertexException)
    {
        if (v > vertex_count)
             throw NoSuchVertexException(v);

        return vertices[v];
    }

protected:
    vector<V*> vertices;
    unsigned long vertex_count;
};


/**
 * uAdjacencyList: extends library AdjacencyList to include functionality
 * regarding user provided types of vertices and edges
 */
template <class V, class E>
class uAdjacencyList : public AdjacencyList<>
{
public:
    // constructors and destructor
    uAdjacencyList()
    : AdjacencyList<>() { }

    uAdjacencyList(unsigned long num_vertices)
    : AdjacencyList<>(num_vertices) { }

    // each of the base class methods are overloaded properly
    void clearList()
    {
        AdjacencyList<>::clearList();
        uvertices.clear();
        uedges.clear();
    }

    Edge* removeEdge(unsigned long from, unsigned long to)
    throw (NoSuchVertexException)
    {
        string key = concat_keys(from, to);
        uedges.erase(key);
        
        return AdjacencyList<>::removeEdge(from, to);
    }

    bool removeIfIsolatedVertex(unsigned long key)
    throw (NoSuchVertexException)
    {
        if (AdjacencyList<>::removeIfIsolatedVertex(key))
        {
            uvertices.erase(key);
            return true;
        }
        else
            return false;
    }

    // redefined to avoid traversing the graph twice
    void removeVertex(unsigned long key)
    throw (NoSuchVertexException)
    {
        if (key>vertex_count)
            throw NoSuchVertexException(key);

        // remove arcs from all vertices and the specified one
        for (unsigned long u = 1; u<=vertex_count; ++u)
        {
            removeEdge(u,key);
            removeEdge(key,u);
        }

        // remove vertex: delete object, erase vector position and adjust counter
        uvertices.erase(key);
        delete vertices[key];
        vertices.erase(vertices.begin()+key);
        --vertex_count;
    }
    
    // new set/get methods for the user-specific objects:

    virtual void set_uvertex(unsigned long index, V *obj)
    {
        uvertices[index] = obj;
    }

    virtual V* get_uvertex(unsigned long index)
    {
        // if key was found, return the mapped value; return 0 otherwise
        if (uvertices.find(index) != uvertices.end())
            return (uvertices.find(index))->second;
        else
            return 0;
    }

    virtual void set_uedge(unsigned long from, unsigned long to, E *obj)
    {
        string key = concat_keys(from, to);
        uedges[key] = obj;
    }

    virtual E* get_uedge(unsigned long from, unsigned long to)
    {
        string key = concat_keys(from, to);

        // if key was found, return the mapped value; return 0 otherwise
        if (uedges.find(key) != uedges.end())
            return (uedges.find(key))->second;
        else
            return 0;
    }

private:
    string concat_keys(unsigned long from, unsigned long to)
    {
        // map key (concatening both keys)
        string val("");
        stringstream ss;

        ss << from;
        ss << ':';
        ss << to;

        return ss.str();
    }
    
    map<unsigned long, V*> uvertices;
    map<string, E*> uedges;
};


/**
 * adapter: extends the library API by providing means to consctruct instances
 * of AdjacencyList (or its subclass: uAdjacencyList) representing a graph which
 * is described as an adjacency list of user-specific types.
 */
template <class V, class E>
uAdjacencyList<V, E>* adapter(vector< pair<V*,E*> > *adjacencies, unsigned long num_vertices)
{
    // creates graph corresponding to the given adjacencies
    uAdjacencyList<V, E> *graph = new uAdjacencyList<V, E>(num_vertices);

    // avalia adjacencias de cada vertice do arranjo fornecido
    for (unsigned long i = 0; i<num_vertices; ++i)
    {
        vector< pair<V*,E*> > edges = adjacencies[i];
        unsigned long degree = edges.size();

        // adiciona arestas do vertice atual
        for (unsigned long j = 0; j<degree; ++j)
        {
            V *terminus = edges[j].first;
            E *edge = edges[j].second;
            graph->addEdge(i+1, terminus->get_key()+1, edge->get_weight());
            graph->set_uedge(i+1, terminus->get_key()+1, edge);
        }
    }

    return graph;

}

template <class V, class E>
AdjacencyList<>* adapter(vector< pair<long,double> > *adjacencies, unsigned long num_vertices)
{
    // creates graph corresponding to the given adjacencies
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    // avalia adjacencias de cada vertice do arranjo fornecido
    for (unsigned long i = 0; i<num_vertices; ++i)
    {
        vector< pair<long,double> > edges = adjacencies[i];
        unsigned long degree = edges.size();

        // adiciona arestas do vertice atual
        for (unsigned long j = 0; j<degree; ++j)
        {
            long terminus = edges[j].first;
            double weight = edges[j].second;
            graph->addEdge(i+1, terminus+1, weight);
        }
    }

    return graph;
}

#endif /* __TYPES_H__ */
