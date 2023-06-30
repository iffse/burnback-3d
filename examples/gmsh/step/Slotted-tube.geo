Merge "Slotted-tube.stp";

SetFactory("OpenCASCADE");

Rotate {{0, 1, 0}, {0, 0, 0}, Pi/2} {
  Volume{2}; Volume{1};
}
Coherence;

Physical Surface("symmetry -60 0", 24) = {9, 4};
Physical Surface("symmetry 0 0", 25) = {1};
Physical Surface("outlet", 26) = {11, 8, 6, 5};
Physical Surface("inlet 0", 27) = {12, 7, 10, 3};

Physical Volume("domain", 28) = {2, 1};

Field[1] = MathEval;
Field[1].F = "5";

Background Field = 1;
