% 单个DR脏带去除
function single_split_remove(filepath)

% 从文件路径中提取文件名
[pathstr, filename, ~] = fileparts(filepath);

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

fraction_height = 200; % 设置为height即是不分割
step = fraction_height / 2;
front = step / 2;
back = step * (ceil(height/step) + 1) - front - height;
imageData_augmented = [65535 * ones(width, front), imageData, 65535 * ones(width, back)];
imageData_augmented_processed = imageData_augmented;
for i=1:ceil(height/step)
    imageData_augmented_processed(:, (i-1)*step+1+step/2:(i-1)*step+fraction_height-step/2) = split_remove(imageData_augmented(:, (i-1)*step+1:(i-1)*step+fraction_height));
end
imageData = imageData_augmented_processed(:, front+1:front+height);

% 显示处理后的灰度图
subplot(1,2,2);
imshow(imageData', []);
colormap gray;
title(sprintf('处理后图像 (尺寸: %d x %d)', width, height));

% 调整子图间距
set(fig, 'Position', [180, 120, 1200, 600]); % 调整图窗大小

% 分割输入路径
pathParts = strsplit(pathstr, filesep);

% 查找并替换data为result
for i = 1:length(pathParts)
    if startsWith(pathParts{i}, 'data')  % 查找以"data"开头的文件夹
        pathParts{i} = strrep(pathParts{i}, 'data', 'result');  % 替换为result
        break;  % 只替换第一个匹配的文件夹
    end
end

% 重建输出路径
outputPath = strjoin(pathParts, filesep);

% 确保输出目录存在
if ~exist(outputPath, 'dir')
    mkdir(outputPath);
end

output_file = fullfile(outputPath, [filename '_comparison.png']);
saveas(fig, output_file);
fprintf('处理结果已保存到: %s\n', output_file);

end