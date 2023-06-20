Merge "star.stl";

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
//+
Physical Surface("inlet 0", 30) = {6, 8, 7, 5};
//+
Physical Surface("outlet", 31) = {3, 4, 2};

Surface Loop(1) = {2, 3, 4, 5, 6, 7, 8};
Volume(1) = {1};

Physical Volume("domain") = {1};

Field[1] = MathEval;
Field[1].F = "0.1";
Background Field = 1;
