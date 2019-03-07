#include "interface.hh"

static const uint64_t buf_size = 4096;
static const uint64_t table_size = 4096;

struct circular_buffer {
    uint64_t index;
    int64_t buffer[buf_size];
};

void circular_buffer_set(circular_buffer dp, int64_t value)
{
    dp.buffer[dp.index] = value;
    dp.index = (dp.index + 1) % buf_size;
}

int64_t circular_buffer_get(circular_buffer dp)
{
    return dp.buffer[dp.index];
}

struct dcpt_table_entry {
    Addr pc;
    Addr last_addr;
    Addr last_prefetch;
    circular_buffer delta_pointer;
};

struct dcpt_table {
    dcpt_table_entry table[table_size];
};

struct inFlight {
    circular_buffer inFlight_pointer;
};
