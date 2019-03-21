#include "buffer.hh"
#include "interface.hh"

#include <iostream>

using namespace std;

void circular_buffer::push(int64_t value)
{
// TODO: LRU
    if (buffer.size() == buffer_size)
        buffer.pop_front();

    buffer.push_back(value);
}

dcpt_table_entry& dcpt_table::insert(Addr pc)
{
    if (program_counters.size() > ENTRY_TABLE_SIZE) {
        Addr old_pc = program_counters.front();
        program_counters.pop();
        table.erase(old_pc);
    }

    program_counters.push(pc);
    return table[pc];
}

dcpt_table_entry& dcpt_table::lookup(Addr pc, bool& inserted)
{
    if (table.find(pc) == table.end()) {
		inserted = true;
        return insert(pc);
    } else {
		inserted = false;
        return table[pc];
    }
}
