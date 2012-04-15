#include "paths.h"
#include <cmath>   // for floor
#include <cfloat>  // for DBL_MAX
#include <omp.h>
#include <iostream>
#include "magical_config.h"
//#include <sched.h>   // for linux 'sched_getcpu()' function

/*
 * Auxiliary data structure: min-heap based priority queue
 */

typedef struct {
    double estimate;   // d in Dijkstra algorithm @Cormen
    unsigned long heap_pos;   // handle to current position in the heap
    Vertex *v;   // handle to get adjacencies, needed for edge relaxing in Dijkstra
    std::vector<unsigned long> path;   // key from vertices in the shortest path to v (including 'source' and 'v')
} heap_element;

typedef struct
{
public:
    /* test-only status function */
    void print_heap_array()
    {
        std::cout << "heap size: " << heap_size << std::endl << "vector size: " << heap.size()
             << " (capacity: " << heap.capacity() << ")" << std::endl;

        if (heap_size>0)
        {
            for (unsigned long i = 1; i<=heap_size; ++i)
                std::cout << "node " << i << ": vertex #" << heap[i]->v->get_key() << " (cost " << heap[i]->estimate << " heap_pos " << heap[i]->heap_pos << ")" << std::endl;
        }

        std::cout << std::endl;
    }

    /* constructs min-heap from an array */
    void build_min_heap(heap_element** array, unsigned long length)
    {
        heap_size = length;

        // fill heap (pointer-) vector
        heap.clear();
        heap.push_back(0);   // dummy head
        for (unsigned long k=0; k<length; ++k)
            heap.push_back(array[k]);

        // heap property
        for (unsigned long i = floor(length/2); i > 0; --i)
            min_heapify(i);
    }

    heap_element* min()
    {
        return heap[1];
    }

    heap_element* extract_min()
    {
        // when exporting a heap interface, signal error for heap underflow:
        if (heap_size < 1) return 0;

        heap_element *min = heap[1];

        // maintain heap property
        heap[1] = heap[heap_size];
        heap[1]->heap_pos = 1;
        heap[heap_size] = 0;
        --heap_size;
        min_heapify(1);

        min->heap_pos = 0;   // invalidate handle between heap and graph objects
        return min;
    }

    void insert(heap_element* e)
    {
        double new_key = e->estimate;
        ++heap_size;
        if (heap.size() <= heap_size)    // no empty node (considering dummy head)
            heap.push_back(0);

        // start at new leaf and move parent down until a smaller parent is found
        unsigned long i = heap_size;
        unsigned long parent = floor(i/2);
        while (i > 1 && heap[parent]->estimate > new_key)
        {
            heap[i] = heap[parent];
            heap[i]->heap_pos = i;
            i = parent;
            parent = floor(i/2);
        }

        heap[i] = e;
        heap[i]->heap_pos = i;
    }

    /* decreases key and return true, or return false case the new key is
     * greater than current one
     */
    bool decrease_key(unsigned long i, double new_key)
    {
        // when exporting a heap interface, signal error here
        if (heap[i]->estimate < new_key) return false;

        // start at current node and move toward the root until a smaller parent is found
        unsigned long parent = floor(i/2);
        while (i > 1 && heap[parent]->estimate > new_key)
        {
            // exchange parent <-> current node
            heap_element *tmp = heap[i];
            heap[i] = heap[parent];
            heap[i]->heap_pos = i;
            heap[parent] = tmp;
            heap[parent]->heap_pos = parent;

            i = parent;
            parent = floor(i/2);
        }

        heap[i]->estimate = new_key;
        return true;
    }

    unsigned long get_size()
    {
        return heap_size;
    }

private:
    /* ensure min-heap property (starting from element at 'root') */
    void min_heapify(unsigned long root)
    {
        unsigned long left = 2*root;  // left child
        unsigned long right = 2*root + 1;  // right child

        // determines the smallest among the root and its children
        unsigned long smallest = root;
        if (right > heap_size)
        {
            if (left > heap_size)
                return;
            else
                smallest = left;
        }
        else
        {
            if (heap[left]->estimate <= heap[right]->estimate)
                smallest = left;
            else
                smallest = right;
        }

        /* if heap property violated, reorder nodes and continue above */
        if (heap[root]->estimate > heap[smallest]->estimate)
        {
            // exchange root
            heap_element *tmp = heap[root];
            heap[root] = heap[smallest];
            heap[root]->heap_pos = root;
            heap[smallest] = tmp;
            heap[smallest]->heap_pos = smallest;

            min_heapify(smallest);
        }
    }

    unsigned long heap_size;
    std::vector<heap_element*> heap;

} binary_min_heap;

/*
 * Dijkstra's implementation
 */
void dijkstra(AdjacencyList<> *graph, unsigned long source, double dist[], std::vector<unsigned long> paths[])
{
    unsigned long num_vertices = graph->get_vertex_count();
    binary_min_heap queue;   // Q in Dijkstra algorithm presented in Cormen et al.

/*    //print graph
    std::cout << "Dijkstra running :: source " << source << std::endl;
    for (unsigned long u = 1; u<=num_vertices; ++u)
    {
        std::cout << std::endl << "Vertex #" << u << std::endl;

        Edge* it = graph->get_vertex(u)->get_adjacencies();
        while (it)
        {
            // current adjacency information
            unsigned long v = it->get_successor()->get_key();
            double w = it->get_weight();

            std::cout << "edge to " << v <<  " weight = " << w << std::endl;
            it = it->get_next();   // next edge
        }

        std::cout << std::endl;
    }
*/

    /* initialize_single_source: construct and insert in the heap an element
     * representing each vertex, including the shortest path estimate and a
     * handle to the vertex object
     */
    heap_element **entries = new heap_element*[num_vertices];
    for (unsigned long i = 1; i<=num_vertices; ++i)
    {
        heap_element *e = new heap_element();
        e->estimate = DBL_MAX;
        e->v = graph->get_vertex(i);
        e->path.clear();
        e->heap_pos = i;

        entries[i-1] = e;   // array starts at 0, while vertex index at 1
    }
    entries[source-1]->estimate = 0;
    entries[source-1]->path.push_back(source);
    queue.build_min_heap(entries, num_vertices);

    /* algorithm kernel: iteratively select closest vertex, and relax incident
     * edges (updating corresponding estimates for shortest paths)
     */
    while (queue.get_size() > 0)
    {
        heap_element *u = queue.extract_min();
        Edge* adj = u->v->get_adjacencies();

        while (adj)
        {
            // current adjacency information
            unsigned long adj_key = adj->get_successor()->get_key();
            heap_element *v = entries[adj_key-1];
            int w = adj->get_weight();

            // relax arc(u,v)
            if (v->estimate > u->estimate + w)
            {
                v->estimate = u->estimate + w;

                // saves path: antecessor path + goal vertex (v)
                v->path = u->path;
                v->path.push_back(v->v->get_key());

                // update heap
                queue.decrease_key(v->heap_pos, u->estimate + w);
            }

            adj = adj->get_next();   // next edge
        }
    }

    // store results and perform cleanup
    for (unsigned long i = 1; i<=num_vertices; ++i)
    {
        dist[i] = entries[i-1]->estimate;
        paths[i] = entries[i-1]->path;

        delete entries[i-1];
    }
    delete[] entries;
}

/*
 * Bellman-Ford's implementation
 */
bool bellman_ford(AdjacencyList<> *graph, unsigned long source, double dist[], std::vector<unsigned long> paths[])
{
    unsigned long num_vertices = graph->get_vertex_count();

    // initialize_single_source: shortest path estimate and paths for each vertex
    for (unsigned long i = 1; i<=num_vertices; ++i)
    {
        dist[i] = DBL_MAX;
        paths[i].clear();
    }
    dist[source] = 0;
    paths[source].push_back(source);

    // algorithm kernel: relax all arcs n-1 times
    for (unsigned long i = 1; i<num_vertices; ++i)
    {
        // iteration is complete if no arc is relaxed
        bool complete = true;
        
        // for each arc (u,v) in E
        for (unsigned long u = 1; u<=num_vertices; ++u)
        {
            Edge* it = graph->get_vertex(u)->get_adjacencies();
            while (it)
            {
                // current adjacency information
                unsigned long v = it->get_successor()->get_key();
                double w = it->get_weight();

                // relax arc(u,v)
                if (dist[u] + w < dist[v])
                {
                    dist[v] = dist[u] + w;

                    // saves path: antecessor path + goal vertex (v)
                    paths[v] = paths[u];
                    paths[v].push_back(v);
                    
                    // as long as any arc gets relaxed, iteration is not complete
                    complete = false;
                }

                it = it->get_next();   // next edge
            }
        }
        
        if (complete)
            return true;
    }

    /* check each arc (u,v) in E one more time: if there is any arc leading to a
     * shorter path after n-1 iterations, then exists a negative-weight cycle
     */
    for (unsigned long u = 1; u<=num_vertices; ++u)
    {
        Edge* it = graph->get_vertex(u)->get_adjacencies();
        while (it)
        {
            // current adjacency information
            unsigned long v = it->get_successor()->get_key();
            double w = it->get_weight();

            // is it possible to relax arc(u,v)?
            if (dist[u] + w < dist[v])
                return false;

            it = it->get_next();   // next edge
        }
    }

    return true;
}


/*
 * Johnson's implementation
 */
bool johnson(AdjacencyList<> *graph, double **dist, std::vector<unsigned long> **paths)
{
    // openmp setup
    if ( !magical_config::load_settings("johnson", graph->get_vertex_count()) )
    {
        std::cout << "Could not load settings from magical_config."
            << "Using default values." << endl;
        
        omp_set_num_threads(omp_get_num_procs());
    }
    
    /* "producing nonnegative weights (while preserving shortest paths) by
     * reweighting" (see Cormen et al. 2001): insert artificial vertex 's' in
     * the graph, and 0-weight edges from 's' to every other vertex
     */
    unsigned long old_num_vertices = graph->get_vertex_count();
    unsigned long new_num_vertices = old_num_vertices + 1;
    unsigned long s = new_num_vertices;
    graph->addVertices(1);
    for (unsigned long i = 1; i <= old_num_vertices; ++i)
        graph->addEdge(s, i, 0);

    /* let h[i] be the weight of the shortest path between vertices 's' and 'i',
     * determined through Bellman-Ford's algorithm
     */
    double *h = new double[new_num_vertices+1];
    std::vector<unsigned long> *bf_paths = new std::vector<unsigned long>[new_num_vertices+1];

    if (bellman_ford(graph, s, h, bf_paths) == false)
    {
        // negative-weight cycle detected
        delete[] h;
        delete[] bf_paths;
        return false;
    }
    else
    {
/*
        for (unsigned long i = 1; i<=old_num_vertices; ++i) {
            std::cout << h[i] << std::endl;
        }
        std::cout << std::endl;
*/
        // it's ok to remove 's' from the graph */
        for (unsigned long i = 1; i <= old_num_vertices; ++i)
            delete graph->removeEdge(s, i);
        graph->removeIfIsolatedVertex(s);

        delete[] bf_paths;   // paths used by bellman-ford are unnecessary

        // redefine each arc weight using 'h'
        for (unsigned long u = 1; u<=old_num_vertices; ++u)
        {
            Edge* it = graph->get_vertex(u)->get_adjacencies();
            while (it)
            {
                // current adjacency information
                unsigned long v = it->get_successor()->get_key();
                double w = it->get_weight();

                // new weight w' from arc (u,v): w + h[u] - h[v]
                it->set_weight(w + h[u] - h[v]);

                it = it->get_next();   // next edge
            }
        }

        /* computes shortest paths for each pair of vertices (all-pairs) by
         * calling Dijkstra's algorithm from each vertex in the original graph
         */
        #pragma omp parallel for default(none) shared(graph, old_num_vertices, dist, paths, h) schedule(static)
        for (long u = 1; u <= (signed) old_num_vertices; ++u)
        {
            double d[old_num_vertices+1];
            std::vector<unsigned long> p[old_num_vertices+1];

//            double *d = new double[old_num_vertices+1];
//            std::vector<unsigned long> *p = new std::vector<unsigned long>[old_num_vertices+1];

            dijkstra(graph, u, d, p);

//            delete[] d;
//            delete[] p;

//            std::cout << "thread #" << omp_get_thread_num() << " (core #" //<< sched_getcpu()
//                << ") finished dijkstra execution from vertex #" << u << std::endl << std::flush;

            // real path weight, using arc (u,v): w = w - h[u] + h[v]
            for (unsigned long v = 1; v<=old_num_vertices; ++v)
            {
                dist[u][v] = d[v] - h[u] + h[v];
                paths[u][v].assign( p[v].begin(), p[v].end() );
            }

            //#pragma omp critical
            //std::cout << paths[u][1][0] << std::endl;
        }

        // RESTORE original weight from arc (u,v): w = w - h[u] + h[v]
        for (unsigned long u = 1; u<=old_num_vertices; ++u)
        {
            Edge* it = graph->get_vertex(u)->get_adjacencies();
            while (it)
            {
                // current adjacency information
                unsigned long v = it->get_successor()->get_key();
                double w = it->get_weight();

                // new weight w' from arc (u,v): w - h[u] + h[v]
                it->set_weight(w - h[u] + h[v]);

                it = it->get_next();   // next edge
            }
        }

        delete[] h;

        return true;
    }

}
