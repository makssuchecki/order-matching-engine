#include <cstdint>
#pragma once
namespace ome 
{
enum class Side : std::uint8_t {
    Buy,
    Sell
};

struct Order
{
    std::uint64_t id;
    std::int64_t price;
    std::uint32_t quantity;
    Side side;
    std::uint64_t timestamp;
    Order* next;
};
}
