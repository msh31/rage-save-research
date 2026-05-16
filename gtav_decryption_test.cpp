#include <cstdint>
#include <print>
#include <filesystem>
#include <fstream>
#include <vector>
#include <array>

#include <openssl/evp.h>

namespace fs = std::filesystem;

/*
 * This is a simple test to decrypt gta v pc saves 
 *
 */

int main() {
//load file 
    fs::path save_path = "/mnt/games/SteamLibrary/steamapps/compatdata/3240220/pfx/drive_c/users/steamuser/Documents/Rockstar Games/GTAV Enhanced/Profiles/66BBE120/";
    fs::path save_file = save_path / "SGTA50000";
    std::ifstream file(save_file, std::ios::binary);
    if(!file.is_open()) {
        std::println("[error] failed to open save file {}", save_file.filename().string());
        return 1; 
    }

    std::vector<uint8_t> save_data;
    auto file_size = fs::file_size(save_file);
    std::println("save size: {} bytes", file_size);

//read file
    save_data.resize(file_size);
    file.read((char*)save_data.data(), file_size);
    if(file.gcount() == (std::streamsize)file_size) std::println("[success] fully read {}!", save_file.filename().string());

//aes decryption
    std::array<uint8_t, 16> key = {
        0x46, 0xed, 0x8d, 0x3f, 0x94, 0x35, 0xe4, 0xec,
        0x12, 0x2c, 0xb2, 0xe2, 0xaf, 0x97, 0xc5, 0x7e
        // 0x4c, 0x5a, 0x8c, 0x30, 0x92, 0xc7, 0x84, 0x4e,
        // 0x11, 0xc6, 0x86, 0xff, 0x41, 0xdf, 0x41, 0x0f
    };

    auto enc_start_offset = 0x104;
    std::vector<uint8_t> out_buffer(file_size - enc_start_offset);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(ctx == nullptr) {
        EVP_CIPHER_CTX_free(ctx);
        return 1;
    }
    EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key.data(), nullptr);
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    int out_len = 0;
    EVP_DecryptUpdate(ctx, out_buffer.data(), &out_len, save_data.data() + enc_start_offset, (file_size - enc_start_offset) & ~0xF);

//offset search 
    std::println("\nsave_data");
    for(uint32_t i {}; i < 512; i++) {
        std::print("{:02x} ", save_data[i]);
        if(i % 16 == 15) std::println();
    }
    std::println("\n\nout_buffer");
    for(uint32_t i {}; i < 512; i++) {
        std::print("{:02x} ", out_buffer[i]);
        if(i % 16 == 15) std::println("");
    }
    std::println("");

    auto sgv2_needle = std::array<uint8_t, 4>{'S','G','V','2'};
    auto it_sgv2 = std::search(out_buffer.begin(), out_buffer.end(), sgv2_needle.begin(), sgv2_needle.end());

    auto sgvz_needle = std::array<uint8_t, 4>{'S','G','V','Z'};
    auto it_sgvz = std::search(out_buffer.begin(), out_buffer.end(), sgvz_needle.begin(), sgvz_needle.end());

    if (it_sgv2 != out_buffer.end()) { 
        int offset = std::distance(out_buffer.begin(), it_sgv2);
        std::println("found sgv2 offet: {}", offset);
    } else { std::println("sgv2 offset not found"); }
    if (it_sgvz != out_buffer.end()) { 
        int offset = std::distance(out_buffer.begin(), it_sgvz);
        std::println("found sgvz offet: {}", offset);
    } else { std::println("sgvz offset not found"); }


    return 0;
}
