% 只处理传入数据中间部分的数据，两边数据只辅助判断
function imageData_processed = split_remove(imageData)
    % close all;
    [x_length, y_length] = size(imageData);
    is_object = imageData ~= 65535;

    is_boundary = zeros(x_length, y_length);
    for j = 1:y_length
        % 找出可能的边界
        is_boundary(2:end-1, j) = (is_object(2:end-1, j) ~= 0) & ... % 本身非0
                    ((is_object(1:end-2, j) == 0) | ... % 左0
                     (is_object(3:end, j) == 0)); % 右0
    end
    
    % % 显示可能的边界
    % figure;
    % imshow(is_boundary', []);
    % colormap gray;
    % title(sprintf('可能的边界 (尺寸: %d x %d)', x_length, y_length));

    % 边界数
    boundary_num = sum(is_boundary);

    % 左右极限位置
    boundary = zeros(2, y_length);
    for j = 1:y_length
        indices = find(is_object(:, j));
        if ~isempty(indices)
            boundary(1, j) = min(indices);
            boundary(2, j) = max(indices);
        else
            boundary(:, j) = NaN;
        end
    end

    % 平均位置的一阶导
    mean_diff = diff(mean(boundary));
    first_order = (abs([NaN, mean_diff]) + abs([mean_diff, NaN])) / 2;

    % 物体宽度
    delta = boundary(2, :) - boundary(1, :);

    % figure;
    % % 边界数变化曲线
    % subplot(3, 1, 1); % 3行1列的第1个子图
    % plot(1:y_length, boundary_num, 'r-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('边界数');
    % title('边界数随纵坐标的变化');
    % grid on;
    % % 平均位置一阶导变化曲线
    % subplot(3, 1, 2); % 3行1列的第2个子图
    % plot(1:y_length, first_order, 'g-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('平均位置一阶导');
    % title('平均位置一阶导随纵坐标的变化');
    % grid on;
    % % 宽度变化曲线
    % subplot(3, 1, 3); % 3行1列的第3个子图
    % plot(1:y_length, delta, 'b-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('物体宽度');
    % title('物体宽度随纵坐标的变化');
    % grid on;
    % % 调整子图间距
    % set(gcf, 'Position', [100, 100, 800, 600]); % 设置图形窗口大小
    
    % 筛选脏带
    filtered_indices = ((first_order > 0.25*max(first_order)) & (delta > 1.15*mean(delta(boundary_num < 10),'omitnan'))) ...
                       | (boundary_num == 0 & [false, cummax(boundary_num(1:end-1) ~= 0)] & [cummax(boundary_num(2:end) ~= 0, 'reverse'), false]);

    % figure;
    % % 边界数变化曲线
    % subplot(3, 1, 1); % 3行1列的第1个子图
    % plot(1:y_length, boundary_num .* filtered_indices, 'r-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('边界数');
    % title('边界数随纵坐标的变化');
    % grid on;
    % % 平均位置一阶导变化曲线
    % subplot(3, 1, 2); % 3行1列的第2个子图
    % plot(1:y_length, first_order .* filtered_indices, 'g-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('平均位置一阶导');
    % title('平均位置一阶导随纵坐标的变化');
    % grid on;
    % % 宽度变化曲线
    % subplot(3, 1, 3); % 3行1列的第3个子图
    % plot(1:y_length, delta .* filtered_indices, 'b-', 'LineWidth', 0.5);
    % xlabel('y');
    % ylabel('物体宽度');
    % title('物体宽度随纵坐标的变化');
    % grid on;
    % % 调整子图间距
    % set(gcf, 'Position', [100, 100, 800, 600]); % 设置图形窗口大小

    belt_indices = find(filtered_indices);
    top = min(belt_indices);
    btm = max(belt_indices);
    
    if ~isempty(belt_indices)
        if boundary_num(1) == 0
            imageData(1:boundary(1, min(btm+1, y_length)), 1+y_length/4:btm) = 65535;
            imageData(boundary(2, min(btm+1, y_length)):x_length, 1+y_length/4:btm) = 65535;
        end
        if boundary_num(y_length) == 0
            imageData(1:boundary(1, max(top-1, 1)), top:y_length*3/4) = 65535;
            imageData(boundary(2, max(top-1, 1)):x_length, top:y_length*3/4) = 65535;
        end
        if boundary_num(1) ~= 0 && boundary_num(y_length) ~= 0
            imageData(1:min(boundary(1, max(top-1, 1)), boundary(1, min(btm+1, y_length))), max(top, 1+y_length/4):min(btm, y_length*3/4)) = 65535;
            imageData(max(boundary(2, max(top-1, 1)), boundary(2, min(btm+1, y_length))):x_length, max(top, 1+y_length/4):min(btm, y_length*3/4)) = 65535; 
        end
    end
    imageData_processed = imageData(:, y_length/4+1:y_length*3/4);
end