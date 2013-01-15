close all;
clear all;

%% l2 [Order][Mesh]

%% [Sin(10x), Sin(10y), Sin(10z)]

h = [1, 1/2, 1/4, 1/8;]%, 1/16];
p = [1:5];

P = size(p, 2);
H = size(h, 2);

l2 = zeros(P, H);

l2(1, 0 + 1) = +1.142827e+00;
l2(1, 1 + 1) = +6.869424e-01;
l2(1, 2 + 1) = +1.846014e-01;
l2(1, 3 + 1) = +5.101822e-02;
%l2(1, 4 + 1) = +1.275097e-02;

l2(2, 0 + 1) = +1.084487e+00;
l2(2, 1 + 1) = +2.502310e-01;
l2(2, 2 + 1) = +4.834511e-02;
l2(2, 3 + 1) = +6.441440e-03;
%l2(2, 4 + 1) = +8.565687e-04;

l2(3, 0 + 1) = +7.455704e-01;
l2(3, 1 + 1) = +9.920041e-02;
l2(3, 2 + 1) = +5.990588e-03;
l2(3, 3 + 1) = +3.872353e-04;
%l2(3, 4 + 1) = +2.306230e-05;

l2(4, 0 + 1) = +3.385621e-01;
l2(4, 1 + 1) = +2.092672e-02;
l2(4, 2 + 1) = +1.002601e-03;
l2(4, 3 + 1) = +3.386785e-05;

l2(5, 0 + 1) = +3.275472e-01;
l2(5, 1 + 1) = +6.078320e-03;
l2(5, 2 + 1) = +7.688799e-05;
l2(5, 3 + 1) = +1.169738e-06;

delta = zeros(P, H - 1);

for i = 1:H-1
    delta(:, i) = ...
        (log10(l2(:, i + 1)) - log10(l2(:, i))) / ...
        (log10(1/h(i + 1))   - log10(1/h(i)));
end

delta

figure;
loglog(1./h, l2, '-*');
grid;
xlabel('1/h [-]');
ylabel('l2Error [-]');
title('Edge Tets');
