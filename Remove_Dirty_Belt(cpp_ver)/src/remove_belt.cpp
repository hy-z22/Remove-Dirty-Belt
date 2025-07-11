#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

using namespace std;

uint16_t* remove_belt(const uint16_t* initial_imageData, int width, int height) {
    // 将一维数组转换为二维矩阵 (列优先)
    uint16_t** imageData = new uint16_t*[height];
    for (int y = 0; y < height; y++) {
        imageData[y] = new uint16_t[width];
        memcpy(imageData[y], &initial_imageData[y * width], sizeof(uint16_t) * width);
    }

    // 区分物体和背景
    bool** is_object = new bool*[height];
    for (int y = 0; y < height; y++) {
        is_object[y] = new bool[width];
        for (int x = 0; x < width; x++) {
            is_object[y][x] = (imageData[y][x] != 65535);
        }
    }

    // 找出可能的边界
    bool** is_boundary = new bool*[height];
    for (int y = 0; y < height; y++) {
        is_boundary[y] = new bool[width];
        is_boundary[y][0] = 0;
        is_boundary[y][width - 1] = 0;
        for (int x = 1; x < width - 1; x++) {
            is_boundary[y][x] = is_object[y][x] && (!is_object[y][x-1] || !is_object[y][x+1]);
        }
    }

    // 计算每行的边界数量
    int* boundary_num = new int[height];
    for (int y = 0; y < height; y++) {
        int cnt = 0;
        for (int x = 0; x < width; x++) {
            if (is_boundary[y][x]) cnt++;
        }
        boundary_num[y] = cnt;
    }

    // 计算左右极限位置
    int (*boundaries)[2] = new int[height][2];
    for (int y = 0; y < height; y++) {
        int left = width, right = -1;
        for (int x = 0; x < width; x++) {
            if (is_object[y][x]) {
                if (x < left) left = x;
                if (x > right) right = x;
            }
        }
        boundaries[y][0] = (left != width) ? left : -1;
        boundaries[y][1] = (right != -1) ? right : -1;
    }

    // 计算边界位置的一阶导数
    double* left_diff = new double[height]();
    double* right_diff = new double[height]();
    for (int y = 1; y < height; y++) {
        left_diff[y] = (boundaries[y][0] != -1 && boundaries[y-1][0] != -1) * (boundaries[y][0] - boundaries[y-1][0]);
        right_diff[y] = (boundaries[y][1] != -1 && boundaries[y-1][1] != -1) * (boundaries[y][1] - boundaries[y-1][1]);
    }

    double* first_order = new double[height]();
    for (int y = 1; y < height - 1; y++) {
        double l_avg = (left_diff[y] + left_diff[y+1]) / 2.0;
        double r_avg = (right_diff[y] + right_diff[y+1]) / 2.0;
        first_order[y] = fabs(l_avg) + fabs(r_avg);
    }

    // 计算边界数与一阶导数的乘积
    double* multiply = new double[height];
    for (int y = 0; y < height; y++) {
        multiply[y] = boundary_num[y] * first_order[y];
    }

    // 找到乘积最大的位置 (定位脏带)
    int pin = max_element(multiply, multiply + height) - multiply;

    // 寻找脏带的上下边界
    int top_edge = -1, btm_edge = -1;
    for (int y = pin; y >= 15; y--) {
        bool cond1 = true, cond2a = true, cond2b = true;
        for (int i = y-15; i < y && i >= 0; i++) if (multiply[i] >= 100) cond1 = false;
        for (int i = y-7; i < y && i >= 0; i++) if (boundary_num[i] != 2) cond2a = false;
        for (int i = y-5; i < y && i >= 0; i++) if (multiply[i] >= 100) cond2b = false;
        if (cond1 || (cond2a && cond2b)) {
            top_edge = y;
            break;
        }
    }

    for (int y = pin; y < height - 15; y++) {
        bool cond1 = true, cond2a = true, cond2b = true;
        for (int i = y+1; i <= y+15 && i < height; i++) if (multiply[i] >= 100) cond1 = false;
        for (int i = y+1; i <= y+7 && i < height; i++) if (boundary_num[i] != 2) cond2a = false;
        for (int i = y+1; i <= y+5 && i < height; i++) if (multiply[i] >= 100) cond2b = false;
        if (cond1 || (cond2a && cond2b)) {
            btm_edge = y;
            break;
        }
    }

    // 清除脏带
    auto clip = [&](int y_from, int y_to, int left, int right) {
        for (int y = y_from; y <= y_to && y < height; y++) {
            for (int x = 0; x <= left; x++) imageData[y][x] = 65535;
            for (int x = right; x < width; x++) imageData[y][x] = 65535;
        }
    };

    if (top_edge != -1 && btm_edge != -1) {
        int left = min(boundaries[btm_edge+3][0], boundaries[top_edge-3][0]);
        int right = max(boundaries[btm_edge+3][1], boundaries[top_edge-3][1]);
        clip(top_edge - 2, btm_edge + 2, left, right);
    } else if (top_edge == -1 && btm_edge != -1) {
        int left = boundaries[btm_edge+3][0];
        int right = boundaries[btm_edge+3][1];
        clip(0, btm_edge + 2, left, right);
    } else if (top_edge != -1 && btm_edge == -1) {
        int left = boundaries[top_edge-3][0];
        int right = boundaries[top_edge-3][1];
        clip(top_edge - 2, height - 1, left, right);
    }

    // 返回处理结果 (一维数组)
    uint16_t* processed_imageData = new uint16_t[width * height];
    for (int y = 0; y < height; y++) {
        memcpy(&processed_imageData[y * width], imageData[y], sizeof(uint16_t) * width);
    }

    // 内存释放
    for (int y = 0; y < height; y++) {
        delete[] imageData[y];
        delete[] is_object[y];
        delete[] is_boundary[y];
    }
    delete[] imageData;
    delete[] is_object;
    delete[] is_boundary;
    delete[] boundary_num;
    delete[] boundaries;
    delete[] left_diff;
    delete[] right_diff;
    delete[] first_order;
    delete[] multiply;

    return processed_imageData;
}
