/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "buffer.hh"
using namespace std;

static dcpt_table dcpt;

vector<Addr> prefetch_filter(dcpt_table_entry entry, vector<Addr> candidates);
vector<Addr> delta_correlation(dcpt_table_entry entry);

void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}
// Addr pf_addr = stat.mem_addr + BLOCK_SIZE;
//
// if (stat.miss && !in_cache(pf_addr) && !in_mshr_queue(pf_addr)) {
//     issue_prefetch(pf_addr);
// }

void prefetch_access(AccessStat stat)
{
    dcpt_table_entry entry = dcpt.lookup(stat.mem_addr);

    if (stat.mem_addr == entry.last_addr) {
        entry.delta_buffer.push(stat.mem_addr - entry.last_addr);
        entry.last_addr = stat.mem_addr;

        vector<Addr> candidates = delta_correlation(entry);
        vector<Addr> prefetches = prefetch_filter(entry, candidates);
        for (vector<Addr>::iterator addr = prefetches.begin(); addr != prefetches.end(); ++addr) {
            if (current_queue_size() < MAX_QUEUE_SIZE)
                issue_prefetch(*addr);
        }
    }
}

vector<Addr> prefetch_filter(dcpt_table_entry entry, vector<Addr> candidates)
{
    vector<Addr> prefetches;

    for (vector<Addr>::iterator candidate = candidates.begin(); candidate != candidates.end(); ++candidate) {

        if (!in_cache(*candidate) && !in_mshr_queue(*candidate) && current_queue_size() < MAX_QUEUE_SIZE) {
            prefetches.push_back(*candidate);
            entry.last_prefetch = *candidate;
        }

        if (*candidate == entry.last_prefetch)
            prefetches.clear();
    }

    return prefetches;
}

vector<Addr> delta_correlation(dcpt_table_entry entry)
{
    vector<Addr> candidates;

    deque<int64_t>::reverse_iterator it = entry.delta_buffer.buffer.rbegin();

    Addr d1 = *(++it);
    Addr d2 = *(++it);

    Addr address = entry.last_addr;

    for (int i = 0; i < buf_size - 2; ++i) {
        Addr u = *(it++);
        Addr v = *(it);

        if (u == d1 && v == d2) {
            for (; it != entry.delta_buffer.buffer.rend(); ++it) {
                address += *it;
                candidates.push_back(address);
            }
            break;
        }
    }
    return candidates;
}

void prefetch_complete(Addr addr)
{
    /*
     * Called when a block requested by the prefetcher has been loaded.
     */
}
