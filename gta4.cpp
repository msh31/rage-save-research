// source: https://gtamods.com/wiki/Saves_(GTA_4)

#include <print>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;

std::ifstream save_file;
std::vector<std::uint8_t> save_data = {};

constexpr uint8_t block_signature[5] = { 'B', 'L', 'O', 'C', 'K' }; 
constexpr uint8_t end_signature[4] = { 'E', 'N', 'D', 0x00 };

int block_count = 0;
std::unordered_map<int, size_t> block_offsets = {};//k=count v=start offset

size_t read_blocks(size_t offset) {
	if (offset + 8 > save_data.size()) {
	    std::println("{} out of bounds, bailing!", offset);
	    return 0;
	}
	std::println("found offset: {}", offset);

	auto block_res = std::memcmp(&save_data[offset], block_signature, 5);
	auto end_res = std::memcmp(&save_data[offset], end_signature, 4);

	if (block_res != 0 && end_res != 0) {
		auto end_res_checksum = std::memcmp(&save_data[offset + 4], end_signature, 4);
		if (end_res_checksum == 0) {
			std::println("reached END\\0 block (after checksum)!");
			return offset + 4;
		}
		std::println("{} does not match a block or end signature, bailing!", offset);
		return 0;
	}
	if (end_res == 0) {
		std::println("reached END\\0 block!");
		return offset;
	}

	block_count += 1;
	block_offsets.insert({block_count, offset});
	uint32_t size = save_data[offset+5] | (save_data[offset+6] << 8) | (save_data[offset+7] << 16) | (save_data[offset+8] << 24);
	return read_blocks(offset + size);
}

bool read_save( fs::path path ) {
    save_file.open( path, std::ios::binary );
    if ( !save_file.is_open( ) ) {
	std::println( "Failed to open savegame!" );
        return false;
    }

    save_data = std::vector<uint8_t>( std::istreambuf_iterator<char>( save_file ), { } );
    if ( save_data.empty( ) ) {
	std::println( "Failed to load data from savegame!" );
        save_file.close( );
        return false;
    }

    if(read_blocks(0x110) == 0) { //error
	std::println("failed to read blocks in savegame!");
	save_file.close();
	return false;
    }

    std::println("read all blocks in savefile!");

    save_file.close();
    return true;
}

auto main(int argc, char** argv) -> int {
	if(argc <= 1) {
		std::println("too few arguments! use the path to your save as the argument");
		return 1;
	}

	if(argc > 2) {
		std::println("too many arguments only specify the path!");
		return 1;
	}

	auto input = argv[1];
	std::println("input: {}", input);

	if(!fs::is_regular_file(argv[1])) {
		std::println("this is not a regular file..");
		return 1;
	}

	std::println("reading savefile..");
	if(read_save(argv[1])) {
		std::println("savefile read successfully!");
	}

	std::println("size (bytes): {}", save_data.size());
	std::println("blocks found: {}", block_count);
	return 0;
}
