#include <cstdint>
#include <print>
#include <filesystem>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>

#include <openssl/evp.h>

namespace fs = std::filesystem;

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
        0xb7, 0x62, 0xdf, 0xb6, 0xe2, 0xb2, 0xc6, 0xde,
        0xaf, 0x72, 0x2a, 0x32, 0xd2, 0xfb, 0x6f, 0x0c,
        0x98, 0xa3, 0x21, 0x74, 0x62, 0xc9, 0xc4, 0xed,
        0xad, 0xaa, 0x2e, 0xd0, 0xdd, 0xf9, 0x2f, 0x10
    };

    auto enc_start_offset = 0x130;
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

    auto sgv2_needle = std::array<uint8_t, 4>{'S','G','V','2'};
    auto it_sgv2 = std::search(out_buffer.begin(), out_buffer.end(), sgv2_needle.begin(), sgv2_needle.end());

    auto sgvz_needle = std::array<uint8_t, 4>{'S','G','V','Z'};
    auto it_sgvz = std::search(out_buffer.begin(), out_buffer.end(), sgvz_needle.begin(), sgvz_needle.end());

    auto rsav_needle = std::array<uint8_t, 4>{'R','S','A','V'};
    auto it_rsav = std::search(out_buffer.begin(), out_buffer.end(), rsav_needle.begin(), rsav_needle.end());

    auto chks_needle = std::array<uint8_t, 4>{'C','H','K','S'};
    auto it_chks = std::search(out_buffer.begin(), out_buffer.end(), chks_needle.begin(), chks_needle.end());

    if (it_rsav != out_buffer.end()) { 
        int rsav_offset = std::distance(out_buffer.begin(), it_rsav);
        std::println("found rsav offet: {}", rsav_offset);
    } else { std::println("rsav offset not found"); }
    if (it_chks != out_buffer.end()) { 
        int chks_offset = std::distance(out_buffer.begin(), it_chks);
        std::println("found chks offet: {}", chks_offset);
    } else { std::println("chks offset not found"); }
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
