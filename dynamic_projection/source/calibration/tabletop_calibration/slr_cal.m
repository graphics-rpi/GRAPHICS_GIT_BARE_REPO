clear;clc;close all
[filename, pathname] = uigetfile('*.ppm', 'Select Cornell Box Image');
image = imread([pathname '/' filename]);
cpselect(image, image);
stop

points2d = cpstruct.inputPoints;

points3d = [-0.21274 0 0.0470875;...
0.0191837 0 0.221913;...
0.199799 0 -0.0102294;...
-0.0355353 0 -0.189154;...
-0.21274 0.254 0.0470875;...
0.0191837 0.254 0.221913;...
0.199799 0.254 -0.0102294;...
-0.0355353 0.254 -0.189154];


mid_square = (points3d(1:4,:) + points3d(5:8,:))/2;
points3d = [points3d(1:4,:); mid_square; points3d(5:8,:)];

centers12 = (points3d([1 5 9], :) + points3d([2 6 10], :))/2;
centers23 = (points3d([2 6 10], :) + points3d([3 7 11], :))/2;
centers34 = (points3d([3 7 11], :) + points3d([4 8 12], :))/2;
centers41 = (points3d([4 8 12], :) + points3d([1 5 9], :))/2;
%midpoints = [centers12; centers23; centers34; centers41];
midpoints = zeros(size(points3d));
midpoints([1:4:size(points3d,1)],:) = centers12;
midpoints([2:4:size(points3d,1)],:) = centers23;
midpoints([3:4:size(points3d,1)],:) = centers34;
midpoints([4:4:size(points3d,1)],:) = centers41;

newpoints = zeros(size(points3d,1)*2, 3);
newpoints([1:2:size(newpoints,1)],:) = points3d;
newpoints([2:2:size(newpoints,1)],:) = midpoints;
points3d = newpoints;

scatter3(points3d(:,1), points3d(:,2), points3d(:,3),'w.')
xlabel 'X';
ylabel 'Y';
zlabel 'Z';
text(points3d(:,1), points3d(:,2), points3d(:,3),...
    num2str([(1:size(points3d,1))']))


points2d = points2d - repmat([1362 230], [size(points2d,1), 1]);


points = [points3d points2d];
save 'eos_50d_tsai.dat' points -ASCII

figure
axis([0 4752 0 3168])
text(points2d(:,1)', points2d(:,2)', num2str([1:size(points2d,1)]'));
