/* 
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#include "join-iterator.h"
#include <map>
#include <vector>
#include <iostream>

namespace detail{
    const int& extract_value(const std::map<int, float>::const_iterator& iterator, detail::join_iterator_tag) {
        return iterator->first;
    }
}

int main() {
    std::vector<int> i;
    i.emplace_back(1);
    i.emplace_back(2);
    i.emplace_back(3);
    i.emplace_back(4);
    std::map<int, float> m;
    m.emplace(15, 34);
    m.emplace(16, 34);
    m.emplace(17, 34);
    std::vector<int> i1;
    i1.emplace_back(101);
    i1.emplace_back(102);
    join_container_t join_c(type_holder_t<int>{}, i, m, i1);
    join_container_t join_c1 = join_c;
    for (auto&& v : join_c) {
        std::cout << v << std::endl;
    }
    return 0;
} 