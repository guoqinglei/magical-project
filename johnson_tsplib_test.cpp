#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include "types.h"
#include "paths.h"

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
	
}

AdjacencyList<>* graph_from_tsplib(const char * filename)
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


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        cout << "usage: " << argv[0] << " [tsplib_file]" << endl;
        return 1;
    }
    
    AdjacencyList<> *graph = graph_from_tsplib(argv[1]);

    if (graph == 0)
        return(0);

	// -- time evaluation (start) ----------------------------------------------
	start_timer();

	struct rusage *resources = (struct rusage *) malloc( sizeof(struct rusage) );
	int rc;
	double u_time, s_time;
	// -------------------------------------------------------------------------

    unsigned long num_vertices = graph->get_vertex_count();
    
    // 'distances' and 'paths': result data structures (with dummy node at head)
    double **distances = new double* [num_vertices+1];
    vector<unsigned long> **paths = new vector<unsigned long>* [num_vertices+1];

    for (unsigned long n = 0; n<num_vertices+1; ++n)
    {
        distances[n] = new double[num_vertices+1];
        paths[n] = new vector<unsigned long>[num_vertices+1];
    }

    if (johnson(graph, distances, paths) == false)
        cout << "negative-weight cycle detected" << endl;

    // clean-up
    for (unsigned long n = 0; n<num_vertices+1; ++n)
    {
        delete[] distances[n];
        delete[] paths[n];
    }
    delete[] distances;
    delete[] paths;
    delete graph;

	// -- time evaluation (finish) ---------------------------------------------
	get_timer();   // saida do tempo de relogio gasto
	rc = getrusage( RUSAGE_SELF , resources );
	
	if( rc != 0 )
	    perror( "getrusage failed" );
	
	u_time = (double) resources->ru_utime.tv_sec + 1.e-6 * (double) resources->ru_utime.tv_usec;
	s_time = (double) resources->ru_stime.tv_sec + 1.e-6 * (double) resources->ru_stime.tv_usec;
	
	// printf("\nuser time: %.6f sec\nsys time: %.6f sec\n", u_time, s_time);
}
