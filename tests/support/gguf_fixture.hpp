#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace test::gguf {

constexpr std::uint32_t kMagic = 0x46554747U;
constexpr std::uint32_t kVersion = 3;
constexpr std::uint32_t kTypeF32 = 0;
constexpr std::uint32_t kTypeF16 = 1;
constexpr std::uint32_t kTypeQ40 = 2;
constexpr std::uint32_t kTypeQ41 = 3;
constexpr std::uint32_t kTypeQ50 = 6;
constexpr std::uint32_t kTypeQ51 = 7;
constexpr std::uint32_t kTypeQ80 = 8;
constexpr std::uint32_t kTypeQ81 = 9;
constexpr std::uint32_t kTypeQ2K = 10;
constexpr std::uint32_t kTypeQ3K = 11;
constexpr std::uint32_t kTypeQ4K = 12;
constexpr std::uint32_t kTypeQ5K = 13;
constexpr std::uint32_t kTypeQ6K = 14;
constexpr std::uint32_t kTypeQ8K = 15;
constexpr std::uint32_t kValueTypeString = 8;
constexpr std::uint32_t kValueTypeArray = 9;
constexpr std::uint32_t kValueTypeUint32 = 4;

inline void append_u32(std::vector<std::byte>& bytes, std::uint32_t value) {
  for (std::size_t index = 0; index < 4; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xffU));
  }
}

inline void append_u64(std::vector<std::byte>& bytes, std::uint64_t value) {
  for (std::size_t index = 0; index < 8; ++index) {
    bytes.push_back(static_cast<std::byte>((value >> (index * 8U)) & 0xffU));
  }
}

inline void append_string(std::vector<std::byte>& bytes, std::string_view value) {
  append_u64(bytes, value.size());
  for (const auto character : value) {
    bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
  }
}

inline void append_tensor(std::vector<std::byte>& bytes, std::string_view name,
                          const std::vector<std::uint64_t>& dimensions,
                          std::uint32_t type, std::uint64_t offset) {
  append_string(bytes, name);
  append_u32(bytes, static_cast<std::uint32_t>(dimensions.size()));
  for (const auto dimension : dimensions) {
    append_u64(bytes, dimension);
  }
  append_u32(bytes, type);
  append_u64(bytes, offset);
}

inline void append_padding(std::vector<std::byte>& bytes,
                           std::uint64_t alignment = 32) {
  while (bytes.size() % alignment != 0U) {
    bytes.push_back(std::byte{0});
  }
}

inline std::vector<std::byte> tensor_file(
    std::vector<std::uint64_t> dimensions, std::uint32_t type,
    std::uint64_t payload_bytes, std::uint64_t offset = 0,
    bool append_payload = true) {
  std::vector<std::byte> bytes;
  append_u32(bytes, kMagic);
  append_u32(bytes, kVersion);
  append_u64(bytes, 1);
  append_u64(bytes, 0);
  append_tensor(bytes, "weight", dimensions, type, offset);
  append_padding(bytes);
  if (append_payload) {
    for (std::uint64_t index = 0; index < payload_bytes; ++index) {
      bytes.push_back(static_cast<std::byte>(index));
    }
  }
  return bytes;
}

inline std::vector<std::byte> valid_f32_file(
    std::vector<std::uint64_t> dimensions = {2}, std::uint64_t offset = 0,
    bool append_payload = true) {
  const auto payload_bytes = dimensions.empty() ? 0U : dimensions.front() * 4U;
  return tensor_file(std::move(dimensions), kTypeF32, payload_bytes, offset,
                     append_payload);
}

inline std::vector<std::byte> truncated_metadata_string_file() {
  std::vector<std::byte> bytes;
  append_u32(bytes, kMagic);
  append_u32(bytes, kVersion);
  append_u64(bytes, 0);
  append_u64(bytes, 1);
  append_string(bytes, "k");
  append_u32(bytes, kValueTypeString);
  append_u64(bytes, 64);
  return bytes;
}

inline std::vector<std::byte> truncated_metadata_array_file() {
  std::vector<std::byte> bytes;
  append_u32(bytes, kMagic);
  append_u32(bytes, kVersion);
  append_u64(bytes, 0);
  append_u64(bytes, 1);
  append_string(bytes, "k");
  append_u32(bytes, kValueTypeArray);
  append_u32(bytes, kValueTypeUint32);
  append_u64(bytes, 2);
  return bytes;
}

inline std::filesystem::path write_file(std::string_view stem,
                                        const std::vector<std::byte>& bytes) {
  const auto nonce = std::chrono::steady_clock::now().time_since_epoch().count();
  const auto path = std::filesystem::temp_directory_path() /
                    ("bruce-gguf-" + std::string(stem) + "-" +
                     std::to_string(nonce) + ".gguf");
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  output.write(reinterpret_cast<const char*>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
  output.close();
  return path;
}

inline std::vector<std::byte> read_file(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  std::vector<std::byte> bytes;
  char character = 0;
  while (input.get(character)) {
    bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
  }
  return bytes;
}

}  // namespace test::gguf
