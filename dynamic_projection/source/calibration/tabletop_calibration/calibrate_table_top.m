%
% find and calibrate the table top in image space
%
% 4/13/09/tcy
%
function calibrate_table_top

clear;clc; close all
image = double(rgb2gray(imread('../../../build/camera_images/table_cal.ppm')))/255;
figure(1);imshow(image,[])

%
% !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
%
% set this threshold so that table edge is detected
%
% !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
thresh = .4;

% estimate edge strength
H = fspecial('sobel');
dir1 = imfilter(image, H, 'symmetric');
dir2 = imfilter(image, H', 'symmetric');
edges = abs(dir1) + abs(dir2);
figure(2);imshow(edges,[])

% find higest edge pixel on each half of each row
point_image = zeros(size(edges));
points = zeros(2*size(edges,1), 2);
half = size(edges,2)/2;

edges = (edges > thresh);
idx = 1;
for row = 1:size(edges,1)
    v1 = edges(row,1:half);
    [Y,I] = max(v1);
    if Y > 0
        points(idx, 1) = row;
        points(idx, 2) = I;
        idx = idx + 1;
        point_image(row, I) = 1;
    end
    v2 = edges(row,half+1:end);
    [Y,I] = max(v2);
    if Y > 0
        points(idx, 1) = row;
        points(idx, 2) = I+half;   
        idx = idx + 1;
        point_image(row,I+half) = 1;
    end
end

figure(3);imshow(point_image)

[xc,yc,r,a] = circfit(points(:,2), points(:,1))

figure(4);
imshow(image);
hold on;
plot(xc,yc,'y+');
for th = 0:pi/100:2*pi
    x = xc + r * cos(th);
    y = yc + r * sin(th);
    plot(x,y,'y.');
end

s = floor((yc + (size(image,1)-yc))/2);
%min_col = floor(xc - s) - 1;
%max_col = floor(xc + s-1) - 1;
%min_row = 0;
%max_row = size(image, 1) - 1;

min_col = 0;
max_col = 1935;
min_row = 0;
max_row = 1455;


%out = image(:,min_col:max_col);
%
%figure(5);imshow(out);

pause

fid = fopen('table_cal.dat', 'w');
fprintf(fid, '# table top calibration data\n');
fprintf(fid, '# table center (row col)\n');
fprintf(fid, '%f %f\n', yc, xc);
fprintf(fid, '# table radius\n');
fprintf(fid, '%f\n', r);
fprintf(fid, '# image crop region (min_row max_row min_col max_col)\n');
fprintf(fid, '%d %d %d %d\n', min_row, max_row, min_col, max_col);
fclose(fid);
%system 'cp table_cal.dat ../archdisplay';
system 'cp table_cal.dat /home/grfx/GIT_CHECKOUT/dynamic_projection/state/table_top_calibration';
quit
end
