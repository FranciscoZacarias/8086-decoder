#define MEMORY_ACCESS_MASK 0xffff

struct Memory {
	u8 bytes[65536];
};
// NOTE(fz): This is a compile-time assertion
static_assert((ArrayCount(Memory::bytes) - 1) == MEMORY_ACCESS_MASK, "Memory size doesn't match access mask");

struct SegmentedAccess {
	u32 base;
	u32 offset;
};

u32 load_file_to_memory(Memory* memory, char* filename, u32 offset);
u32 get_absolute_address(SegmentedAccess access, u16 additional_offset = 0);
u8  read_memory(Memory* memory, u32 absolute_address);

