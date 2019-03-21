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
static circular_buffer in_flight(IN_FLIGHT_BUFFER_SIZE);

vector<Addr> prefetch_filter(dcpt_table_entry& entry, vector<Addr>& candidates);
vector<Addr> delta_correlation(dcpt_table_entry& entry);


void prefetch_init(void)
{
    /* Called before any calls to prefetch_access. */
    /* This is the place to initialize data structures. */

    //DPRINTF(HWPrefetch, "Initialized sequential-on-access prefetcher\n");
}

void issue_prefetches(vector<Addr>& prefetches, dcpt_table_entry& entry)
{
	// For all candidates, issue the prefetches as long as there's enough room
	// in the prefetch queue
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

		// Discard delta if it's wider than max delta width
		if (delta > pow(2, DELTA_WIDTH) - 1)
			return;

        entry.delta_buffer.push(delta);
		entry.last_addr = stat.mem_addr;

        vector<Addr> candidates = delta_correlation(entry);
        vector<Addr> prefetches = prefetch_filter(entry, candidates);

		issue_prefetches(prefetches, entry);
    }
}

vector<Addr> prefetch_filter(dcpt_table_entry& entry, vector<Addr>& candidates)
{
	// Purge candidates that are already in cache, in mshr queue or in the prefetch queue

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

vector<Addr> delta_correlation(dcpt_table_entry& entry)
{
	// Find matching deltas and add prefetch candidates with
	// subsequent address+delta addresses

    vector<Addr> candidates;

	// Do we have enough deltas to generate candidates?
	if (entry.delta_buffer.size() < 2)
		return candidates;

	// Delta values of the two last entries in the delta buffer
    deq_rit rit = entry.delta_buffer.rbegin();
    Addr d1 = *(rit++);
    Addr d2 = *rit;

	// Match on compatible deltas
	Addr u, v;
	deq_it it = entry.delta_buffer.begin();
	for (; it+1 != entry.delta_buffer.end(); ) {
        u = *(it++);
        v = *(it++);

        if (u == d2 && v == d1) break;
	}

	// Add all subsequent candidates
    Addr address = entry.last_addr;
	for (; it != entry.delta_buffer.end(); ++it) {
		address += *it;

		// Make sure to not prefetch outside of physical memory
		if (address >= MAX_PHYS_MEM_ADDR)
			break;
				
		candidates.push_back(address);
	}

    return candidates;
}

void prefetch_complete(Addr addr)
{
	// Remove from in_flight queue so that it doesn't match on complete prefetches
	in_flight.erase(addr);
}
