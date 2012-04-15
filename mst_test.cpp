#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "types.h"
#include "mst.h"

#include <sys/time.h>       // for 'gettimeofday()'
#include <sys/resource.h>   // for 'getrusage()'

using namespace std;

// -- time evaluation functions ------------------------------------------------

static struct timeval *clock_start;

void start_timer()
{
	// current clock time
	clock_start = (struct timeval *) malloc(sizeof(struct timeval) );
	gettimeofday(clock_start, 0);
}

void get_timer()
{
    // outputs elapsed time

	unsigned long clock_time;
	struct timeval *clock_stop;
	
	clock_stop = (struct timeval *) malloc( sizeof(struct timeval) );
	
	gettimeofday(clock_stop, 0);
	
	clock_time = 1.e6 * (clock_stop->tv_sec - clock_start->tv_sec) +
	                    (clock_stop->tv_usec - clock_start->tv_usec);
	
	printf( "%.6f\n" , ((double)clock_time / (double)1.e6) );
    
    free(clock_start);
    free(clock_stop);
}

// -----------------------------------------------------------------------------

AdjacencyList<>* completeGraph(unsigned long num_vertices, unsigned long range)
{
    AdjacencyList<> *kn = new AdjacencyList<>(num_vertices);

    srand(1234567);

    for (unsigned long i=1; i<=num_vertices; ++i)
    {
        for (unsigned long j=1; j<=num_vertices; ++j)
        {
            if (i==j)
                continue;
            
            unsigned long w = rand() % (range+1);   // edge weight w in [0..range]
            kn->addEdge(i, j, w);
        }
    }
    
    return kn;
}

AdjacencyList<>* randomTree(unsigned long num_vertices, unsigned long range)
{
    AdjacencyList<> *tree = new AdjacencyList<>(num_vertices);
    unsigned long i;
    long w;

    srand(1234567);

    for (unsigned long j=2; j<=num_vertices; ++j)
    {
        w = rand() % (range+1);   // edge weight w in [0..range]
        i = (rand() % (j-1)) + 1;   // vertex i in [1..j-1], i.e. already in the tree
        tree->addEdge(i, j, w);
        tree->addEdge(j, i, w);
    }
    
    return tree;
}

AdjacencyList<>* lineGraph(unsigned long num_vertices, unsigned long range)
{
    AdjacencyList<> *line = new AdjacencyList<>(num_vertices);

    srand(1234567);

    for (unsigned long j=2; j<=num_vertices; ++j)
    {
        unsigned long w = rand() % (range+1);   // edge weight w in [0..range]
        unsigned long i = j - 1;
        line->addEdge(i, j, w);
        line->addEdge(j, i, w);
    }
    
    return line;
}

AdjacencyList<>* tsplibGraph(const char * filename)
{
    // cities coordinates vectors
    vector<float> xcoord;
    vector<float> ycoord;
    xcoord.clear();
    ycoord.clear();
	
    // input file handler
    ifstream input_fh(filename);

    if (input_fh.is_open())
    {
        string line;

        // skips file until the string preciding the first coordinates is found 
        getline(input_fh, line);
        while(line.find("NODE_COORD_SECTION") == string::npos)
        {
            getline(input_fh, line);
            //cout << line << endl;
        }

        // parse each line, until 'end of file' is found
        getline(input_fh, line);
        while(line.find("EOF") == string::npos)
        {
            float x, y;
            
            // reads current coordinates, ignoring the city index
            sscanf(line.c_str(), "%*d %f %f", &x, &y);
            
            xcoord.push_back(x);
            ycoord.push_back(y);
            
            getline(input_fh, line);
        }

        input_fh.close();
    }
    else
    {
        cerr << "ERROR: Could not open file (might not exist)." << endl;
        return(0);
    }

    //cout << "completed reading tsplib file" << endl;
    //cout << "# cities: " << xcoord.size() << endl;

    // creates graph corresponding to this TSP instance
    unsigned long num_vertices = xcoord.size();
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    // in the case of constructing a sparse graph
    //srand (time(0));

    for (unsigned int i = 0; i<num_vertices; ++i)
    {
        for (unsigned int j = 0; j<num_vertices; ++j)
        {
            // euclidean distance between cities i and j
            double xd = xcoord[i] - xcoord[j];
            double yd = ycoord[i] - ycoord[j];
            double dij = floor( sqrt(pow(xd,2)+pow(yd,2)) + 0.5 );

            // inserts an arc joining i and j, with weight = dij
            //if ((rand() % 100) < 10)      // sparsity: 10% of the edges only
            graph->addEdge(i+1, j+1, dij);
        }
    }

    return graph;
}

int main()
{
    unsigned long num_vertices = 5000;
    //AdjacencyList<> *graph = randomTree(num_vertices, 1000000);
    AdjacencyList<> *graph = completeGraph(num_vertices, 100000);
    //AdjacencyList<> *graph = lineGraph(num_vertices, 100);
    //AdjacencyList<> *graph = tsplibGraph("tsplib_input/u2319.tsp");
    
    if (graph == 0)
        return(0);
    
/*
    // graph from figure 5.3 (pag 143) at dasgupta, papadimitrou e vazirani
    unsigned long num_vertices = 6;
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    graph->addEdge(1,2,2);
    graph->addEdge(1,3,1);
    
    graph->addEdge(2,1,2);
    graph->addEdge(2,3,2);
    graph->addEdge(2,4,1);
    
    graph->addEdge(3,1,1);
    graph->addEdge(3,2,2);
    graph->addEdge(3,4,2);
    graph->addEdge(3,5,3);
    
    graph->addEdge(4,2,1);
    graph->addEdge(4,3,2);
    graph->addEdge(4,5,3);
    graph->addEdge(4,6,4);
    
    graph->addEdge(5,3,3);
    graph->addEdge(5,4,3);
    graph->addEdge(5,6,1);
    
    graph->addEdge(6,4,4);
    graph->addEdge(6,5,1);
    
    // solution
    // 1 3 1
    // 1 2 2
    // 2 4 1
    // 4 5 3
    // 5 6 1
*/
/*
    // graph from the first example at dasgupta, papadimitrou e vazirani
    unsigned long num_vertices = 6;
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    graph->addEdge(1,2,5);
    graph->addEdge(1,3,6);
    graph->addEdge(1,4,4);
    
    graph->addEdge(2,1,5);
    graph->addEdge(2,3,1);
    graph->addEdge(2,4,2);
    
    graph->addEdge(3,1,6);
    graph->addEdge(3,2,1);
    graph->addEdge(3,4,2);
    graph->addEdge(3,5,5);
    graph->addEdge(3,6,3);
    
    graph->addEdge(4,1,4);
    graph->addEdge(4,2,2);
    graph->addEdge(4,3,2);
    graph->addEdge(4,6,4);
    
    graph->addEdge(5,3,5);
    graph->addEdge(5,6,4);
    
    graph->addEdge(6,3,3);
    graph->addEdge(6,4,4);
    graph->addEdge(6,5,4);
    
    // solution
    // 1 4 4
    // 2 3 1
    // 3 4 2   // ou 2 4 2 
    // 3 6 3
    // 5 6 4
*/
/*
    // graph from the example at Cormen et al.
    unsigned long num_vertices = 9;
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    graph->addEdge(1,2,4);
    graph->addEdge(1,8,8);
    
    graph->addEdge(2,1,4);
    graph->addEdge(2,3,8);
    graph->addEdge(2,8,11);
    
    graph->addEdge(3,2,8);
    graph->addEdge(3,4,7);
    graph->addEdge(3,6,4);
    graph->addEdge(3,9,2);
    
    graph->addEdge(4,3,7);
    graph->addEdge(4,5,9);
    graph->addEdge(4,6,14);
    
    graph->addEdge(5,4,9);
    graph->addEdge(5,6,10);
    
    graph->addEdge(6,3,4);
    graph->addEdge(6,4,14);
    graph->addEdge(6,5,10);
    graph->addEdge(6,7,2);
    
    graph->addEdge(7,6,2);
    graph->addEdge(7,8,1);
    graph->addEdge(7,9,6);
    
    graph->addEdge(8,1,8);
    graph->addEdge(8,2,11);
    graph->addEdge(8,7,1);
    graph->addEdge(8,9,7);
    
    graph->addEdge(9,3,2);
    graph->addEdge(9,7,6);
    graph->addEdge(9,8,7);
    
    // solution:
    // 1 2 4
    // 1 8 8   // ou 2 3 8
    // 3 4 7
    // 3 6 4
    // 3 9 2
    // 4 5 9
    // 6 7 2
    // 7 8 1
*/
    
/*
    // graph from the second figure in the english article in wikipedia:
    // http://en.wikipedia.org/wiki/Minimum_spanning_tree
    unsigned long num_vertices = 6;
    AdjacencyList<> *graph = new AdjacencyList<>(num_vertices);

    graph->addEdge(1, 2, 1);
    graph->addEdge(1, 4, 4);
    graph->addEdge(1, 5, 3);

    graph->addEdge(2, 1, 1);
    graph->addEdge(2, 4, 4);
    graph->addEdge(2, 5, 2);

    graph->addEdge(3, 5, 4);
    graph->addEdge(3, 6, 5);

    graph->addEdge(4, 1, 4);
    graph->addEdge(4, 2, 4);
    graph->addEdge(4, 5, 4);

    graph->addEdge(5, 1, 3);
    graph->addEdge(5, 2, 2);
    graph->addEdge(5, 3, 4);
    graph->addEdge(5, 4, 4);
    graph->addEdge(5, 5, 7);

    graph->addEdge(6, 3, 5);
    graph->addEdge(6, 5, 7);
*/

    // 'mst': result data structure
    AdjacencyList<> *mst = new AdjacencyList<>(num_vertices);
    
    // -- time evaluation (start) ----------------------------------------------
	start_timer();

	struct rusage *resources = (struct rusage *) malloc( sizeof(struct rusage) );
	int rc;
	//double u_time, s_time;
	// -------------------------------------------------------------------------
    
    if (!boruvka(graph, mst))
        cerr << "boruvka returned false" << endl;
    else
    {
        // -- time evaluation (finish) -----------------------------------------
    	get_timer();   // saida do tempo de relogio gasto
    	rc = getrusage( RUSAGE_SELF , resources );

    	if( rc != 0 )
    	    perror( "getrusage failed" );

    	//u_time = (double) resources->ru_utime.tv_sec
    	//                + 1.e-6 * (double) resources->ru_utime.tv_usec;
    	//s_time = (double) resources->ru_stime.tv_sec
    	//                + 1.e-6 * (double) resources->ru_stime.tv_usec;
        free(resources);
    	// ---------------------------------------------------------------------
	    
	    // output results
/*
        unsigned long mst_vertices = mst->get_vertex_count();
        double mst_weight = 0;
	    
		cout << endl << "the mst is" << endl;
		cout << "#vertices = " << mst_vertices << endl;
//		cout << "edges:" << endl;
		
        for (unsigned long i=1; i<=mst_vertices; ++i)
		{
            Vertex *u = mst->get_vertex(i);
            Edge* it = u->get_adjacencies();
            while (it)
            {
                // current adjacency information
                unsigned long v = it->get_successor()->get_key();
                double w = it->get_weight();
                
                if (u->get_key() <= v)
                {
//                    cout << "\t" << u->get_key() << " -> " << v
//                    << " (" << w << ")" << endl;

                    mst_weight += w;
                }
                
                it = it->get_next();
            }
            
		}
		
        cout << endl << "mst weight = " << mst_weight << endl;
*/
    }
	
    delete mst;
    delete graph;
}
