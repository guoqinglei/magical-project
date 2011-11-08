#ifndef __PATHS_H__
#define __PATHS_H__

#include <vector>
#include "types.h"

/* Dijkstra's single-source shortest path algorithm */
void dijkstra(AdjacencyList<>*, unsigned long, double*, std::vector<unsigned long>*);

/* Bellman-Ford's single-source shortest path algorithm */
bool bellman_ford(AdjacencyList<>*, unsigned long, double*, std::vector<unsigned long>*);

/* Johnson's all-pairs shortest path algorithm */
bool johnson(AdjacencyList<>*, double**, std::vector<unsigned long>**);

#endif /* __PATHS_H__ */
