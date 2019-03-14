#pragma once
#include "interface.hh"
#include <map>

static const uint64_t buf_size = 4096;
static const uint64_t table_size = 4096;

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

void circular_buffer_set(circular_buffer dp, int64_t value);
int64_t circular_buffer_get(circular_buffer dp);
dcpt_table_entry* table_lookup(dcpt_table_entry* table, Addr pc);
