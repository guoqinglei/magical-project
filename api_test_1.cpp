#include <iostream>
#include <vector>
#include <utility>
#include "types.h"
#include "paths.h"

#define NUM_VERTICES 5

using namespace std;

typedef struct adj_list
{
    adj_list(int n)
    {
        neighbors = new vector<int>[n];
        weights = new vector<double>[n];
    }
    ~adj_list()
    {
        delete[] neighbors;
        delete[] weights;
    }

    vector<int> *neighbors;    // listas de vertices alcancaveis
    vector<double> *weights;   // respectivas listas de pesos

} adj_list;

vector< pair<long,double> >* get_adjacencies(adj_list *g)
{
    vector< pair<long,double> > *h = new vector< pair<long,double> >[NUM_VERTICES];

    // check each node in the original graph
    for (int i=0; i<NUM_VERTICES; ++i)
    {
        int degree = g->neighbors[i].size();

        // insert each of this node's adjacencies in the resulting vector
        for (int j=0; j<degree; ++j)
        {
            long vertex = g->neighbors[i][j];
            double weight = g->weights[i][j];
            
            pair<long, double> arc = make_pair(vertex, weight);
            h[i].push_back(arc);
        }
    }

    return h;
}


int main()
{
    adj_list *original = new adj_list(NUM_VERTICES);

    // arcos
    original->neighbors[0].push_back(1);
    original->neighbors[0].push_back(2);
    original->weights[0].push_back(4);
    original->weights[0].push_back(1);

    original->neighbors[1].push_back(2);
    original->neighbors[1].push_back(3);
    original->neighbors[1].push_back(4);
    original->weights[1].push_back(3);
    original->weights[1].push_back(3);
    original->weights[1].push_back(1);

    original->neighbors[2].push_back(3);
    original->neighbors[2].push_back(4);
    original->weights[2].push_back(1);
    original->weights[2].push_back(18);

    original->neighbors[3].push_back(1);
    original->weights[3].push_back(1);

    original->neighbors[4].push_back(1);
    original->neighbors[4].push_back(3);
    original->weights[4].push_back(0);
    original->weights[4].push_back(1);

    /*
     * interfacing with the library API
     */
    vector< pair<long,double> > *adj = get_adjacencies(original);
    AdjacencyList<> *adaptee = adapter<long,double>(adj, NUM_VERTICES);

    /*
     * testing by means of dijkstra's sssp algorithm
     */
    int source = 1;
    double *distances = new double[NUM_VERTICES+1];
    vector<unsigned long> *paths = new vector<unsigned long>[NUM_VERTICES+1];

    dijkstra(adaptee, source, distances, paths);

    // output results
    cout << "distance from vertex #" << source << ":" << endl;
    for (unsigned long i=1; i<=NUM_VERTICES; ++i)
    {
        cout << "dist(" << i << ") = " << distances[i];
        cout << "\t path: ";

        for(unsigned long j=0; j<paths[i].size(); ++j)
            cout << "[" << paths[i].at(j) << "]";

        cout << endl;
    }

    delete[] distances;
    delete[] paths;
    delete adaptee;
    delete original;

    return 0;
}
