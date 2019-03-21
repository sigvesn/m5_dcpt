/*
 * A sample prefetcher which does sequential one-block lookahead.
 * This means that the prefetcher fetches the next block _after_ the one that
 * was just accessed. It also ignores requests to blocks already in the cache.
 */

#include "buffer.hh"
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

static dcpt_table dcpt;

vector<Addr> prefetch_filter(dcpt_table_entry entry, vector<Addr>& candidates);
vector<Addr> delta_correlation(dcpt_table_entry entry);

static circular_buffer in_flight(IN_FLIGHT_BUFFER_SIZE);

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

void issue_prefetches(vector<Addr>& prefetches, dcpt_table_entry& entry)
{
	for (addr_it addr = prefetches.begin(); addr != prefetches.end(); ++addr) {
		if (current_queue_size() < MAX_QUEUE_SIZE) {
			issue_prefetch(*addr);
			entry.last_prefetch = *addr;
			in_flight.push(*addr);
		}
	}
}

void prefetch_access(AccessStat stat)
{
	bool inserted = false;
    dcpt_table_entry& entry = dcpt.lookup(stat.pc, inserted);


	if (inserted) {
		entry.last_addr = stat.mem_addr;
	} else if (stat.mem_addr != entry.last_addr) {
		int64_t delta = stat.mem_addr - entry.last_addr;

		// discard delta if it's wider than max delta width
		if (delta > pow(2, DELTA_WIDTH) - 1)
			return;

        entry.delta_buffer.push(delta);
		entry.last_addr = stat.mem_addr;

        vector<Addr> candidates = delta_correlation(entry);
        vector<Addr> prefetches = prefetch_filter(entry, candidates);

		issue_prefetches(prefetches, entry);
    }
}

vector<Addr> prefetch_filter(dcpt_table_entry entry, vector<Addr>& candidates)
{
    vector<Addr> prefetches;

    for (addr_it candidate = candidates.begin(); candidate != candidates.end(); ++candidate) {

        if (!in_cache(*candidate) && !in_mshr_queue(*candidate) && !in_flight.in(*candidate)) {
            prefetches.push_back(*candidate);
			in_flight.push(*candidate);
        }

        if (*candidate == entry.last_prefetch) {
            prefetches.clear();
		}
    }

    return prefetches;
}

vector<Addr> delta_correlation(dcpt_table_entry entry)
{
    vector<Addr> candidates;
	if (entry.delta_buffer.size() < 2)
		return candidates;

	// std::cerr << "DELTA_CORRELATE\n";
    deque<int64_t>::reverse_iterator rit = entry.delta_buffer.rbegin();

    Addr d1 = *(rit++);
    Addr d2 = *rit;

    Addr address = entry.last_addr;

	for (deq_it it = entry.delta_buffer.begin(); it+1 != entry.delta_buffer.end(); ) {
        Addr u = *(it++);
        Addr v = *(it++);

        if (u == d2 && v == d1) {
            for (; it != entry.delta_buffer.end(); ++it) {
                address += *it;

				// Make sure to not prefetch outside of physical memory
				if (address >= MAX_PHYS_MEM_ADDR)
					break;
				
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

	in_flight.erase(find(in_flight.begin(), in_flight.end(), addr), in_flight.end());
}
