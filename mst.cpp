#include "mst.h"
#include <omp.h>
#include <vector>
#include <queue>
#include <utility>
#include <map>
#include <cfloat>     // for DBL_MAX
#include <iostream>   // for cerr
#include "magical_config.h"

#define ulong unsigned long

using namespace std;

// private tree wrapper for the boruvka minimum spanning tree algorithm
class forest_tree
{
private:
    // class constructor and destructor
    forest_tree(ulong id)
    {
        tree_vertices.clear();
        tree_id = id;
    }
    
    ~forest_tree()
    {
        tree_vertices.clear();
    }
    
    // vertices in this tree
    vector<ulong> tree_vertices;
    
    pair< ulong , pair<ulong,double> > last_edge;
    ulong tree_id;
    
    friend bool boruvka(AdjacencyList<>*, AdjacencyList<>*);
};


bool boruvka(AdjacencyList<>* g, AdjacencyList<>* final_mst)
{
    long num_vertices = g->get_vertex_count();
    
    // openmp setup
    // TO-DO: would be better to use num_edges
    if ( !magical_config::load_settings("boruvka", num_vertices) )
    {
        std::cout << "Could not load settings from magical_config."
            << "Using default values." << endl;
        
        omp_set_num_threads(omp_get_num_procs());
    }
    
    // edges in the final mst: (u, (v,w))
    map< ulong, map<ulong,double> > mst;
    
    // list of trees which are grown and merged in order to build the MST of g
    vector<forest_tree*> forest;
    forest.reserve(num_vertices);
    
    // handle to the tree currently containing each vertex
    map< ulong, forest_tree* > vertex2tree;
    
    // initialize trees
    #pragma omp parallel for default(none) shared(g, forest, vertex2tree, num_vertices) schedule(static)
    for (long i=1; i<=num_vertices; ++i)
    {
        forest_tree *tree = new forest_tree(i);
        tree->tree_vertices.push_back(i);
                
        #pragma omp critical
        {
            forest.push_back(tree);
            vertex2tree[i] = tree;
        }
    }
    
    // repeat while not connected (not a MST)
    while (forest.size() > 1)
    {
        ulong forest_size = forest.size();
        int return_error = 0;

        // STEP 1: for each tree, select the cheapest edge leaving it
        #pragma omp parallel for default(none) shared(g, forest, forest_size, vertex2tree, return_error) schedule(static)
        for (long i=0; i < (signed) forest_size; ++i)
        {
            // test if the graph is connected
            bool isolated_tree = true;
            
            forest_tree *tree = forest[i];
            ulong tree_id = tree->tree_id;
            
            /* cheapest feasible "heap top", i.e. edge (u,v,w) such that
             * u is in the i-th tree, v not in the i-th tree, and w is minimum
             */
            pair<double, pair<ulong,ulong> > cheapest = make_pair(DBL_MAX, make_pair(0,0));
            
            for (ulong j=0; j<tree->tree_vertices.size(); ++j)
            {
                long u = tree->tree_vertices[j];
                Edge* it = g->get_vertex(u)->get_adjacencies();
                while(it)
                {
                    long v = it->get_successor()->get_key();
                    double w = it->get_weight();
                    
                    // is edge cheaper and feasible (i.e. v is in another tree)?
                    if (w<cheapest.first && vertex2tree[v]->tree_id != tree_id)
                    {
                        cheapest = make_pair(w, make_pair(u,v));
                        isolated_tree = false;
                    }
                    
                    it = it->get_next();
                }
            }
            
            /* if no edge was available (while there is more than 1 tree) the
             * graph is not connected.
             */
            if (isolated_tree)
            {
                #pragma omp atomic
                ++return_error;
            }
            
            // add selected edge (u,v,w) to mst, and save last_edge
            double w = cheapest.first;
            ulong u = cheapest.second.first;
            ulong v = cheapest.second.second;
            
            tree->last_edge = make_pair(u, make_pair(v,w));
        }
        
        if (return_error != 0)
        {
            cerr << "[magical] graph given to boruvka's algorith is not connected." << endl;
            return false;
        }
        
        // STEP 2: merge trees connected by the selected edges
        
        bool *merged = new bool[num_vertices+1];
        for (long i=0; i<=num_vertices; ++i)
            merged[i] = false;
        
        // traverse forest merging trees
        for (ulong i=0; i<forest_size; ++i)
        {
            forest_tree *start_tree = forest[i];
            forest_tree *goal_tree = vertex2tree[start_tree->last_edge.second.first];
            
            while ( !merged[start_tree->tree_id] &&
                    !merged[goal_tree->tree_id] &&
                    goal_tree->tree_id != start_tree->tree_id)
            {
                merged[goal_tree->tree_id] = true;
                
                #pragma omp parallel sections default(none) shared(start_tree, goal_tree, mst, vertex2tree)
                {
                    #pragma omp section
                    {
                        // save mst edge
                        ulong u = (start_tree->last_edge).first;
                        ulong v = (start_tree->last_edge.second).first;
                        double w = (start_tree->last_edge.second).second;
                        
                        if (u<v)
                            (mst[u])[v] = w;
                        else
                            (mst[v])[u] = w;
                    }
                    
                    #pragma omp section
                    {
                        // move vertices from goal into start tree
                        vector<ulong>::iterator it;
                        for (it = goal_tree->tree_vertices.begin();
                            it != goal_tree->tree_vertices.end(); ++it)
                        {
                            long vertex = (*it);
                            start_tree->tree_vertices.push_back(vertex);

                            // update vertex2tree index
                            vertex2tree[vertex] = start_tree;
                        }
                    }
                    
                    #pragma omp section
                    {
                        // copy last_edge from goal into start tree
                        ulong u = (goal_tree->last_edge).first;
                        ulong v = (goal_tree->last_edge.second).first;
                        ulong w = (goal_tree->last_edge.second).second;
                        start_tree->last_edge = make_pair(u, make_pair(v,w));
                    }
                    
                }
                
                // continue traversal with goal tree
                goal_tree = vertex2tree[(goal_tree->last_edge.second).first];
            }
            
        }
        
        // remove original trees which had data copied into others during merge
        vector<forest_tree*>::iterator it = forest.begin();
        while (it < forest.end())
        {
            if (merged[(*it)->tree_id])
            {
                delete (*it);
                it = forest.erase(it);
            }
            else
                ++it;
        }
        
        delete[] merged;
        
    } // new iteration of steps 1 (find cheapest edges) and 2 (merge trees)
    
    // generate AdjacencyList<> instance corresponding to the built MST
    map< ulong, map<ulong,double> >::iterator it_u;
    for (it_u=mst.begin(); it_u!=mst.end(); ++it_u)
    {
        //ulong adj = (*it).second.size();
        
        ulong u = (*it_u).first;
        
        map<ulong,double>::iterator it_v;
        for (it_v=(*it_u).second.begin(); it_v!=(*it_u).second.end(); ++it_v)
        {
            ulong v = (*it_v).first;
            double w = (*it_v).second;
            
            final_mst->addEdge(u,v,w);
            final_mst->addEdge(v,u,w);
        }
    }
    
    // clean-up
    for (ulong i=0; i<forest.size(); ++i)
        delete forest[i];
    
    forest.clear();
    mst.clear();
    vertex2tree.clear();
    
    return true;
}
