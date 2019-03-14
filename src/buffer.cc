#include "buffer.hh"
#include "interface.hh"

using namespace std;

void circular_buffer::push(int64_t value)
{
    if (buffer.size() == buf_size)
        buffer.pop_front();

    buffer.push_back(value);
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
    if (dcpt_table.find(pc) == dcpt_table.end()) {
        return insert(pc);
    } else {
        return dcpt_table[pc];
    }
}
