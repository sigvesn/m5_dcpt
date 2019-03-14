#pragma once
#include "interface.hh"
#include <map>
#include <queue>

static const uint64_t buf_size = 32;
static const uint64_t table_size = 256;


struct circular_buffer {
    uint64_t index;
    int64_t buffer[buf_size];
};

struct dcpt_table_entry {
    // Addr pc;
    Addr last_addr;
    Addr last_prefetch;
    circular_buffer delta_pointer;
};

struct inFlight {
    circular_buffer inFlight_pointer;
};

struct dcpt_table {
	std::map<Addr, dcpt_table_entry> dcpt_table;
	std::queue<Addr> program_counters;

    dcpt_table_entry& lookup(Addr);
    dcpt_table_entry& insert(Addr);
};

void circular_buffer_set(circular_buffer dp, int64_t value);
int64_t circular_buffer_get(circular_buffer dp);
dcpt_table_entry* table_lookup(dcpt_table_entry* table, Addr pc);
