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

void inc_lru(uint8_t lru)
{
    if (lru < 255)
        ++lru;
}

dcpt_table_entry* table_lookup(dcpt_table_entry* table, Addr pc)
{
    dcpt_table_entry* return_pc = table[0];
    bool found = false;

    for (int i = 0; i < table_size; ++i) {

        if (pc == table[i].pc) {
            return_pc = &table[i];
            found = true;
        } else {
            inc_lru(table[i].lru);
            if (!found && table[i].lru > return_pc.lru)
                return_pc = table[i];
        }
    }

    return_pc.lru = 0;
    return return_pc;
}
