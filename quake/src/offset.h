#pragma once
#include <cstddef>
namespace offset
{
	//cgamex
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xA9C210;
	constexpr ::std::ptrdiff_t yaw = 0xA4;
	constexpr ::std::ptrdiff_t pitch = 0xA0;
	constexpr ::std::ptrdiff_t dwRefDef = 0xA9C820;

	//quakelive.exe
	constexpr ::std::ptrdiff_t yawSum = 0x1072754;
	constexpr ::std::ptrdiff_t pitchSum = 0x1072758;

	//qagamex
	constexpr ::std::ptrdiff_t dwPlayerList = 0x5ADD58;
	constexpr ::std::ptrdiff_t dwPlayerLength = 0xBD8;

	//qagamex + entity
	constexpr ::std::ptrdiff_t entityX = 0x14;
	constexpr ::std::ptrdiff_t entityY = 0x18;
	constexpr ::std::ptrdiff_t entityZ = 0x1C;
	constexpr ::std::ptrdiff_t team = 0x10C;

	//cgamex
	constexpr ::std::ptrdiff_t redPlayersNum = 0xA404BC;
	constexpr ::std::ptrdiff_t bluePlayersNum = 0xA404C0;
	//constexpr ::std::ptrdiff_t viewMatrix = 0x;

}