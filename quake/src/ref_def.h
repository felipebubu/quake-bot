#pragma once

class pRefDef
{
public:
	__int32 x; //0x0000 
	__int32 y; //0x0004 
	__int32 width; //0x0008 
	__int32 height; //0x000C 
	float fovY; //0x0010
	float fovX;
	float viewOrigin[3]; //0x0018 
	float viewAxis[12]; //0x0024 
	__int32 time; //0x0048 
	__int32 rdflags; //0x004C 
	char unk[8];
	float yaw;
	float pitch;
};//Size=0x0050