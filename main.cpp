/*
 * @Author: vrwboi && virwboi
 * @Date: 2025-08-01 08:51:25
 * @LastEditors: vrwboi && virwboi
 * @LastEditTime: 2025-08-04 11:23:54
 * @FilePath: \vector\main.cpp
 * @Description:
 *
 * Copyright (c) 2025 by ${git_name_email}, All Rights Reserved.
 */
#include "include/Vector.hpp"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <algorithm>

int main()
{
    using TestType = int; // 你也可以尝试用自定义复杂类型

    Vector<TestType> my_vector;
    std::vector<TestType> std_vector;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const int TEST_ROUNDS = 100000; // 大量操作

    for (int i = 0; i < TEST_ROUNDS; ++i)
    {
        int op = std::rand() % 6;
        switch (op)
        {
        case 0:
        { // push_back
            int val = std::rand();
            my_vector.push_back(val);
            std_vector.push_back(val);
            break;
        }
        case 1:
        { // insert at random position
            if (my_vector.size() == 0)
                break;
            int val = std::rand();
            size_t pos = std::rand() % my_vector.size();
            auto it_my = my_vector.begin() + pos;
            auto it_std = std_vector.begin() + pos;
            my_vector.insert(it_my, val);
            std_vector.insert(it_std, val);
            break;
        }
        case 2:
        { // erase at random position
            if (my_vector.size() == 0)
                break;
            size_t pos = std::rand() % my_vector.size();
            auto it_my = my_vector.begin() + pos;
            auto it_std = std_vector.begin() + pos;
            my_vector.erase(it_my);
            std_vector.erase(it_std);
            break;
        }
        case 3:
        { // resize smaller or bigger
            int new_size = std::rand() % (std::max<size_t>(my_vector.size(), 1) * 2);
            if (std::rand() % 2 == 0)
            { // resize with default
                my_vector.resize(new_size);
                std_vector.resize(new_size);
            }
            else
            { // resize with value
                int val = std::rand();
                my_vector.resize(new_size, val);
                std_vector.resize(new_size, val);
            }
            break;
        }
        case 4:
        { // assign with fill
            int n = std::rand() % 100;
            int val = std::rand();
            my_vector.assign(static_cast<size_t>(n), val);
            std_vector.assign(static_cast<size_t>(n), val);
            break;
        }
        case 5:
        { // assign with initializer list (simulate)
            int n = std::rand() % 20;
            std::vector<TestType> temp;
            for (int j = 0; j < n; ++j)
                temp.push_back(std::rand());
            my_vector.assign(temp.begin(), temp.end());
            std_vector.assign(temp.begin(), temp.end());
            break;
        }
        }

        // 验证两者大小相同
        assert(my_vector.size() == std_vector.size());

        // 验证每个元素相同
        for (size_t j = 0; j < my_vector.size(); ++j)
        {
            assert(my_vector[j] == std_vector[j]);
        }
    }

    std::cout << "no problem\n";
    return 0;
}