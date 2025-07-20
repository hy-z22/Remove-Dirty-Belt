% 批量DR脏带去除
clc; clear; close all;

% 检查data2文件夹是否存在
if ~exist('..\..\data2', 'dir')
    error('data2文件夹不存在！');
end

% 获取data2文件夹下所有.raw文件
fileList = dir(fullfile('..\..\data2', '*.raw'));

if isempty(fileList)
    error('data2文件夹中没有找到.raw文件！');
end

fprintf('找到 %d 个.raw文件，开始批量处理...\n', length(fileList));

% 遍历所有文件
for i = 1:length(fileList)
    filepath = fullfile('..\..\data2', fileList(i).name);
    fprintf('正在处理 %d/%d: %s\n', i, length(fileList), filepath);
    
    % 调用单文件处理函数
    single_remove4data2(filepath);
end

fprintf('\n批量处理完成！共处理 %d 个文件。\n', length(fileList));