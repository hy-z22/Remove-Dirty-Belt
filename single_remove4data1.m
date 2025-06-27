% 单个DR脏带去除
function single_remove4data1(filepath)

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

is_boundary = false(width, height); % 初始化边界标签矩阵

% 定义8邻域偏移量（包括上下左右和对角线）
neighbors = [-1 -1; -1 0; -1 1; 0 -1; 0 1; 1 -1; 1 0; 1 1];

for i = 1:width
    for j = 1:height
        % 只处理自身不为白的点
        if imageData(i, j) ~= 65535
            % 检查所有相邻像素
            for k = 1:size(neighbors, 1)
                ni = i + neighbors(k, 1);
                nj = j + neighbors(k, 2);
                
                % 确保相邻像素在图像范围内
                if ni >= 1 && ni <= width && nj >= 1 && nj <= height
                    if imageData(ni, nj) == 65535
                        is_boundary(i, j) = true;
                        break; % 找到一个满足条件的相邻点即可
                    end
                end
            end
        end
    end
end

is_object = imageData<45000;

% 结合粗略物体分布修正边界分布
[edge_x, edge_y] = find(is_boundary);
for k = 1:length(edge_x)
    x = edge_x(k);
    y = edge_y(k);
    if sum(is_object(max(x-100, 1):min(x+100, width), max(y-100, 1):min(y+100, height))) == 0
        is_boundary(x, y) = 0;
    end
end

for j = 1:height
    internal = find(is_boundary(:, j));
    if isempty(internal)
        imageData(:, j) = 65535;
    else
        left = min(internal);
        right = max(internal);
        imageData(1:left-1, j) = 65535;
        imageData(right+1:width, j) = 65535;
    end
end

% 显示处理后的灰度图
subplot(1,2,2);
imshow(imageData', []);
colormap gray;
title(sprintf('处理后图像 (尺寸: %d x %d)', width, height));

% 调整子图间距
set(fig, 'Position', [180, 120, 1200, 600]); % 调整图窗大小

output_path = fullfile('result1', [filename '_comparison.png']);
saveas(fig, output_path);
fprintf('处理结果已保存到: %s\n', output_path);

end