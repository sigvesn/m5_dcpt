#pragma once
#include "interface.hh"
#include <deque>
#include <map>
#include <queue>

static const uint64_t buf_size = 32;
static const uint64_t table_size = 256;

struct circular_buffer {
    std::deque<int64_t> buffer;

    void push(int64_t value);
};

struct dcpt_table_entry {
	dcpt_table_entry() {
		last_addr = 0;
		last_prefetch = 0;
	}
    Addr last_addr;
    Addr last_prefetch;
    circular_buffer delta_buffer;
};

struct dcpt_table {
    std::map<Addr, dcpt_table_entry> table;
    std::queue<Addr> program_counters;

    dcpt_table_entry& insert(Addr, Addr);
    dcpt_table_entry& lookup(Addr, Addr, bool&);
};
