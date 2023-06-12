Merge "star-symmetry.stl";

DefineConstant[
	angle = {
		40, Min 20, Max 120, Step 1,
		Name "Parameters/Angle for surface detection"
	},
	forceParametrizablePatches = {0, Choices{0,1},
	Name "Parameters/Create surfaces guaranteed to be parametrizable"},
	includeBoundary = 1,
	curveAngle = 180
];
ClassifySurfaces{
	angle * Pi/180,
	includeBoundary,
	forceParametrizablePatches,
	curveAngle * Pi / 180
};

CreateGeometry;
Physical Surface("inlet", 26) = {5};
Physical Surface("outlet", 27) = {2, 6};
Physical Surface("symmetry 180 0", 28) = {4};
Physical Surface("symmetry 0 -90", 29) = {3};
Physical Surface("symmetry -45 0", 30) = {7};
Surface Loop(1) = {2, 3, 4, 5, 6, 7};
Volume(1) = {1};
Physical Volume("domain") = {1};

Field[1] = MathEval;
Field[1].F = "10";
Background Field = 1;
