#pragma once

struct ViewMatrix {
	ViewMatrix(
		const float _11 = 0.f, const float _12 = 0.f, const float _13 = 0.f, const float _14 = 0.f,
		const float _21 = 0.f, const float _22 = 0.f, const float _23 = 0.f, const float _24 = 0.f,
		const float _31 = 0.f, const float _32 = 0.f, const float _33 = 0.f, const float _34 = 0.f,
		const float _41 = 0.f, const float _42 = 0.f, const float _43 = 0.f, const float _44 = 0.f
	) noexcept :
		_11(_11), _12(_12), _13(_13), _14(_14),
		_21(_21), _22(_22), _23(_23), _24(_24),
		_31(_31), _32(_32), _33(_33), _34(_34),
		_41(_41), _42(_42), _43(_43), _44(_44) {}

	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
};
