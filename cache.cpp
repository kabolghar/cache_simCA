#include <iostream>
#include  <iomanip>
#include <vector>
#include <cmath>

using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)

enum cacheResType {MISS=0, HIT=1};

struct cache_line{
	int valid;
	int tag;
	int tracker;
};

// Cache structure
struct cache {
    vector<vector<cache_line>> sets;  
    int num_sets;       
    int num_ways;      
    int line_size;      
};

enum line_size{
	L16 = 16,
	L32 = 32,
	L64 = 64,
	L128 = 128
};

enum cache_associativity{
	DIRECT_MAPPED = 1,
	MAPPED_2WAY = 2,
	MAPPED_4WAY = 4,
	MAPPED_8WAY = 8,
	MAPPED_16WAY = 16
};

/* The following implements a random number generator */
unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1() // sequential incr by 1 bet3eed 3and dram size
{
	static unsigned int addr=0;
	return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2() // generates a random address within limit of 24 kb // small random
{
	static unsigned int addr=0;
	return  rand_()%(24*1024);
}

unsigned int memGen3()  // // generates a random address within dram // big random // worst case performance
{
	return rand_()%(DRAM_SIZE);
}

unsigned int memGen4() // sequential 3ala meory small
{
	static unsigned int addr=0;
	return (addr++)%(4*1024);
}

unsigned int memGen5() // sequntail 3ala memory shabah 4 bas dram= 7agm el cache fa u can fit the dram gowa el cache
{ // miss wahda bas
	static unsigned int addr=0;
	return (addr++)%(1024*64);
}

unsigned int memGen6() //sequntial by 32bytes witin a 256 kb region. with regualar not sequantial access patterns
{
	static unsigned int addr=0;
	return (addr+=32)%(64*4*1024);
}

// Initialize cache with given parameters
cache* initCache(int cache_size, int line_size, int associativity) {
    cache* c = new cache;
    
    // Calculate number of sets
    c->num_sets = cache_size / (line_size * associativity);
    c->num_ways = associativity;
    c->line_size = line_size;
    
    // Initialize the 2D vector with the correct dimensions
    c->sets.resize(c->num_sets, vector<cache_line>(c->num_ways));
    
    // Initialize all cache lines
    for (int i = 0; i < c->num_sets; i++) {
        for (int j = 0; j < c->num_ways; j++) {
            c->sets[i][j].valid = 0;
            c->sets[i][j].tag = 0;
            c->sets[i][j].tracker = 0;
        }
    }
    
    return c;
}

// Global cache pointer
cache* g_cache = nullptr;

// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr)
{	
    if (!g_cache) {
        // Initialize cache if not already done
        g_cache = initCache(CACHE_SIZE, L32, DIRECT_MAPPED);
    }

    // Calculate tag and index
    int offset_bits = log2(g_cache->line_size);
    int index_bits = log2(g_cache->num_sets);
    int tag_bits = 32 - offset_bits - index_bits;

    unsigned int tag = addr >> (offset_bits + index_bits);
    unsigned int index = (addr >> offset_bits) & ((1 << index_bits) - 1);

    // Check all ways in the set for a hit
    for (int way = 0; way < g_cache->num_ways; way++) {
        if (g_cache->sets[index][way].valid && g_cache->sets[index][way].tag == tag) {
            return HIT;
        }
    }

    // Cache miss - find least recently used line in the set
    int lru_way = 0;
    int max_tracker = g_cache->sets[index][0].tracker;
    // the higher the tracker value the less its not been used
    for (int way = 1; way < g_cache->num_ways; way++) {
        if (g_cache->sets[index][way].tracker > max_tracker) {
            max_tracker = g_cache->sets[index][way].tracker;
            lru_way = way;
        }
    }

    // Update the LRU line
    g_cache->sets[index][lru_way].valid = 1;
    g_cache->sets[index][lru_way].tag = tag;
    g_cache->sets[index][lru_way].tracker = 0;

    // Increment all other trackers in the set
    for (int way = 0; way < g_cache->num_ways; way++) {
        if (way != lru_way) g_cache->sets[index][way].tracker++;
    }

    return MISS;
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{	
    if (!g_cache) {
        // Initialize cache if not already done
        g_cache = initCache(CACHE_SIZE, L32, CACHE_SIZE/L32); // For fully associative, num_ways = cache_size/line_size
    }

    // Calculate tag (in fully associative, we only need tag as there's no index)
    int offset_bits = log2(g_cache->line_size);
    unsigned int tag = addr >> offset_bits;

    // Search through all cache lines for a match
    for (int i = 0; i < g_cache->num_sets; i++) {
        if (g_cache->sets[i][0].valid && g_cache->sets[i][0].tag == tag) {
            // Update LRU tracker
            g_cache->sets[i][0].tracker = 0;
            // Increment all other trackers
            for (int j = 0; j < g_cache->num_sets; j++) {
                if (j != i) g_cache->sets[j][0].tracker++;
            }
            return HIT;
        }
    }

    // Cache miss - find least recently used line
    int lru_index = 0;
    int max_tracker = g_cache->sets[0][0].tracker;
    
    for (int i = 1; i < g_cache->num_sets; i++) {
        if (g_cache->sets[i][0].tracker > max_tracker) {
            max_tracker = g_cache->sets[i][0].tracker;
            lru_index = i;
        }
    }

    // Update the LRU line
    g_cache->sets[lru_index][0].valid = 1;
    g_cache->sets[lru_index][0].tag = tag;
    g_cache->sets[lru_index][0].tracker = 0;

    // Increment all other trackers
    for (int i = 0; i < g_cache->num_sets; i++) {
        if (i != lru_index) g_cache->sets[i][0].tracker++;
    }

    return MISS;
}
const char *msg[2] = {"Miss","Hit"};

#define		NO_OF_Iterations	1000000

void runExperiment(int line_size, int associativity, unsigned int (*memGen)(), const char* gen_name) {
    unsigned int hit = 0;
    cacheResType r;
    unsigned int addr;
    
    // Initialize cache with given parameters
    if (g_cache) {
        delete g_cache;
    }
    g_cache = initCache(CACHE_SIZE, line_size, associativity);
    
    for(int inst = 0; inst < NO_OF_Iterations; inst++) {
        addr = memGen();
        // Use FA for fully associative (when associativity equals cache_size/line_size)
        // Otherwise use DM
        if (associativity == CACHE_SIZE/line_size) {
            r = cacheSimFA(addr);
        } else {
            r = cacheSimDM(addr);
        }
        if(r == HIT) hit++;
    }
    
    cout << gen_name << " - Line size: " << line_size << " bytes, Ways: " << associativity 
         << " - Hit ratio = " << (100.0 * hit / NO_OF_Iterations) << "%" << endl;
}

int main() {
    cout << "Cache Performance Analysis\n";
    cout << "Running " << NO_OF_Iterations << " iterations per experiment\n\n";
    
    // Array of memory generators and their names
    struct {
        unsigned int (*func)();
        const char* name;
    } generators[] = {
        {memGen1, "memGen1"},
        {memGen2, "memGen2"},
        {memGen3, "memGen3"},
        {memGen4, "memGen4"},
        {memGen5, "memGen5"},
        {memGen6, "memGen6"}
    };
    
    // Run experiments for each generator
    for (auto& gen : generators) {
        cout << "\n=== Using " << gen.name << " ===\n";
        
        // Experiment 1: Fixed sets (4) with varying line sizes
        cout << "Experiment 1: Fixed sets (4) with varying line sizes\n";
        int associativity = CACHE_SIZE / (4 * L16); // For 16-byte lines
        
        runExperiment(L16, associativity, gen.func, gen.name);
        runExperiment(L32, associativity/2, gen.func, gen.name);
        runExperiment(L64, associativity/4, gen.func, gen.name);
        runExperiment(L128, associativity/8, gen.func, gen.name);
        
        // Experiment 2: Fixed line size (64 bytes) with varying ways
        cout << "\nExperiment 2: Fixed line size (64 bytes) with varying ways\n";
        
        runExperiment(L64, DIRECT_MAPPED, gen.func, gen.name);
        runExperiment(L64, MAPPED_2WAY, gen.func, gen.name);
        runExperiment(L64, MAPPED_4WAY, gen.func, gen.name);
        runExperiment(L64, MAPPED_8WAY, gen.func, gen.name);
        runExperiment(L64, MAPPED_16WAY, gen.func, gen.name);
    }
    
    // Clean up
    if (g_cache) {
        delete g_cache;
        g_cache = nullptr;
    }
    
    return 0;
}