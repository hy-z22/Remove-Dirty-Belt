% 单个DR脏带去除
function single_remove4data2(filepath)

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

is_object = imageData ~= 65535;

is_boundary = zeros(width, height);
for j = 1:height
    % 找出可能的边界
    is_boundary(2:end-1, j) = (is_object(2:end-1, j) ~= 0) & ... % 本身非0
                ((is_object(1:end-2, j) == 0) | ... % 左0
                 (is_object(3:end, j) == 0)); % 右0
end

boundary_num = sum(is_boundary);

% 左右极限位置
boundary = zeros(2, height);
for j = 1:height
    indices = find(is_object(:, j));
    if ~isempty(indices)
        boundary(1, j) = min(indices);
        boundary(2, j) = max(indices);
    else
        boundary(:, j) = NaN;
    end
end

% 极限位置的一阶导
left_diff = diff(boundary(1, :));
right_diff = diff(boundary(2, :));
left_first_order = ([NaN, left_diff] + [left_diff, NaN]) / 2;
right_first_order = ([NaN, right_diff] + [right_diff, NaN]) / 2;
first_order = abs(left_first_order) + abs(right_first_order);

% 边界数与一阶导的乘积
multiply = boundary_num .* first_order;

% 找到一个脏带范围内的纵坐标
pin = find(multiply == max(multiply));

top_edge = NaN;
btm_edge = NaN;

% 寻找脏带的上下边界
for j= pin:-1:1+15
    if (sum(~(multiply(j-15:j-1) < 100)) == 0) || ((sum(~(boundary_num(j-7:j-1) == 2)) == 0) && (sum(~(multiply(j-5:j-1) < 100)) == 0))
        top_edge = j;
        break;
    else
        j = j-1;
    end
end

for j= pin:height-15
    if (sum(~(multiply(j+1:j+15) < 100)) == 0) || ((sum(~(boundary_num(j+1:j+7) == 2)) == 0) && (sum(~(multiply(j+1:j+5) < 100)) == 0))
        btm_edge = j;
        break;
    else
        j = j+1;
    end
end

% 既找到上边界又找到下边界
if ~isnan(top_edge) && ~isnan(btm_edge)
    % 左边
    i = min(boundary(1, btm_edge+3), boundary(1, top_edge-3));
    imageData(1:i, top_edge-2:btm_edge+2) = 65535;
    % 右边
    i = max(boundary(2, btm_edge+3), boundary(2, top_edge-3));
    imageData(i: width, top_edge-2:btm_edge+2) = 65535;
else
    if isnan(top_edge) % 只找到下边界
        % 左边
        imageData(1:boundary(1, btm_edge+3), 1:btm_edge+2) = 65535; 
        % 右边
        imageData(boundary(2, btm_edge+3): width, 1:btm_edge+2) = 65535;
    else % 只找到上边界
        % 左边
        imageData(1:boundary(1, top_edge-3), top_edge-2:height) = 65535;
        % 右边
        imageData(boundary(2, top_edge-3): width, top_edge-2:height) = 65535;
    end
end

% 显示处理后的灰度图
subplot(1,2,2);
imshow(imageData', []);
colormap gray;
title(sprintf('处理后图像 (尺寸: %d x %d)', width, height));

% 调整子图间距
set(fig, 'Position', [180, 120, 1200, 600]); % 调整图窗大小

output_path = fullfile('..\result2', [filename '_comparison.png']);
saveas(fig, output_path);
fprintf('处理结果已保存到: %s\n', output_path);

end