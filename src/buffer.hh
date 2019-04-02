#pragma once
#include "interface.hh"
#include <deque>
#include <map>
#include <queue>
#include <vector>
#include <algorithm>


// ETS_IFL_DBS_DW
// 280_64_12_20 -> 1.09
// Previous best 100, 12, 15, *
#define ENTRY_TABLE_SIZE      80
#define DELTA_BUFFER_SIZE     10
#define DELTA_WIDTH           14
#define IN_FLIGHT_BUFFER_SIZE 64


// static const uint64_t buf_size = 32;
// static const uint64_t table_size = 256;

typedef std::deque<int64_t>::iterator deq_it;
typedef std::deque<int64_t>::reverse_iterator deq_rit;
typedef std::vector<Addr>::iterator addr_it;

struct circular_buffer {
	circular_buffer(size_t buffer_size) : buffer_size(buffer_size) {}
	
    std::deque<int64_t> buffer;

    void push(int64_t value);

	// For convenience only. Less typing in the main implementation
	// Mostly mapping deque buffer.
	deq_it begin() { return buffer.begin(); }
	deq_it end() { return buffer.end(); }
	deq_rit rbegin() { return buffer.rbegin(); }
	deq_rit rend() { return buffer.rend(); }
	std::deque<int64_t>::size_type size() { return buffer.size(); }
	deq_it erase(deq_it it) { return buffer.erase(it); }
	deq_it erase(deq_it first, deq_it last) { return buffer.erase(first, last); }

	// Convenience, not mapping deque buffer directly
	bool in(int64_t value) { return std::find(begin(), end(), value) != end(); }
	void erase(int64_t value) { erase(std::find(begin(), end(), value), end()); }

private:
	size_t buffer_size;
};

struct dcpt_table_entry {
	dcpt_table_entry()
		: last_addr(0),
		  last_prefetch(0),
		  delta_buffer(DELTA_BUFFER_SIZE) {}

    Addr last_addr;
    Addr last_prefetch;
    circular_buffer delta_buffer;
};

struct dcpt_table {
    std::map<Addr, dcpt_table_entry> table;
    std::queue<Addr> program_counters;

    dcpt_table_entry& insert(Addr);
	dcpt_table_entry& lookup(Addr, bool&);
};
