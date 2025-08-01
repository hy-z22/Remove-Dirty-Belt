#include "remove_belt.h"
#include "utils.h"

#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>

namespace fs = filesystem;
using namespace fs;

int main() {
    string inputDir;
    cout << "请输入包含待处理的二进制文件的文件夹的相对路径: ";
    getline(cin, inputDir);

    if (!exists(inputDir)) {
        cerr << "错误: 文件夹不存在 - " << inputDir << endl;
        return 1;
    }

    // 获取输入路径的父目录
    path parentDir = path(inputDir).parent_path();
    
    // 获取输入目录的最后部分（如"data1"）
    string dirName = path(inputDir).filename().string();

    // 构造输出目录名（如"data1" -> "result1"）
    string outputName = "result" + dirName.substr(dirName.find_first_of("0123456789"));

    // 组合成完整输出路径
    path outputDir = parentDir / outputName;
    create_directory(outputDir);

    cout << "正在扫描二进制文件..." << endl;
    size_t processedCount = 0;

    for (const auto& entry : directory_iterator(inputDir)) {
        if (entry.path().extension() == ".raw") {
            try {
                // 从文件名解析尺寸
                int width = 0, height = 0;
                if (!parseDimensions(entry.path().filename().string(), width, height)) {
                    cerr << "警告: 无法从文件名解析尺寸 - " << entry.path() << endl;
                    continue;
                }

                // 读取二进制文件
                size_t expectedSize = width * height;
                vector<uint16_t> inputData = readBinaryFile(entry.path(), expectedSize);

                // 处理数据
                int fraction_height = 200; // 设置为height即是不分割
                int step = fraction_height / 2;
                int front = step / 2;
                int back = step * (static_cast<int>(ceil(static_cast<double>(height) / step)) + 1) - front - height;
                vector<uint16_t> inputData_augmented;
                inputData_augmented.insert(inputData_augmented.end(), width * front, 65535);
                inputData_augmented.insert(inputData_augmented.end(), inputData.begin(), inputData.end());
                inputData_augmented.insert(inputData_augmented.end(), width * back, 65535);
                vector<uint16_t> outputData_augmented = inputData_augmented;
                uint16_t* input_ptr_augmented = inputData_augmented.data();
                uint16_t* output_ptr_augmented = outputData_augmented.data();
                for (int i = 0; i < static_cast<int>(ceil(static_cast<double>(height) / step)); i++) {
                    remove_belt(input_ptr_augmented + i * step * width, output_ptr_augmented + i * step * width, width, fraction_height);
                }
                vector<uint16_t> outputData = vector<uint16_t>(outputData_augmented.begin() + width * front, outputData_augmented.begin() + width * front + width * height);
                if (outputData.size() != expectedSize) {
                    cerr << "Error: outputData size incorrect! Got "
                            << outputData.size() << ", expected " << expectedSize << "\n";
                    exit(1);
                }

                // 构造输出文件名（将.raw替换为.png）
                path outputPath = outputDir / (entry.path().stem().string() + ".png");

                // 保存为PNG图片
                saveAsPng(outputData, width, height, outputPath);
                
                processedCount++;
                cout << "已处理: " << entry.path() << " (" << width << "x" << height 
                          << ") -> " << outputPath << endl;
            } catch (const exception& e) {
                cerr << "处理失败: " << entry.path() << " - " << e.what() << endl;
            }
        }
    }

    cout << "\n处理完成! 共处理 " << processedCount << " 个文件\n";
    cout << "输出目录: " << outputDir << endl;

    return 0;
}