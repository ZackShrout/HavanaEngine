#if !defined(HAVANA_COMMON_HLSLI) && !defined(__cplusplus)
#error Do not include this header directly in shader files. Only include this file via Common.hlsli.
#endif

// Compute a plane from 3 noncollinear points that form a triangle.
// This equation assumes a right-handed (counter-clockwise winding order) 
// coordinate system to determine the direction of the plane normal.
Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
	Plane plane;

	// NOTE: swap v0 and v2 if using left-handed (clockwise winding order)
	//		 coordinate system
	const float3 v0 = p1 - p0;
	const float3 v2 = p2 - p0;

	plane.Normal = normalize(cross(v0, v2));
	plane.Distance = dot(plane.Normal, p0);

	return plane;
}

// Check to see if a point is fully behind (inside the negative halfspace of) a plane.
bool PointInsidePlane(float3 p, Plane plane)
{
	return dot(plane.Normal, p) - plane.Distance < 0;
}

// Check to see if a sphere is fully behind (inside the negative halfspace of) a plane.
// Source: Real-time collision detection, Christer Ericson (2005)
bool SphereInsidePlane(Sphere sphere, Plane plane)
{
	return dot(plane.Normal, sphere.Center) - plane.Distance < -sphere.Radius;
}

// Check to see if a cone if fully behind (inside the negative halfspace of) a plane.
// Source: Real-time collision detection, Christer Ericson (2005)
bool ConeInsidePlane(Cone cone, Plane plane)
{
	// Compute the farthest point on the end of the cone to the positive space of the plane.
	float3 m = cross(cross(plane.Normal, cone.Direction), cone.Direction);
	float3 Q = cone.Tip + cone.Direction * cone.Height - m * cone.Radius;

	// The cone is in the negative halfspace of the plane if both
	// the tip of the cone and the farthest point on the end of the cone to the 
	// positive halfspace of the plane are both inside the negative halfspace 
	// of the plane.
	return PointInsidePlane(cone.Tip, plane) && PointInsidePlane(Q, plane);
}

// Check to see of a light is partially contained within the frustum.
bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
	// First check depth
	// Note: Here, the view vector points in the -Z axis so the 
	// far depth value will be approaching -infinity.
	return !((sphere.Center.z - sphere.Radius > zNear || sphere.Center.z + sphere.Radius < zFar) ||
			  SphereInsidePlane(sphere, frustum.Planes[0]) ||
			  SphereInsidePlane(sphere, frustum.Planes[1]) ||
			  SphereInsidePlane(sphere, frustum.Planes[2]) ||
			  SphereInsidePlane(sphere, frustum.Planes[3]));
}

bool ConeInsideFrustum(Cone cone, Frustum frustum, float zNear, float zFar)
{
	bool result = true;

	Plane nearPlane = { float3(0, 0, -1), -zNear };
	Plane farPlane = { float3(0, 0, 1), zFar };

	// First check the near and far clipping planes.
	if (ConeInsidePlane(cone, nearPlane) || ConeInsidePlane(cone, farPlane))
	{
		result = false;
	}

	// Then check frustum planes
	for (int i = 0; i < 4 && result; i++)
	{
		if (ConeInsidePlane(cone, frustum.Planes[i]))
		{
			result = false;
		}
	}

	return result;
}

float4 ClipToView(float4 clip, float4x4 inverseProjection)
{
	// View space position
	float4 view = mul(inverseProjection, clip);
	
	// Perspective (un)projection
	view /= view.w;

	return view;
}

float4 ScreenToView(float4 screen, float2 invViewDimensions, float4x4 inverseProjection)
{
	// Convert to normalized texture coordinates
	float2 texCoord = screen.xy * invViewDimensions;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.f - texCoord.y) * 2.f - 1.f, screen.z, screen.w);

	return ClipToView(clip, inverseProjection);
}