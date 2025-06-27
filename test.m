% DR脏带去除
clc; clear; close all;

% 文件路径
filepath = 'data1\006332_1800x831.raw';

% 从文件路径中提取文件名
[~, filename, ~] = fileparts(filepath);

% 从文件名提取宽高
pattern = '(?<width>\d+)x(?<height>\d+)'; % 正则表达式匹配
fileInfo = regexp(filename, pattern, 'names');

if isempty(fileInfo)
    error('文件名不包含有效的宽高信息');
end

width = str2double(fileInfo.width);   % 图像宽度
height = str2double(fileInfo.height); % 图像高度

fprintf('文件路径: %s\n', filepath);
fprintf('检测到的尺寸: 宽度 = %d, 高度 = %d\n', width, height);

% 读取二进制文件
fid = fopen(filepath, 'rb');
if fid == -1
    error('无法打开文件: %s', filepath);
end

imageData = fread(fid, [width, height], 'uint16');
fclose(fid);

% 显示原始灰度图
figure;
imshow(imageData', []);
colormap gray;
title(sprintf('原始图像 (尺寸: %d x %d)', width, height));

% 粗略筛选出带的部分
is_belt = ((imageData >= 60000) .* (imageData < 65535));

figure;
imshow(is_belt', []);
colormap gray;

% 细致筛选
x_sum = sum(is_belt, 2); % 沿运动方向求和
[rows, cols, ~] = find(is_belt);
for i = 1:length(rows)
    row = rows(i);
    col = cols(i);
    if sum(is_belt(row, max(col-25, 1):min((col+25), height))) < 10
        is_belt(row, col) = 0;
    end
end

figure;
imshow(is_belt', []);
colormap gray;

[~, cols, ~] = find(is_belt);
% 统计出现最多的col值并据此最后筛选
is_belt(:, 1:max(mode(cols)-25, 1)) = 0;
is_belt(:, min(mode(cols)+25, height):height) = 0;

[~, cols, ~] = find(is_belt);
min_col = min(cols);
max_col = max(cols);

% 获取纵向灰度差值
y_diff = diff(imageData, 1, 1);
y_diff = abs(y_diff);

% 过滤过小的差值
diff_max = max(y_diff(:));
threshold = 0.22 * diff_max;
diff_filtered = y_diff .* (y_diff > threshold);

figure;
imshow(diff_filtered', []);
colormap gray;

% 确定边界
[edge_rows, edge_cols, ~] = find(diff_filtered);
valid_indices = edge_cols >= min_col & edge_cols <= max_col;
if any(valid_indices)
    filtered_rows = edge_rows(valid_indices);
    filtered_cols = edge_cols(valid_indices);
    [unique_cols, ~, ic] = unique(filtered_cols);
    min_rows = accumarray(ic, filtered_rows, [], @min);
    sorted_min_rows = sort(min_rows);
    n = numel(sorted_min_rows);
    keep_start = max(floor(0.3 * n) + 1, 1);  % 确保至少保留1个点
    keep_end = min(n - floor(0.3 * n), n);    % 确保不越界
    robust_min_rows = sorted_min_rows(keep_start:keep_end);
    mean_row = mean(robust_min_rows);
    imageData(1:floor(mean_row)-50, min_col:max_col) = 65535;
else
    imageData(:, min_col:max_col) = 65535;
end

% 显示处理后的灰度图
figure;
imshow(imageData', []);
colormap gray;
title(sprintf('处理后图像 (尺寸: %d x %d)', width, height));