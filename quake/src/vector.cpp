#include "vector.h"

Vector3 Vector3::WorldToScreen(pRefDef pRefDef)
{
	const auto viewOriginVec = Vector3{ pRefDef.viewOrigin[0], pRefDef.viewOrigin[1],pRefDef.viewOrigin[2] };
	const auto viewAxisX = Vector3{ pRefDef.viewAxis[0], pRefDef.viewAxis[1],pRefDef.viewAxis[2] };
	const auto viewAxisY = Vector3{ pRefDef.viewAxis[3], pRefDef.viewAxis[4],pRefDef.viewAxis[5] };
	const auto viewAxisZ = Vector3{ pRefDef.viewAxis[6], pRefDef.viewAxis[7],pRefDef.viewAxis[8] };
	const auto pos = Vector3{ x, y, z };
	auto screen = Vector3{};
	Vector3 trans;
	float xc, yc;
	float px, py;
	float z;

	px = tan(pRefDef.fovY * 3.14 / 360.0);
	py = tan(pRefDef.fovX * 3.14 / 360.0);

	trans = pos - viewOriginVec;

	xc = pRefDef.width / 2.0;
	yc = pRefDef.height / 2.0;

	z = trans.dot(viewAxisX);
	if (z <= 0.001)
		return false;

	screen.x = xc - trans.dot(viewAxisY) * xc / (z * px);
	screen.y = yc - trans.dot(viewAxisZ) * yc / (z * py);
	return screen;
}