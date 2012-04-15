#include "euler_tour.h"
//#include <omp.h>

#define DEBUG
//#define DEBUG1
#define PROCS 1
#define OPENMP

void build_circuit(vector<ulong>*,vector<ulong>*);

bool euler_tour(AdjacencyList<>* g, std::vector<ulong>* circuit, ulong nEdges)
{
    AdjacencyList<>* gs;
    ulong numNodes = g->get_vertex_count();
    map<string, pair<ulong,Edge*> > tours[PROCS];
    vector<ulong> circuit_aux;
    vector<ulong> build;
    vector<ulong> starts;
    stack< pair<ulong,ulong> > rec;
    stringstream skey;
    Vertex* V;
    Edge* E;
    ulong k;
    
    gs = g;
    
    map<string, pair<ulong,Edge*> >::iterator I;
    vector< map<string, pair<ulong,Edge*> >::iterator > Ptrs;
    //map<string, pair<ulong,Edge*> >::iterator Ptrs[tours[0].size()];
    //map<string, pair<ulong,Edge*> >::iterator* Ptrs;
    //Ptrs = new map<string, pair<ulong,Edge*> >::iterator[tours[0].size()];

    ulong currentV;
    ulong startV;
    ulong aux;
    bool flag;
    bool stop;
    double start, end;
    
#ifdef OPENMP
    //setenv("OMP_NUM_THREADS","4",1);
    printf("Number of threads: %d\n",omp_get_num_threads());
    printf("Avaliable processors: %d\n", omp_get_num_procs());
    printf("Max threads: %d\n",omp_get_max_threads());
#endif

    /*check for the first vertex with an avaliable edge*/
    startV = 1;  //test
//    V = g->get_vertex(startV);
//    E = V->get_adjacencies();
//    currentV = E->get_successor()->get_key();
//    skey<<startV<<":"<<currentV;
//    cout<<startV<<" "<<currentV<<endl;
    
//    tours[0].insert(make_pair(skey.str(),make_pair(k,E)));
 
    //for(int i=0; i<nEdges; i++)
        //circuit_aux.push_back(0);
    circuit_aux.reserve(numNodes+nEdges);
    stop = false;
    k = 0;
    while(!stop)
    {
        circuit_aux[k] = startV;
        //circuit_aux.push_back(startV);
        V = g->get_vertex(startV);
        E = V->get_adjacencies();
        currentV = E->get_successor()->get_key();
        skey.str("");
        skey<<startV<<":"<<currentV;
        //cout<<startV<<"***"<<currentV<<endl;
        k++;
        tours[0].insert(make_pair(skey.str(),make_pair(k,E)));        
        while(currentV != startV)//&&E!=NULL
        {
            skey.str("");
            skey<<currentV<<":";
            //cout<<currentV<<" ";
            V = g->get_vertex(currentV);
            E = V->get_adjacencies();
            flag = true;
            while(flag)
            {
                aux = E->get_successor()->get_key();
                skey<<aux;
                if(tours[0].find(skey.str()) == tours[0].end())
                {
                    k++;
                    tours[0].insert(make_pair(skey.str(),make_pair(k,E)));
                    //cout<<"aux:"<<aux<<endl;
                    currentV = aux;
                    flag = false;
                }
                else
                {
                    //cout<<"tentou:"<<aux<<endl;
                    skey.str("");
                    skey<<currentV<<":";
                    E = E->get_next();
                }
            }
            //cout<<currentV<<endl;
            //tours[0].insert(make_pair(skey.str(),make_pair(k,E)));
            //E = E->get_next();
        }

        assert(g==gs);

        /*parallel: find the tour with maximum number of edges*/
        ulong max;
        max = 0;

        ulong is=0;
        Ptrs.clear();
        for(I=tours[max].begin(); I!=tours[max].end(); I++)
        {
            //cout<<I->second.first<<" "<<(I->second).second->get_origin()->get_key()<<" "<<(I->second).second->get_successor()->get_key()<<endl;;
            //g->removeEdge((I->second).second->get_origin()->get_key(),(I->second).second->get_successor()->get_key());  
            //cout<<"removed: "<<(I->second).second->get_origin()->get_key()<<" "<<(I->second).second->get_successor()->get_key()<<endl;
            Ptrs.push_back(I);
        }
            //Ptrs[is++] = I;

        assert(g==gs);

        /*for parallelization...*/
        start = omp_get_wtime();
        //#pragma omp parallel for private(is,E,aux)
        for(is=0; is<tours[max].size(); is++)
        {
            E = (Ptrs[is]->second).second;
            aux = (Ptrs[is]->second).first;
            circuit_aux[aux] = E->get_successor()->get_key();
            //cout<<Ptrs[is]->first<<" "<<E->get_origin()->get_key()<<" "<<E->get_successor()->get_key()<<endl;;
            g->removeEdge(E->get_origin()->get_key(),E->get_successor()->get_key());  
            //cout<<"removed: "<<E->get_origin()->get_key()<<" "<<E->get_successor()->get_key()<<endl;
        }

        //#pragma omp parallel for private(i)
        for(int i=0; i<PROCS; ++i)
            tours[i].clear();
        
        end = omp_get_wtime();
        printf(" took %f seconds.\n", end-start);

#ifdef DEBUG1       
        for(int i=0;i<=k;++i)
            cout<<circuit_aux[i]<<" ";
        cout<<endl;
#endif

        stop = true;
        //cout<<"k:"<<k<<endl;
        for(ulong i=0; i<k && stop; ++i)
        {
            startV = circuit_aux[i];
            //cout<<"start:"<<startV<<endl;
            V = g->get_vertex(startV);
            if(V)
                if(V->get_adjacencies())
                {
                    stop = false;
                    //cout<<"new start vertex: "<<startV<<"k: "<<k<<endl;
                    build.push_back(++k);
                    starts.push_back(i);
                    //circuit_aux[k++] = startV;
                    //k++;
                }
        }
    }
    circuit_aux.push_back(circuit_aux[0]);
    k++;

#ifdef DEBUG
    cout<<"circuit_aux:";
    for(ulong i=0;i<k;++i)
        cout<<circuit_aux[i]<<" ";
    cout<<endl;
    cout<<"build:";
    for(unsigned int i=0;i<build.size();++i)
        cout<<build[i]<<" ";
    cout<<endl;
    cout<<"starts:";
    for(unsigned int i=0;i<starts.size();++i)
        cout<<starts[i]<<" ";
    cout<<endl;
#endif
    
    /*Build the Euler tour merging the tours*/

    ulong is = 0;
    ulong js = 0;
    
    startV = circuit_aux[0];
    
    aux=0;
    stop = false;
    while(!stop)
    {
        //cout<<"starting at "<<is<<" startV("<<startV<<")"<<endl;
        do
        {
            /*in parallel?*/
            flag = false;
            for(unsigned aux=0;aux<starts.size() && !flag;aux++)
            {
                if(is==starts[aux])
                {
                    flag = true;
                    js = aux;
                }
            }
            if(!flag && circuit_aux[is]!=startV)
            {
                circuit->push_back(circuit_aux[is]);
                is++;
            }
        }while(!flag && circuit_aux[is]!=startV); //is!=aux

        if(flag)
        {
            //cout<<"at "<<is<<" starts a new cycle."<<endl;
            rec.push(make_pair(startV,is+1));
            is=build[js];
            circuit->push_back(circuit_aux[is]);
            startV=circuit_aux[is++];
            //++js;
        }
        else if(circuit_aux[is]==startV)
        {
            if(!rec.empty())
            {
                //cout<<"cycle finished at "<<is<<endl;
                circuit->push_back(startV);
                startV=rec.top().first;
                is = rec.top().second;
                rec.pop();
                //cout<<"back to "<<is<<"(startV)"<<startV<<endl;
            }
            else
            {
                if(circuit->size() == nEdges)
                    stop = true;
                else
                    is++;
            }
        }
    }
    if(circuit->back() != circuit->front())
        circuit->push_back(circuit_aux[0]);
    
    circuit_aux.clear();
    build.clear();
    starts.clear();
    Ptrs.clear();
    
    return true;
}
