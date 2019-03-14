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
    int64_t get();
};

struct dcpt_table_entry {
    Addr last_addr;
    Addr last_prefetch;
    circular_buffer delta_buffer;
};

struct dcpt_table {
    std::map<Addr, dcpt_table_entry> dcpt_table;
    std::queue<Addr> program_counters;

    dcpt_table_entry& lookup(Addr a);
    dcpt_table_entry& insert(Addr a);
};
