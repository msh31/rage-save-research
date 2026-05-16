#include <cstdint>
#include <print>
#include <filesystem>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>

#include <openssl/evp.h>

namespace fs = std::filesystem;

/*
 * This is a simple test to see if the ps4 decryption logic can be used on PC saves
 *
 * (It does not)
 */

int main() {
//load file 
    fs::path save_path = "/home/unknown/Downloads/complete_save_game_file/";
    fs::path save_file = save_path / "SRDR30015";
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
    std::array<uint8_t, 32> key = {
        0x16, 0x85, 0xFF, 0xA3, 0x8D, 0x01, 0x0F, 0x0D, 0xFE, 0x66, 0x1C, 0xF9, 0xB5, 0x57, 0x2C, 0x50,
        0x0D, 0x80, 0x26, 0x48, 0xDB, 0x37, 0xB9, 0xED, 0x0F, 0x48, 0xC5, 0x73, 0x42, 0xC0, 0x22, 0xF5
    };

    auto enc_start_offset = 0x110;
    std::vector<uint8_t> out_buffer(file_size - enc_start_offset);
    
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(ctx == nullptr) {
        EVP_CIPHER_CTX_free(ctx);
        return 1;
    }
    EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, key.data(), nullptr);
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

    auto rsav_needle = std::array<uint8_t, 4>{'R','S','A','V'};
    auto it_rsav = std::search(out_buffer.begin(), out_buffer.end(), rsav_needle.begin(), rsav_needle.end());

    auto psin_needle = std::array<uint8_t, 4>{'P','S','I','N'};
    auto it_psin = std::search(out_buffer.begin(), out_buffer.end(), psin_needle.begin(), psin_needle.end());

    auto chks_needle = std::array<uint8_t, 4>{'C','H','K','S'};
    auto it_chks = std::search(out_buffer.begin(), out_buffer.end(), chks_needle.begin(), chks_needle.end());

    if (it_rsav != out_buffer.end()) { 
        int rsav_offset = std::distance(out_buffer.begin(), it_rsav);
        std::println("found rsav offet: {}", rsav_offset);
    } else { std::println("rsav offset not found"); }
    if (it_psin != out_buffer.end()) { 
        int psin_offset = std::distance(out_buffer.begin(), it_psin);
        std::println("found psin offet: {}", psin_offset);
    } else { std::println("psin offset not found"); }
    if (it_chks != out_buffer.end()) { 
        int chks_offset = std::distance(out_buffer.begin(), it_chks);
        std::println("found chks offet: {}", chks_offset);
    } else { std::println("chks offset not found"); }

    return 0;
}
