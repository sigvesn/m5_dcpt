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

dcpt_table_entry& dcpt_table::insert(Addr pc)
{
    if (program_counters.size() > table_size) {
        Addr old_pc = program_counters.front();
        program_counters.pop();
        dcpt_table.erase(old_pc);
    }

    program_counters.push(pc);
    return dcpt_table[pc];
}

dcpt_table_entry& dcpt_table::lookup(Addr pc)
{
    if (dcpt_table.find(pc) == m.end()) {
        return insert(pc);
    } else {
        return dcpt_table[pc];
    }
}
