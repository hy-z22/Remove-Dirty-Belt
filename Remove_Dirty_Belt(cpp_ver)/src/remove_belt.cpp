#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>

using namespace std;

vector<uint16_t> remove_belt(const vector<uint16_t>& initial_imageData, int width, int height) {
    // 将一维数组转换为二维矩阵 (列优先)
    vector<vector<uint16_t>> imageData(height, vector<uint16_t>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            imageData[y][x] = initial_imageData[y * width + x];
        }
    }

    // 区分物体和背景
    vector<vector<bool>> is_object(height, vector<bool>(width, false));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            is_object[y][x] = (imageData[y][x] != 65535);
        }
    }

    // 找出可能的边界
    vector<vector<bool>> is_boundary(height, vector<bool>(width, false));
    for (int y = 0; y < height; y++) {
        for (int x = 1; x < width - 1; x++) {
            is_boundary[y][x] = is_object[y][x] && (!is_object[y][x-1] || !is_object[y][x+1]);
        }
    }

    // 计算每行的边界数量
    vector<int> boundary_num(height, 0);
    for (int y = 0; y < height; y++) {
        boundary_num[y] = count(is_boundary[y].begin(), is_boundary[y].end(), true);
    }

    // 计算左右极限位置
    vector<pair<int, int>> boundaries(height, {0, 0});
    for (int y = 0; y < height; y++) {
        int left = width, right = -1;
        for (int x = 0; x < width; x++) {
            if (is_object[y][x]) {
                if (x < left) left = x;
                if (x > right) right = x;
            }
        }
        boundaries[y] = {left != width ? left : -1, right != -1 ? right : -1};
    }

    // 计算边界位置的一阶导数
    vector<double> left_diff(height, 0), right_diff(height, 0);
    for (int y = 1; y < height; y++) {
        left_diff[y] = (boundaries[y].first != -1) * (boundaries[y-1].first != -1) * (boundaries[y].first - boundaries[y-1].first);
        right_diff[y] = (boundaries[y].second != -1) * (boundaries[y-1].second != -1) * (boundaries[y].second - boundaries[y-1].second);
    }

    vector<double> left_first_order(height, 0), right_first_order(height, 0);
    vector<double> first_order(height, 0);
    for (int y = 1; y < height - 1; y++) {
        left_first_order[y] = (left_diff[y] + left_diff[y+1]) / 2.0;
        right_first_order[y] = (right_diff[y] + right_diff[y+1]) / 2.0;
        first_order[y] = abs(left_first_order[y]) + abs(right_first_order[y]);
    }

    // 计算边界数与一阶导数的乘积
    vector<double> multiply(height, 0);
    for (int y = 0; y < height; y++) {
        multiply[y] = boundary_num[y] * first_order[y];
    }

    // 找到乘积最大的位置 (定位脏带)
    int pin = distance(multiply.begin(), max_element(multiply.begin(), multiply.end()));

    // 寻找脏带的上下边界
    int top_edge = -1, btm_edge = -1;

    // 寻找上边界
    for (int y = pin; y >= 15; y--) {
        bool condition1 = true;
        for (int i = y-15; i < y; i++) {
            if (i >= 0 && multiply[i] >= 100) {
                condition1 = false;
                break;
            }
        }

        bool condition2a = true;
        for (int i = y-7; i < y; i++) {
            if (i >= 0 && boundary_num[i] != 2) {
                condition2a = false;
                break;
            }
        }

        bool condition2b = true;
        for (int i = y-5; i < y; i++) {
            if (i >= 0 && multiply[i] >= 100) {
                condition2b = false;
                break;
            }
        }

        if (condition1 || (condition2a && condition2b)) {
            top_edge = y;
            break;
        }
    }

    // 寻找下边界
    for (int y = pin; y < height - 15; y++) {
        bool condition1 = true;
        for (int i = y+1; i <= y+15; i++) {
            if (i < height && multiply[i] >= 100) {
                condition1 = false;
                break;
            }
        }

        bool condition2a = true;
        for (int i = y+1; i <= y+7; i++) {
            if (i < height && boundary_num[i] != 2) {
                condition2a = false;
                break;
            }
        }

        bool condition2b = true;
        for (int i = y+1; i <= y+5; i++) {
            if (i < height && multiply[i] >= 100) {
                condition2b = false;
                break;
            }
        }

        if (condition1 || (condition2a && condition2b)) {
            btm_edge = y;
            break;
        }
    }

    // 消除脏带
    if (top_edge != -1 && btm_edge != -1) {
        // 既找到上边界又找到下边界
        int left = min(boundaries[btm_edge+3].first, boundaries[top_edge-3].first);
        int right = max(boundaries[btm_edge+3].second, boundaries[top_edge-3].second);

        for (int y = top_edge-2; y <= btm_edge+2; y++) {
            for (int x = 0; x <= left; x++) {
                imageData[y][x] = 65535;
            }
            for (int x = right; x < width; x++) {
                imageData[y][x] = 65535;
            }
        }
    } else if (top_edge == -1 && btm_edge != -1) {
        // 只找到下边界
        int left = boundaries[btm_edge+3].first;
        int right = boundaries[btm_edge+3].second;
        for (int y = 0; y <= btm_edge+2; y++) {
            for (int x = 0; x <= left; x++) {
                imageData[y][x] = 65535;
            }
            for (int x = right; x < width; x++) {
                imageData[y][x] = 65535;
            }
        }
    } else if (top_edge != -1 && btm_edge == -1) {
        // 只找到上边界
        int left = boundaries[top_edge-3].first;
        int right = boundaries[top_edge-3].second;
        for (int y = top_edge-2; y < height; y++) {
            for (int x = 0; x <= left; x++) {
                imageData[y][x] = 65535;
            }
            for (int x = right; x < width; x++) {
                imageData[y][x] = 65535;
            }
        }
    }

    // 将处理后的二维矩阵转换回一维数组
    vector<uint16_t> processed_imageData(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            processed_imageData[y * width + x] = imageData[y][x];
        }
    }

    return processed_imageData;
}