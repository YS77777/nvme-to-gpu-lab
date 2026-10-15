#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "file_reader.hpp"
#include "result.hpp"
#include "size_parser.hpp"

namespace {

void print_usage(std::ostream& output) {
  output
      << "Usage: nvme_gpu_lab --file PATH [options]\n\n"
      << "Stage 1 options:\n"
      << "  --backend cpu          CPU-only backend (the only v0.1 backend)\n"
      << "  --block-size SIZE      Read block size, e.g. 64K or 1MiB\n"
      << "  --max-bytes SIZE       Stop after SIZE bytes; 0 means the whole file\n"
      << "  --verify-pattern       Verify byte i equals i modulo 251\n"
      << "  --json                 Emit one JSON object instead of text\n"
      << "  --help                 Show this message\n";
}

std::string require_value(int& index, int argc, char** argv,
                          const std::string& option) {
  if (index + 1 >= argc) {
    throw std::invalid_argument(option + " requires a value");
  }
  ++index;
  return argv[index];
}

}  // namespace

int main(int argc, char** argv) {
  try {
    nvme_gpu::ReadOptions options;
    std::string backend = "cpu";
    bool json = false;

    for (int index = 1; index < argc; ++index) {
      const std::string argument = argv[index];
      if (argument == "--help" || argument == "-h") {
        print_usage(std::cout);
        return 0;
      }
      if (argument == "--file") {
        options.path = require_value(index, argc, argv, argument);
      } else if (argument == "--backend") {
        backend = require_value(index, argc, argv, argument);
      } else if (argument == "--block-size") {
        options.block_size = nvme_gpu::parse_size(
            require_value(index, argc, argv, argument));
      } else if (argument == "--max-bytes") {
        options.max_bytes = nvme_gpu::parse_size(
            require_value(index, argc, argv, argument));
      } else if (argument == "--verify-pattern") {
        options.verify_pattern = true;
      } else if (argument == "--json") {
        json = true;
      } else {
        throw std::invalid_argument("unknown argument: " + argument);
      }
    }

    if (backend != "cpu") {
      throw std::invalid_argument(
          "v0.1 supports only --backend cpu; pageable and pinned arrive in "
          "stage 2");
    }

    auto result = nvme_gpu::read_file_cpu(options);
    result.requested_backend = backend;
    if (json) {
      nvme_gpu::print_json(std::cout, result);
    } else {
      nvme_gpu::print_human(std::cout, result);
    }
    if (result.verification_performed && !result.verification_ok) {
      return 2;
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 1;
  }
}
