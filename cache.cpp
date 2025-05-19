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

unsigned int memGen1()
{
	static unsigned int addr=0;
	return (addr++)%(DRAM_SIZE);
}

unsigned int memGen2()
{
	static unsigned int addr=0;
	return  rand_()%(24*1024);
}

unsigned int memGen3()
{
	return rand_()%(DRAM_SIZE);
}

unsigned int memGen4()
{
	static unsigned int addr=0;
	return (addr++)%(4*1024);
}

unsigned int memGen5()
{
	static unsigned int addr=0;
	return (addr++)%(1024*64);
}

unsigned int memGen6()
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

    // Check if the line is valid and tag matches
    if (g_cache->sets[index][0].valid && g_cache->sets[index][0].tag == tag) {
        return HIT;
    }

    // Cache miss - update the cache line
    g_cache->sets[index][0].valid = 1;
    g_cache->sets[index][0].tag = tag;
    return MISS;
}

// Fully Associative Cache Simulator
cacheResType cacheSimFA(unsigned int addr)
{	
	// This function accepts the memory address for the read and 
	// returns whether it caused a cache miss or a cache hit

	// The current implementation assumes there is no cache; so, every transaction is a miss
	return MISS;
}
char *msg[2] = {"Miss","Hit"};

#define		NO_OF_Iterations	100		// CHange to 1,000,000
int main()
{
	unsigned int hit = 0;
	cacheResType r;
	
	unsigned int addr;
	cout << "Direct Mapped Cache Simulator\n";

	for(int inst=0;inst<NO_OF_Iterations;inst++)
	{
		addr = memGen2();
		r = cacheSimDM(addr);
		if(r == HIT) hit++;
		cout <<"0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\n";
	}
	cout << "Hit ratio = " << (100*hit/NO_OF_Iterations)<< endl;
}