#include "size_parser.hpp"

#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>
#include <string>

namespace nvme_gpu {

std::uint64_t parse_size(std::string_view text) {
  if (text.empty()) {
    throw std::invalid_argument("size must not be empty");
  }

  std::string value{text};
  value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char ch) {
                return std::isspace(ch) != 0;
              }),
              value.end());
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) {
                   return static_cast<char>(std::tolower(ch));
                 });

  const auto number_end = value.find_first_not_of("0123456789");
  const auto number_text = value.substr(0, number_end);
  const auto suffix =
      number_end == std::string::npos ? std::string{} : value.substr(number_end);
  if (number_text.empty()) {
    throw std::invalid_argument("size must start with a non-negative integer: " +
                                std::string{text});
  }

  std::uint64_t multiplier = 1;
  if (suffix.empty() || suffix == "b") {
    multiplier = 1;
  } else if (suffix == "k" || suffix == "kb" || suffix == "kib") {
    multiplier = 1024ULL;
  } else if (suffix == "m" || suffix == "mb" || suffix == "mib") {
    multiplier = 1024ULL * 1024ULL;
  } else if (suffix == "g" || suffix == "gb" || suffix == "gib") {
    multiplier = 1024ULL * 1024ULL * 1024ULL;
  } else {
    throw std::invalid_argument("unsupported size suffix: " + suffix);
  }

  std::size_t consumed = 0;
  const auto number = std::stoull(number_text, &consumed, 10);
  if (consumed != number_text.size()) {
    throw std::invalid_argument("invalid size: " + std::string{text});
  }
  if (number > std::numeric_limits<std::uint64_t>::max() / multiplier) {
    throw std::overflow_error("size is too large: " + std::string{text});
  }
  return number * multiplier;
}

}  // namespace nvme_gpu
