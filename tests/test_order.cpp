#include <gtest/gtest.h>
#include <type_traits>

#include "ome/order.hpp"

TEST(Order, IsTriviallyCopyable){
    static_assert(std::is_trivially_copyable_v<ome::Order>);
}
TEST(Order, ReportsSize) {
    std::cout << "sizeof(Order) = " << sizeof(ome::Order) << " bytes\n";
    SUCCEED();
}


