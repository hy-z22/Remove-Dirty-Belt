#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>

namespace fs = filesystem;
using namespace fs;

// 辅助函数：从文件名中解析宽度和高度
bool parseDimensions(const string& filename, int& width, int& height);

// 辅助函数：将二进制文件读取到一维数组
vector<uint16_t> readBinaryFile(const path& filePath, size_t expectedSize);

// 辅助函数：保存一维数组为PNG图片
void saveAsPng(const vector<uint16_t>& data, int width, int height, const path& outputPath);