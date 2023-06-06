#ifndef _MEMORY_H
#define _MEMORY_H

#define MEMORY_ACCESS_MASK 0xfffff

struct Memory {
	u8 bytes[1024*1024];
};
static_assert((ArrayCount(memory.bytes) - 1) == MEMORY_ACCESS_MASK, "Memory size doesn't match access mask");


struct SegmentedAccess {
	u32 base;
	u32 offset;
}

u32 load_file(Memory* memory, char* filename, u32 offset);

#endif //_MEMORY_H
