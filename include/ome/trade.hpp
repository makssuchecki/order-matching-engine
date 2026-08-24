#pragma once

#include <cstdint>

namespace ome{

struct Trade {
    std::uint64_t buy_order_id;
    std::uint64_t sell_order_id;
    std::int64_t price;
    std::uint32_t quantity;
};
}