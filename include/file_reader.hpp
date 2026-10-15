#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

#include "result.hpp"

namespace nvme_gpu {

struct ReadOptions {
  std::filesystem::path path;
  std::uint64_t block_size = 1024U * 1024U;
  std::uint64_t max_bytes = 0;
  bool verify_pattern = false;
};

ReadResult read_file_cpu(const ReadOptions& options);

}  // namespace nvme_gpu
