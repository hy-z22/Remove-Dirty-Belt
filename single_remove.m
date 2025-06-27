% 单个DR脏带去除
function single_remove(filepath)

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

% 创建图窗
fig = figure('Visible', 'off');

% 显示原始灰度图
subplot(1,2,1);
imshow(imageData', []);
colormap gray;
title(sprintf('原始图像 (尺寸: %d x %d)', width, height));

% 获取纵向灰度差值
y_diff = diff(imageData, 1, 1);
y_diff = abs(y_diff);

% 过滤过小的差值
diff_max = max(y_diff(:));
threshold = 0.3 * diff_max;
diff_filtered = y_diff .* (y_diff > threshold);

% 确定边界
[rows, ~, ~] = find(diff_filtered);
min_row = min(rows);
max_row = max(rows);

% 截取
imageData_cut = imageData(min_row:max_row, :);

% 显示裁剪后的灰度图
subplot(1,2,2);
imshow(imageData_cut', []);
colormap gray;
title(sprintf('裁剪图像 (尺寸: %d x %d)', max_row-min_row+1, height));

% 调整子图间距
set(fig, 'Position', [180, 120, 1200, 600]); % 调整图窗大小

output_path = fullfile('result1', [filename '_comparison.png']);
saveas(fig, output_path);
fprintf('处理结果已保存到: %s\n', output_path);

end