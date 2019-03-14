#include "buffer.hh"
#include "interface.hh"

void circular_buffer_set(circular_buffer dp, int64_t value)
{
    dp.buffer[dp.index] = value;
    dp.index = (dp.index + 1) % buf_size;
}

int64_t circular_buffer_get(circular_buffer dp)
{
    return dp.buffer[dp.index];
}

dcpt_table_entry* table_lookup(dcpt_table_entry* table, Addr pc)
{
    dcpt_table_entry* return_pc = table[0];
}
