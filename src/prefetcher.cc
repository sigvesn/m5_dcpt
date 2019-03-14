/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "buffer.cc"
#include <map>
#include <queue>

struct dcpt_table {
    map<Addr, dcpt_table_entry> dcpt_table;
    queue<Addr> program_counters;

    dcpt_table_entry& insert(Addr)
    {
        if (program_counters.size() > table_size) {
            Addr pc = program_counters.front();
            program_counters.pop();
            dcpt_table.erase(pc);
        }

        program_counters.push(Addr);
        return dcpt_table[Addr];
    }

    dcpt_table_entry& lookup(Addr)
    {
        if (dcpt_table.find(Addr) == m.end()) {
            return insert(Addr);
        } else {
            return dcpt_table[Addr];
        }
    }
}

// static dcpt_table_entry dcpt_table[table_size] = {};
static dcpt_table dcpt;

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void prefetch_access(AccessStat stat)
{
    /* pf_addr is now an address within the _next_ cache block */
    Addr pf_addr = stat.mem_addr + BLOCK_SIZE;

    // lookup(pf_addr, dcpt_table);

    /*
     * Issue a prefetch request if a demand miss occured,
     * and the block is not already in cache.
     */
    if (stat.miss && !in_cache(pf_addr) && !in_mshr_queue(pf_addr)) {
        issue_prefetch(pf_addr);
    }
}

void prefetch_complete(Addr addr)
{
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
