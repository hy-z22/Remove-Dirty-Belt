#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

using namespace std;

void remove_belt(uint16_t* imageData, int width, int height) {
    // 分配必要数组
    int* boundary_num = new int[height]{};
    int (*boundaries)[2] = new int[height][2];
    double* left_diff = new double[height]{};
    double* right_diff = new double[height]{};
    double* first_order = new double[height]{};
    double* multiply = new double[height]{};

    // 计算 boundary_num, boundaries
    for (int y = 0; y < height; y++) {
        int left = width, right = -1, boundary_counts = 0;
        int row_offset = y * width;
        for (int x = 0; x < width; x++) {
            int idx = row_offset + x;

            if (imageData[idx] != 65535) {
                if (x < left) left = x;
                if (x > right) right = x;
                if ((x != 0) && (x != width - 1) && ((imageData[idx - 1] == 65535) || (imageData[idx + 1] == 65535))) {
                    boundary_counts++;
                }
            }
        }
        boundary_num[y] = boundary_counts;
        boundaries[y][0] = (left != width) ? left : -1;
        boundaries[y][1] = (right != -1) ? right : -1;
    }
    
    // 计算一阶导数
    for (int y = 1; y < height; y++) {
        if (boundaries[y][0] != -1 && boundaries[y - 1][0] != -1)
            left_diff[y] = boundaries[y][0] - boundaries[y - 1][0];
        if (boundaries[y][1] != -1 && boundaries[y - 1][1] != -1)
            right_diff[y] = boundaries[y][1] - boundaries[y - 1][1];
    }

    for (int y = 1; y < height - 1; y++) {
        double l_avg = (left_diff[y] + left_diff[y + 1]) * 0.5;
        double r_avg = (right_diff[y] + right_diff[y + 1]) * 0.5;
        first_order[y] = fabs(l_avg) + fabs(r_avg);
    }

    // 计算 multiply 并定位脏带
    int pin = -1;
    double max_val = -1;
    for (int y = 0; y < height; y++) {
        multiply[y] = boundary_num[y] * first_order[y];
        if (multiply[y] > max_val) {
            max_val = multiply[y];
            pin = y;
        }
    }

    // 定位上下边界
    int top_edge = -1, btm_edge = -1;
    for (int y = pin; y >= 15; y--) {
        bool cond1 = true, cond2a = true, cond2b = true;
        for (int i = y - 15; i < y && i >= 0; i++) if (multiply[i] >= 100) cond1 = false;
        for (int i = y - 7; i < y && i >= 0; i++) if (boundary_num[i] != 2) cond2a = false;
        for (int i = y - 5; i < y && i >= 0; i++) if (multiply[i] >= 100) cond2b = false;
        if (cond1 || (cond2a && cond2b)) {
            top_edge = y;
            break;
        }
    }

    for (int y = pin; y < height - 15; y++) {
        bool cond1 = true, cond2a = true, cond2b = true;
        for (int i = y + 1; i <= y + 15 && i < height; i++) if (multiply[i] >= 100) cond1 = false;
        for (int i = y + 1; i <= y + 7 && i < height; i++) if (boundary_num[i] != 2) cond2a = false;
        for (int i = y + 1; i <= y + 5 && i < height; i++) if (multiply[i] >= 100) cond2b = false;
        if (cond1 || (cond2a && cond2b)) {
            btm_edge = y;
            break;
        }
    }

    // 消除脏带
    auto clip = [&](int y_from, int y_to, int left, int right) {
        for (int y = y_from; y <= y_to && y < height; y++) {
            int row_offset = y * width;
            for (int x = 0; x <= left && x < width; x++) imageData[row_offset + x] = 65535;
            for (int x = right; x < width; x++) imageData[row_offset + x] = 65535;
        }
    };

    if (top_edge != -1 && btm_edge != -1 && top_edge >= 3 && btm_edge + 3 < height) {
        int left = min(boundaries[btm_edge + 3][0], boundaries[top_edge - 3][0]);
        int right = max(boundaries[btm_edge + 3][1], boundaries[top_edge - 3][1]);
        clip(top_edge - 2, btm_edge + 2, left, right);
    } else if (top_edge == -1 && btm_edge != -1 && btm_edge + 3 < height) {
        clip(0, btm_edge + 2, boundaries[btm_edge + 3][0], boundaries[btm_edge + 3][1]);
    } else if (top_edge != -1 && btm_edge == -1 && top_edge >= 3) {
        clip(top_edge - 2, height - 1, boundaries[top_edge - 3][0], boundaries[top_edge - 3][1]);
    }

    // 释放内存
    delete[] boundary_num;
    delete[] boundaries;
    delete[] left_diff;
    delete[] right_diff;
    delete[] first_order;
    delete[] multiply;
}
