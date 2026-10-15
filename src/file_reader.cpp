#include "file_reader.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace nvme_gpu {
namespace {

using Clock = std::chrono::steady_clock;

double milliseconds(Clock::duration duration) {
  return std::chrono::duration<double, std::milli>(duration).count();
}

std::uint8_t expected_pattern_byte(std::uint64_t offset) {
  return static_cast<std::uint8_t>(offset % 251U);
}

}  // namespace

ReadResult read_file_cpu(const ReadOptions& options) {
  if (options.path.empty()) {
    throw std::invalid_argument("--file is required");
  }
  if (options.block_size == 0) {
    throw std::invalid_argument("block size must be greater than zero");
  }
  if (options.block_size >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::invalid_argument("block size is too large for this platform");
  }

  std::ifstream input(options.path, std::ios::binary | std::ios::ate);
  if (!input) {
    throw std::runtime_error("failed to open file: " + options.path.string());
  }

  const auto end_position = input.tellg();
  if (end_position < 0) {
    throw std::runtime_error("failed to determine file size: " +
                             options.path.string());
  }
  const auto file_size = static_cast<std::uint64_t>(end_position);
  input.seekg(0, std::ios::beg);
  if (!input) {
    throw std::runtime_error("failed to seek to beginning of file: " +
                             options.path.string());
  }

  const auto bytes_to_read = options.max_bytes == 0
                                 ? file_size
                                 : std::min(file_size, options.max_bytes);
  const auto allocation_size = static_cast<std::size_t>(
      std::min<std::uint64_t>(options.block_size,
                              std::max<std::uint64_t>(bytes_to_read, 1U)));
  std::vector<std::uint8_t> buffer(allocation_size);

  ReadResult result;
  result.file = options.path.string();
  result.file_size = file_size;
  result.block_size = options.block_size;
  result.verification_performed = options.verify_pattern;

  const auto total_start = Clock::now();
  while (result.bytes_read < bytes_to_read) {
    const auto remaining = bytes_to_read - result.bytes_read;
    const auto request_size = static_cast<std::size_t>(
        std::min<std::uint64_t>(remaining, options.block_size));

    const auto read_start = Clock::now();
    input.read(reinterpret_cast<char*>(buffer.data()),
               static_cast<std::streamsize>(request_size));
    const auto actual = input.gcount();
    result.storage_read_ms += milliseconds(Clock::now() - read_start);

    if (actual <= 0) {
      throw std::runtime_error("unexpected end of file after " +
                               std::to_string(result.bytes_read) + " bytes");
    }
    const auto actual_size = static_cast<std::size_t>(actual);

    const auto processing_start = Clock::now();
    for (std::size_t index = 0; index < actual_size; ++index) {
      const auto byte = buffer[index];
      result.checksum_sum64 += byte;
      if (options.verify_pattern &&
          byte != expected_pattern_byte(result.bytes_read + index)) {
        ++result.pattern_mismatches;
      }
    }
    result.processing_ms += milliseconds(Clock::now() - processing_start);
    result.bytes_read += actual_size;
    ++result.chunks;

    if (actual_size != request_size && result.bytes_read < bytes_to_read) {
      throw std::runtime_error("short read before requested byte range completed");
    }
  }

  result.end_to_end_ms = milliseconds(Clock::now() - total_start);
  result.verification_ok =
      options.verify_pattern && result.pattern_mismatches == 0;
  if (result.end_to_end_ms > 0.0) {
    constexpr double bytes_per_gib = 1024.0 * 1024.0 * 1024.0;
    result.throughput_gib_s =
        (static_cast<double>(result.bytes_read) / bytes_per_gib) /
        (result.end_to_end_ms / 1000.0);
  }
  return result;
}

}  // namespace nvme_gpu
