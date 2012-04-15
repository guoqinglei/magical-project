#ifndef __EULER_H__
#define __EULER_H__

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <iostream>
#include <stack>
#include <omp.h>
#include <assert.h>
#include "types.h"

#define ulong unsigned long

using namespace std;

/* Hierholzer's algorithm for finding an Euleur's circuit */
bool euler_tour(AdjacencyList<>*, std::vector<unsigned long>*, ulong);

#endif /* __EULER_H__ */
