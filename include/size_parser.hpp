#pragma once

#include <cstdint>
#include <string_view>

namespace nvme_gpu {

std::uint64_t parse_size(std::string_view text);

}  // namespace nvme_gpu
