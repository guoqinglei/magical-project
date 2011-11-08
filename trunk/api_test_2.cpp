#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include "types.h"
#include "paths.h"

#define NUM_VERTICES 5

using namespace std;

class MyNode
{
public:
    MyNode(int k, char c)
    {
        key = k;
        type = c;
    }
    
    int get_key() const { return key; }
    char get_type() const { return type; }
    
private:
    int key;
    char type;
};


class MyEdge
{
public:
    MyEdge(MyNode *u, MyNode *v, double w, const char *s)
    {
        from = u; 
        to = v;
        weight = w;
        color = string(s);
    }

    MyNode* get_head() const { return to; }
    MyNode* get_tail() const { return from; }
    double get_weight() const { return weight; }
    string get_color() const { return color; }

    void set_head(MyNode *v) { to = v; }
    void set_tail(MyNode *v) { from = v; }
    void set_weight(double val) { weight = val; }
    void set_color(const char *s) { color = string(s); }
    
private:
    MyNode *from;
    MyNode *to;
    double weight;
    string color;
};


class AdjMatrix
{
public:
    AdjMatrix(int n)
    {
        num_vertices = n;
        matrix = new MyEdge**[n];
        for (int i = 0; i<n; ++i)
            matrix[i] = new MyEdge*[n];
    }
    ~AdjMatrix()
    {
        for (unsigned i = 0; i<num_vertices; ++i)
        {
            for (unsigned j = 0; j<num_vertices; ++j)
                if (matrix[i][j])
                    delete matrix[i][j];

            delete[] matrix[i];
        }
        delete[] matrix;
    }
    
    MyEdge ***matrix;

private:
    unsigned int num_vertices;
};

vector< pair<MyNode*,MyEdge*> >* get_adjacencies(AdjMatrix *m)
{
    vector< pair<MyNode*,MyEdge*> > *h = new vector< pair<MyNode*,MyEdge*> >[NUM_VERTICES];

    // check each row in the original graph
    for (int i=0; i<NUM_VERTICES; ++i)
    {
        // for each non empty column, insert such adjacency in the resulting vector
        for (int j=0; j<NUM_VERTICES; ++j)
        {
            if (m->matrix[i][j] != 0)
            {
                MyNode* vertex = m->matrix[i][j]->get_head();
                MyEdge* edge = m->matrix[i][j];

                pair<MyNode*,MyEdge*> arc = make_pair(vertex, edge);
                h[i].push_back(arc);
            }
        }
    }

    return h;
}


int main()
{
    AdjMatrix *original = new AdjMatrix(NUM_VERTICES);

    // vertices (instancias de MyNode)
    MyNode* vertices[NUM_VERTICES];
    for (int i = 0; i<NUM_VERTICES; ++i)
        vertices[i] = new MyNode(i, 'A'+i);

    // arcos (instancias de MyEdge)
    original->matrix[0][1] = new MyEdge(vertices[0], vertices[1], 4, "cor0");
    original->matrix[0][2] = new MyEdge(vertices[0], vertices[2], 1, "cor0");

    original->matrix[1][2] = new MyEdge(vertices[1], vertices[2], 3, "cor1");
    original->matrix[1][3] = new MyEdge(vertices[1], vertices[3], 3, "cor1");
    original->matrix[1][4] = new MyEdge(vertices[1], vertices[4], 1, "cor1");

    original->matrix[2][3] = new MyEdge(vertices[2], vertices[3], 1, "cor2");
    original->matrix[2][4] = new MyEdge(vertices[2], vertices[4], 18, "cor2");

    original->matrix[3][1] = new MyEdge(vertices[3], vertices[1], 1, "cor3");

    original->matrix[4][1] = new MyEdge(vertices[4], vertices[1], 0, "cor4");
    original->matrix[4][3] = new MyEdge(vertices[4], vertices[3], 1, "cor4");

    /*
     * interfacing with the library API
     */
    vector< pair<MyNode*,MyEdge*> > *adj = get_adjacencies(original);
    uAdjacencyList<MyNode,MyEdge> *adaptee = adapter<MyNode,MyEdge>(adj, NUM_VERTICES);

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
