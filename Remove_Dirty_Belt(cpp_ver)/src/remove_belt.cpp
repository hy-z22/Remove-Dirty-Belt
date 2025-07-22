#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

using namespace std;

void remove_belt(const uint16_t* imageData, uint16_t* imageData_processed, int width, int height) {
    const uint16_t BACKGROUND = 65535;
    
    // 分配所有需要的数组
    int* boundary_num = new int[height]{};
    int (*boundaries)[2] = new int[height][2]{};
    double* mean_pos = new double[height]{};
    double* diff = new double[height]{};
    double* first_order = new double[height]{};
    double* delta = new double[height]{};
    bool* filtered_indices = new bool[height]{};

    // 同时计算boundary_num,boundaries,delta
    for (int y = 0; y < height; y++) {
        int left = width, right = -1, boundary_counts = 0;
        int row_offset = y * width;
        
        for (int x = 0; x < width; x++) {
            int idx = row_offset + x;

            if (imageData[idx] != BACKGROUND) {
                if (x < left) left = x;
                if (x > right) right = x;
                if ((x > 0) && (x < width - 1) && ((imageData[idx - 1] == BACKGROUND) || (imageData[idx + 1] == BACKGROUND))) {
                    boundary_counts++;
                }
            }
        }
        
        boundary_num[y] = boundary_counts;
        boundaries[y][0] = (left != width) ? left : -1;
        boundaries[y][1] = (right != -1) ? right : -1;
        mean_pos[y] = (boundaries[y][0] + boundaries[y][1]) / 2;
        if (boundaries[y][0] != -1 && boundaries[y][1] != -1)
            delta[y] = boundaries[y][1] - boundaries[y][0];
    }

    // 计算一阶导数
    for (int y = 1; y < height; y++) {
        if (mean_pos[y] != -1 && mean_pos[y - 1] != -1)
            diff[y] = mean_pos[y] - mean_pos[y - 1];
    }

    for (int y = 1; y < height - 1; y++) {
        first_order[y] = (fabs(diff[y]) + fabs(diff[y + 1])) / 2;
    }

    double mean_delta = 0;
    int count = 0;
    double max_first_order = 0;
    int inf = -1, sup = -1;
    
    // 计算平均物体宽度（只计入边界数大于10的）和最大一阶导数
    for (int y = 0; y < height; y++) {
        if (boundary_num[y] < 10 && delta[y] > 0) {
            mean_delta += delta[y];
            count++;
        }
        if (first_order[y] > max_first_order) {
            max_first_order = first_order[y];
        }
        if (boundary_num[y] > 0) {
            if (inf == -1) inf = y;
            sup = y;
        }
    }
    mean_delta = (count > 0) ? mean_delta / count : 0;

    // 检测脏带区域
    for (int y = 0; y < height; y++) {
        // 条件1: 一阶导数较大且宽度异常
        bool condition1 = (first_order[y] > 0.25 * max_first_order) && 
                         (delta[y] > 1.15 * mean_delta);
        
        // 条件2: 当前无边界但前后有边界
        bool condition2 = (boundary_num[y] == 0) && y > inf && y < sup;
        
        filtered_indices[y] = condition1 || condition2;
    }

    // 确定脏带上下边界
    int top = -1, btm = -1;
    for (int y = 0; y < height; y++) {
        if (filtered_indices[y]) {
            if (top == -1) top = y;
            btm = y;
        }
    }

    // 处理脏带区域
    auto clear_region = [&](int y_from, int y_to, int left, int right) {
        for (int y = y_from; y <= y_to && y < height; y++) {
           int row_offset = y * width;
            for (int x = 0; x <= left && x < width; x++) imageData_processed[row_offset + x] = BACKGROUND;
            for (int x = right; x < width; x++) imageData_processed[row_offset + x] = BACKGROUND;
        }
    };

    if (top != -1 && btm != -1) {
        if (boundary_num[0] == 0) {
            int left = boundaries[min(btm+1,height-1)][0];
            int right = boundaries[min(btm+1,height-1)][1];
            clear_region(height/4, min(btm,height*3/4  - 1), left, right);
        }
        
        if (boundary_num[height-1] == 0) {
            int left = boundaries[max(top-1,0)][0];
            int right = boundaries[max(top-1,0)][1];
            clear_region(max(top,height/4), height*3/4 - 1, left, right);
        }
        
        if (boundary_num[0] != 0 && boundary_num[height-1] != 0) {
            int left = min(boundaries[max(top-1,0)][0], boundaries[min(btm+1,height-1)][0]);
            int right = max(boundaries[max(top-1,0)][1], boundaries[min(btm+1,height-1)][1]);
            clear_region(max(top,height/4), min(btm,height*3/4  - 1), left, right);
        }
    }

    // 释放内存
    delete[] boundary_num;
    delete[] boundaries;
    delete[] mean_pos;
    delete[] diff;
    delete[] first_order;
    delete[] delta;
    delete[] filtered_indices;
}