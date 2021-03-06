l   = 1;
d   = 50;
L   = l / 2;
cl  = L / d;

Point(1) = {+L, -L, 0, cl};
Point(2) = {+L, +L, 0, cl};
Point(3) = {-L, +L, 0, cl};
Point(4) = {-L, -L, 0, cl};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};

Line Loop(1)     = {1, 2, 3, 4};
Plane Surface(1) = {1};

Physical Line(5) = {3};
Physical Line(6) = {1, 2, 4};
Physical Surface(7) = {1};
