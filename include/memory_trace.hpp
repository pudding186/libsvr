#pragma once
#include "net_data.hpp"
#include "rb_tree.h"

namespace SMemory
{
    extern DataArray<MemTraceInfo*> get_mem_trace_info(void);
}
