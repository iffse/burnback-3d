Merge "cylinder.stl";

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

Physical Surface("inlet 0", 6) = {4};
Physical Surface("outlet", 7) = {2, 3};

Surface Loop(1) = Surface{:};
Volume(1) = {1};

Physical Volume("domain", 8) = {1};
