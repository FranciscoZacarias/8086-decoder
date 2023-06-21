
u32 load_file_to_memory(Memory* memory, char* filename, u32 offset)
{
	u32 bytesLoaded = 0;
	
	if (offset < ArrayCount(memory->bytes)) {
		FILE* fp = {};
		errno_t err = fopen_s(&fp, filename, "rb");
		if (err == 0) {
			size_t memorySize = ArrayCount(memory->bytes);
			bytesLoaded = fread(memory->bytes + offset, 1, memorySize - offset, fp);
			fclose(fp);
		} else {
			fprintf(stderr, "ERROR %d: Unable to open file %s\n", err, filename);
		}
	}
	
	return bytesLoaded;
}

u32 get_absolute_address(SegmentedAccess access, u16 additional_offset) {
	return (((u32)access.base << 4) + ((u32)(access.offset + additional_offset))) & MEMORY_ACCESS_MASK;
}

u8 read_memory(Memory* memory, u32 absolute_address) {
	assert(absolute_address < ArrayCount(memory->bytes));
	return memory->bytes[absolute_address];
}