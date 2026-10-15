#include "result.hpp"

#include <iomanip>
#include <ostream>
#include <string_view>

namespace nvme_gpu {
namespace {

void print_json_string(std::ostream& output, std::string_view text) {
  output << '"';
  for (const char ch : text) {
    switch (ch) {
      case '"':
        output << "\\\"";
        break;
      case '\\':
        output << "\\\\";
        break;
      case '\n':
        output << "\\n";
        break;
      case '\r':
        output << "\\r";
        break;
      case '\t':
        output << "\\t";
        break;
      default:
        output << ch;
    }
  }
  output << '"';
}

const char* boolean(bool value) { return value ? "true" : "false"; }

}  // namespace

void print_human(std::ostream& output, const ReadResult& result) {
  output << std::fixed << std::setprecision(3);
  output << "backend: " << result.requested_backend << '\n'
         << "observed_path: " << result.observed_path << '\n'
         << "file: " << result.file << '\n'
         << "file_size: " << result.file_size << '\n'
         << "bytes_read: " << result.bytes_read << '\n'
         << "block_size: " << result.block_size << '\n'
         << "chunks: " << result.chunks << '\n'
         << "storage_read_ms: " << result.storage_read_ms << '\n'
         << "processing_ms: " << result.processing_ms << '\n'
         << "end_to_end_ms: " << result.end_to_end_ms << '\n'
         << "throughput_gib_s: " << result.throughput_gib_s << '\n'
         << "checksum_sum64: " << result.checksum_sum64 << '\n'
         << "verification_performed: "
         << boolean(result.verification_performed) << '\n'
         << "pattern_mismatches: " << result.pattern_mismatches << '\n'
         << "verification_ok: " << boolean(result.verification_ok) << '\n';
}

void print_json(std::ostream& output, const ReadResult& result) {
  output << std::fixed << std::setprecision(6) << '{';
  output << "\"requested_backend\":";
  print_json_string(output, result.requested_backend);
  output << ",\"observed_path\":";
  print_json_string(output, result.observed_path);
  output << ",\"file\":";
  print_json_string(output, result.file);
  output << ",\"file_size\":" << result.file_size
         << ",\"bytes_read\":" << result.bytes_read
         << ",\"block_size\":" << result.block_size
         << ",\"chunks\":" << result.chunks
         << ",\"storage_read_ms\":" << result.storage_read_ms
         << ",\"processing_ms\":" << result.processing_ms
         << ",\"end_to_end_ms\":" << result.end_to_end_ms
         << ",\"throughput_gib_s\":" << result.throughput_gib_s
         << ",\"checksum_sum64\":" << result.checksum_sum64
         << ",\"verification_performed\":"
         << boolean(result.verification_performed)
         << ",\"pattern_mismatches\":" << result.pattern_mismatches
         << ",\"verification_ok\":" << boolean(result.verification_ok)
         << "}\n";
}

}  // namespace nvme_gpu
