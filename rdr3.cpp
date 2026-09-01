#include <print>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;

std::ifstream save_file;
std::vector<std::uint8_t> save_data = {};

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

	// std::println("reading savefile..");
	// if(read_save(argv[1])) {
	// 	std::println("savefile read successfully!");
	// }

	// std::println("size (bytes): {}", save_data.size());
	return 0;
}
