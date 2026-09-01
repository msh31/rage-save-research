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
int header_size = 9; //not final
std::unordered_map<int, size_t> block_offsets = {};//k=count v=start offset

//basic info
void parse_block_zero() {
	auto offset = block_offsets[1];
	std::println("parsing block zero at offset {}", offset);

	uint32_t month = 0, day = 0, hours = 0, minutes = 0, dayweek = 0;
	std::memcpy(&month, &save_data[offset + header_size + 0x28], sizeof(month));
	std::memcpy(&day, &save_data[offset + header_size + 0x2c], sizeof(day));
	std::memcpy(&hours, &save_data[offset + header_size + 0x30], sizeof(hours));
	std::memcpy(&minutes, &save_data[offset + header_size + 0x34], sizeof(minutes));
	std::memcpy(&dayweek, &save_data[offset + header_size + 0x38], sizeof(dayweek));

	bool has_cheated = false;
	std::memcpy(&has_cheated, &save_data[offset + header_size + 0x3c], sizeof(has_cheated));

	std::println("in-game month: {}", month);
	std::println("in-game day: {}", day);
	std::println("in-game hours: {}", hours);
	std::println("in-game minutes: {}", minutes);
	std::println("in-game day of the week: {}", dayweek);

	std::println("has cheated?: {}", has_cheated);
}

//juicy
void parse_block_one() {
	auto offset = block_offsets[2];
	auto player_info = offset + 5 + 0x14;
	std::println("parsing block 1 at offset {}", offset);

	uint32_t money = 0, display_money = 0, max_wanted = 0, max_chaos = 0;
	uint8_t never_tired = 0, fast_reload = 0, fireproof = 0, keepweaponsbusted = 0, freehealthcare = 0, candriveby = 0, canbehassledgngs = 0;
	uint16_t maxhealth = 0, maxarmor = 0;
	float health = 0.0f, armor = 0.0f;

	std::memcpy(&money, &save_data[player_info + 0x08], sizeof(money));
	std::memcpy(&display_money, &save_data[player_info + 0x10], sizeof(display_money));

	std::memcpy(&never_tired, &save_data[player_info + 0x20], sizeof(never_tired));
	std::memcpy(&fast_reload, &save_data[player_info + 0x21], sizeof(fast_reload));
	std::memcpy(&fireproof, &save_data[player_info + 0x22], sizeof(fireproof));

	std::memcpy(&maxhealth, &save_data[player_info + 0x24], sizeof(maxhealth));
	std::memcpy(&maxarmor, &save_data[player_info + 0x26], sizeof(maxarmor));

	std::memcpy(&keepweaponsbusted, &save_data[player_info + 0x28], sizeof(keepweaponsbusted));
	std::memcpy(&freehealthcare, &save_data[player_info + 0x29], sizeof(freehealthcare));
	std::memcpy(&candriveby, &save_data[player_info + 0x2a], sizeof(candriveby));
	std::memcpy(&canbehassledgngs, &save_data[player_info + 0x2b], sizeof(canbehassledgngs));

	std::memcpy(&max_wanted, &save_data[player_info + 0x30], sizeof(max_wanted));
	std::memcpy(&max_chaos, &save_data[player_info + 0x34], sizeof(max_chaos));

	std::memcpy(&health, &save_data[player_info + 0x50], sizeof(health));
	std::memcpy(&armor, &save_data[player_info + 0x54], sizeof(armor));

	std::println("health: {}/{}", health, maxhealth);
	std::println("armor: {}/{}", armor, maxarmor);
	std::println("money: {} ({})", money, display_money);

	std::println("max wanted level: {}", max_wanted);
	std::println("max chaos: {}", max_chaos);

	std::println("never tired: {}", never_tired);
	std::println("fast reload: {}", fast_reload);
	std::println("fireproof: {}", fireproof);
	std::println("keep weapons after busted: {}", keepweaponsbusted);
	std::println("free health care: {}", freehealthcare);
	std::println("can drive-by: {}", candriveby);
	std::println("can be hassled by gangs: {}", canbehassledgngs);
}

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

    auto read_res = read_blocks(0x110);
    if(read_res == 0 || block_offsets.size() == 0) { // 0 = error
	std::println("failed to read blocks in savegame!");
	save_file.close();
	return false;
    }
    std::println("read all blocks in savefile!");

    parse_block_zero();
    parse_block_one();

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
