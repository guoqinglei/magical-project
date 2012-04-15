#include "magical_config.h"
#include <climits>
#include <cstdlib>
#include <omp.h>

#define ulong unsigned long

// initialization of the structures implementing the parallel settings
namespace magical_config
{
    // prevents permanent (xml file) setting to override dynamic (api functions)
    static bool threads_manually_set = false;

    // prevents parsing the file again (unnecessarily)
    static bool file_already_loaded = false;

    /* IMPORTANT: when including an algorithm in the library, please include a
     * corresponding namespace here too.
     */
    namespace johnson
    {
        map<string, string> defaults;
        map<pair<ulong,ulong>,string> threads;
    }
    
    namespace boruvka
    {
        map<string, string> defaults;
        map<pair<ulong,ulong>,string> threads;
    }
    
    namespace hierholzer
    {
        map<string, string> defaults;
        map<pair<ulong,ulong>,string> threads;
    }
    
    void set_threads(unsigned int thr_count)
    {
        threads_manually_set = true;
        omp_set_num_threads(thr_count);
    }
    
    bool load_settings(const char *algorithm, ulong input_size)
    {
        /* input_size is algorithm-dependent (e.g. #vertices, #edges), but must
         * match xml file description
         */
        
        if (!threads_manually_set)
        {
            // parse xml file if not done yet
            if (!file_already_loaded)
            {
                file_already_loaded = true;
                parse_file();
            }
            
            /* IMPORTANT: when including a new algorithm, please register a
             * corresponding entry here (to use algorithm-specific settings)
             */
            map<string, string> *defaults_ptr = 0;
            map<pair<ulong,ulong>,string> *threads_ptr = 0;
            
            if(strcmp(algorithm, "johnson") == 0)
            {
                defaults_ptr = &(johnson::defaults);
                threads_ptr  = &(johnson::threads);
            }
            else if(strcmp(algorithm, "boruvka") == 0)
            {
                defaults_ptr = &(boruvka::defaults);
                threads_ptr  = &(boruvka::threads);
            }
            else if(strcmp(algorithm, "hierholzer") == 0)
            {
                defaults_ptr = &(hierholzer::defaults);
                threads_ptr  = &(hierholzer::threads);
            }
            else
            {
                // could not match given string
                return false;
            }
            
            // define settings according to specific configuration
            map<pair<ulong, ulong>,string>::iterator it;
            
            for (it = threads_ptr->begin(); it != threads_ptr->end(); ++it)
            {
                pair<ulong, ulong> interval = (*it).first;
                
                // found an interval including the given parameter
                if (input_size >= interval.first && input_size <= interval.second)
                {
                    ulong thr = (*it).second.compare("#cores") == 0 ? omp_get_num_procs() : atoi((*it).second.c_str());
                    omp_set_num_threads(thr);
                    return true;
                }
            }
            
            // no entry regarding current size was found .: use default settings
            ulong thr = (*defaults_ptr)["threads"].compare("#cores") == 0 ? omp_get_num_procs() : atoi((*defaults_ptr)["threads"].c_str());
            omp_set_num_threads(thr);
            return true;
        }
        
        return true;   // manual settings
    }
    
    bool parse_file()
    {
    	TiXmlDocument doc(_MAGICAL_CONFIG_FILE);
    	if (!doc.LoadFile())
    	    return false;

    	TiXmlHandle hDoc(&doc);
    	TiXmlElement* pElem;
    	TiXmlHandle hRoot(0);
		
    	// root block
    	{
    		pElem = hDoc.FirstChildElement().Element();
		
    		// should always have a valid root but handle gracefully if it does not
    		if (!pElem)
    		    return false;
    		//else
            //    cout << "Parsing: " << pElem->Value() << endl;
		
    		// must save this for later
    		hRoot=TiXmlHandle(pElem);
    	}
    
        ////////////////////////////////////////////
        // block: Johnson's All-Pairs Shortest-Paths
    	{
    		pElem =
    		    hRoot.FirstChild("johnson_shortest_paths").FirstChild().Element();
		
    		if (!pElem)
    		{
    		    // set all default values
    		    magical_config::johnson::defaults["threads"] = "#cores";
    		}
		
    		while (pElem)
    		{
    			const char *pKey = pElem->Value();
                const char *pVal;
            
    			if (pKey)
    			{
    			    if (strcmp(pKey, "default") == 0)
    			    {
    			        pVal = pElem->Attribute("threads");
                        magical_config::johnson::defaults["threads"] = pVal;
    			    }
    			    else if(strcmp(pKey, "input") == 0)
    			    {
    			        // min value
    			        ulong min = 0;
    			        pKey = 0;
    			        pKey = pElem->Attribute("min_vertices");
    			        if (pKey)
                            min = atol(pKey);

    			        // max value
                        ulong max = ULONG_MAX;
    			        pKey = 0;
    			        pKey = pElem->Attribute("max_vertices");
    			        if (pKey)
                            max = atol(pKey);
                    
                        // config value
                        pVal = pElem->Attribute("threads");
                        magical_config::johnson::threads[make_pair(min,max)] = pVal;
    			    }
    			    else
    			    {
    			        // current behavior: ignore unexpected elements
    			    }
    			}
    			else
    			{
    			    // should not reach here: element has no name/value?
    			}
			
    			// next sibling entry (another setting for this algorithm)
                pElem = pElem->NextSiblingElement();
    		}
    	}
	
    	/////////////////////////////////////////
        // block: Boruvka's Minimum Spanning Tree
    	{
    		pElem =
    		    hRoot.FirstChild("boruvka_mst").FirstChild().Element();
		
    		if (!pElem)
    		{
    		    // set all default values
    		    magical_config::boruvka::defaults["threads"] = "#cores";
    		}
		
    		while (pElem)
    		{
    			const char *pKey = pElem->Value();
                const char *pVal;
            
    			if (pKey)
    			{
    			    if (strcmp(pKey, "default") == 0)
    			    {
    			        pVal = pElem->Attribute("threads");
                        magical_config::boruvka::defaults["threads"] = pVal;
    			    }
    			    else if(strcmp(pKey, "input") == 0)
    			    {
    			        // min value
    			        ulong min = 0;
    			        pKey = 0;
    			        pKey = pElem->Attribute("min_vertices");
    			        if (pKey)
                            min = atol(pKey);

    			        // max value
                        ulong max = ULONG_MAX;
    			        pKey = 0;
    			        pKey = pElem->Attribute("max_vertices");
    			        if (pKey)
                            max = atol(pKey);
                    
                        // config value
                        pVal = pElem->Attribute("threads");
                        magical_config::boruvka::threads[make_pair(min,max)] = pVal;
    			    }
    			    else
    			    {
    			        // current behavior: ignore unexpected elements
    			    }
    			}
    			else
    			{
    			    // should not reach here: element has no name/value?
    			}
            
    			// next sibling entry (another setting for this algorithm)
                pElem = pElem->NextSiblingElement();
    		}
    	}
	
    	///////////////////////////////////////
        // block: Hierholzer's Eulerian Circuit
    	{
    		pElem =
    		hRoot.FirstChild("hierholzer_eulerian_circuit").FirstChild().Element();
		
    		if (!pElem)
    		{
    		    // set all default values
    		    magical_config::hierholzer::defaults["threads"] = "#cores";
    		}
		
    		while (pElem)
    		{
    			const char *pKey = pElem->Value();
                const char *pVal;
            
    			if (pKey)
    			{
    			    if (strcmp(pKey, "default") == 0)
    			    {
    			        pVal = pElem->Attribute("threads");
                        magical_config::hierholzer::defaults["threads"] = pVal;
    			    }
    			    else if(strcmp(pKey, "input") == 0)
    			    {
    			        // min value
    			        ulong min = 0;
    			        pKey = 0;
    			        pKey = pElem->Attribute("min_vertices");
    			        if (pKey)
                            min = atol(pKey);

    			        // max value
                        ulong max = ULONG_MAX;
    			        pKey = 0;
    			        pKey = pElem->Attribute("max_vertices");
    			        if (pKey)
                            max = atol(pKey);
                    
                        // config value
                        pVal = pElem->Attribute("threads");
                        magical_config::hierholzer::threads[make_pair(min,max)] = pVal;
    			    }
    			    else
    			    {
    			        // current behavior: ignore unexpected elements
    			    }
    			}
    			else
    			{
    			    // should not reach here: element has no name/value?
    			}
			
    			// next sibling entry (another setting for this algorithm)
                pElem = pElem->NextSiblingElement();
    		}
    	}
	
    	///////////////////
    	// parsing complete
        return true;
    }
    
} // namespace magical_config
