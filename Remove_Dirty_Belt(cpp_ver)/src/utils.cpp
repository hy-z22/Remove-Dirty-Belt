#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <regex>
#include <stdexcept>

using namespace std;
namespace fs = filesystem;
using namespace fs;

// stb_image_write 单头文件
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// 辅助函数：从文件名中解析宽度和高度
bool parseDimensions(const string& filename, int& width, int& height) {
    regex pattern(R"((\d+)x(\d+)\.raw$)");
    smatch matches;
    
    if (regex_search(filename, matches, pattern)) {
        if (matches.size() == 3) {
            width = stoi(matches[1].str());
            height = stoi(matches[2].str());
            return true;
        }
    }
    return false;
}

// 辅助函数：将二进制文件读取到一维数组
vector<uint16_t> readBinaryFile(const path& filePath, size_t expectedSize) {
    ifstream file(filePath, ios::binary | ios::ate);
    if (!file) {
        throw runtime_error("无法打开文件: " + filePath.string());
    }

    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    if (size != static_cast<streamsize>(expectedSize * sizeof(uint16_t))) {
        throw runtime_error("文件大小与预期尺寸不匹配");
    }

    vector<uint16_t> buffer(expectedSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw runtime_error("读取文件失败: " + filePath.string());
    }

    return buffer;
}

// 使用 stb_image_write 保存PNG
void saveAsPng(const vector<uint16_t>& data, int width, int height, const path& outputPath) {
    vector<uint8_t> pixels(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        // 归一化到 0~1，再乘以255
        float normalized = static_cast<float>(data[i]) / 65535.0f;
        normalized = clamp(normalized, 0.0f, 1.0f);  // 限制在 [0, 1] 内
        pixels[i] = static_cast<uint8_t>(normalized * 255.0f + 0.5f);  // 四舍五入
    }

    if (stbi_write_png(outputPath.string().c_str(), width, height, 1, pixels.data(), width) == 0) {
        throw runtime_error("保存PNG失败: " + outputPath.string());
    }
}