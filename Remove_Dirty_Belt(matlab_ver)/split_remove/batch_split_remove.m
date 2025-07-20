% 批量DR脏带去除
clc; clear; close all;

% 待处理的文件路径
data_path = '..\..\data3';

% 检查文件夹是否存在
if ~exist(data_path, 'dir')
    error('指定文件夹不存在！');
end

% 获取给定路径下所有.raw文件
fileList = dir(fullfile(data_path, '*.raw'));

if isempty(fileList)
    error('指定文件夹中没有找到.raw文件！');
end

fprintf('找到 %d 个.raw文件，开始批量处理...\n', length(fileList));

% 遍历所有文件
for i = 1:length(fileList)
    filepath = fullfile(data_path, fileList(i).name);
    fprintf('正在处理 %d/%d: %s\n', i, length(fileList), filepath);
    
    % 调用单文件处理函数
    single_split_remove(filepath);
end

fprintf('\n批量处理完成！共处理 %d 个文件。\n', length(fileList));