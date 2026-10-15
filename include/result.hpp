#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace nvme_gpu {

struct ReadResult {
  std::string requested_backend = "cpu";
  std::string observed_path = "file-to-host";
  std::string file;
  std::uint64_t file_size = 0;
  std::uint64_t bytes_read = 0;
  std::uint64_t block_size = 0;
  std::uint64_t chunks = 0;
  std::uint64_t checksum_sum64 = 0;
  std::uint64_t pattern_mismatches = 0;
  bool verification_performed = false;
  bool verification_ok = false;
  double storage_read_ms = 0.0;
  double processing_ms = 0.0;
  double end_to_end_ms = 0.0;
  double throughput_gib_s = 0.0;
};

void print_human(std::ostream& output, const ReadResult& result);
void print_json(std::ostream& output, const ReadResult& result);

}  // namespace nvme_gpu
