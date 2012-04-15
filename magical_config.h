#ifndef __MAGICAL_CONFIG_H__
#define __MAGICAL_CONFIG_H__

// IMPORTANT: NAME OF THE LIBRARY CONFIGURATION FILE
#define _MAGICAL_CONFIG_FILE "magical_config.xml"

#include <string>
#include <list>
#include <map>
#include <cstring>
#include <iostream>
#include "tinyxml_src/tinyxml.h"

using namespace std;

/* namespace reference containing data structures for settings of each parallel
 * algorithm implemented. This is the interface exported to other library files,
 * while the proper initialization is in the corresponding .cpp file.
 */
namespace magical_config
{
    namespace johnson
    {
        extern map<string, string> defaults;
        extern map<pair<unsigned long, unsigned long>,string> threads;
    }
    
    namespace boruvka
    {
        extern map<string, string> defaults;
        extern map<pair<unsigned long, unsigned long>,string> threads;
    }
    
    namespace hierholzer
    {
        extern map<string, string> defaults;
        extern map<pair<unsigned long, unsigned long>,string> threads;
    }
    
    // api for manually setting options (allows dynamic changing configuration)
    void set_threads(unsigned int);
    
    // shall be called by every library algorithm to load the configuration
    bool load_settings(const char*, unsigned long);
    
    // reads xml configuration file specifying the parallel execution settings
    bool parse_file();
}

#endif /* __MAGICAL_CONFIG_H__ */
