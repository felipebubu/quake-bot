#pragma once
#include "vector.h"
class Entity
{
public:
	char unknown0[4]; //0x0000
	__int32 type; //0x0004 1 = alive | 10 = dead or spectating 
	char unknown8[4]; //0x0008
	__int32 clientNum; //0x000C  
	char unknown16[8]; //0x0010
	float posX; //0x0018  
	float posY; //0x001C  
	float posZ; //0x0020  
	char unknown36[28]; //0x0024
	float viewAngleX; //0x0040  
	float viewAngleY; //0x0044  
	char unknown72[128]; //0x0048
	__int32 weaponID; //0x00C8  
	__int32 eState; //0x00CC 151 == crouching 
	char unknown208[16]; //0x00D0
	__int32 shooting; //0x00E0 256+ == shooting 
	char unknown228[420]; //0x00E4
	Vector3 position; //0x0288  
	char unknown660[12]; //0x0294
};//Size=0x02A0(672)