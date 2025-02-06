#pragma once

#include <windows.h>
#include <cstdint>
#include <cmath>

using Any = uint64_t;
using Void = uint64_t;
using ScrHandle = int;

using AnimScene = int;
using Blip = int;
using Cam = int;
using Entity = ScrHandle;
using FireId = int;
using Hash = unsigned int;
using Interior = int;
using ItemSet = ScrHandle;
using Object = ScrHandle;
using Ped = ScrHandle;
using PersChar = ScrHandle;
using Pickup = int;
using Player = unsigned int;
using PopZone = int;
using Prompt = int;
using PropSet = int;
using Vehicle = ScrHandle;
using Volume = ScrHandle;

#define ALIGN8 __declspec(align(8))

struct Vector2
{
	ALIGN8 float x;
	ALIGN8 float y;
};

/*Halen84 (TuffyTown) Vector3 class https://github.com/Halen84/Vector3/blob/master/Vector3.h */
#pragma warning(disable:26495)
class Vector3
{
public:
	union
	{
		struct
		{
			ALIGN8 float x;
			ALIGN8 float y;
			ALIGN8 float z;
		};
		struct
		{
			ALIGN8 float X;
			ALIGN8 float Y;
			ALIGN8 float Z;
		};
	};

	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector3(float value) : x(value), y(value), z(value) {}
	Vector3(const Vector3& vec) : x(vec.x), y(vec.y), z(vec.z) {}

	bool operator!=(const Vector3& right) const { return x != right.x || y != right.y || z != right.z; }
	bool operator==(const Vector3& right) const { return !(*this != right); }

	Vector3 operator+(const Vector3& right) const { return Vector3(x + right.x, y + right.y, z + right.z); }
	Vector3 operator-(const Vector3& right) const { return Vector3(x - right.x, y - right.y, z - right.z); }
	Vector3 operator*(const Vector3& right) const { return Vector3(x * right.x, y * right.y, z * right.z); }
	Vector3 operator/(const Vector3& right) const { return Vector3(x / right.x, y / right.y, z / right.z); }
	Vector3& operator=(const Vector3& right) { x = right.x; y = right.y; z = right.z; return *this; }
	Vector3& operator+=(const Vector3& right) { x += right.x; y += right.y; z += right.z; return *this; }
	Vector3& operator-=(const Vector3& right) { x -= right.x; y -= right.y; z -= right.z; return *this; }
	Vector3& operator*=(const Vector3& right) { x *= right.x; y *= right.y; z *= right.z; return *this; }
	Vector3& operator/=(const Vector3& right) { x /= right.x; y /= right.y; z /= right.z; return *this; }

	Vector3 operator+(const float& right) const { return Vector3(x + right, y + right, z + right); }
	Vector3 operator-(const float& right) const { return Vector3(x - right, y - right, z - right); }
	Vector3 operator*(const float& right) const { return Vector3(x * right, y * right, z * right); }
	Vector3 operator/(const float& right) const { return Vector3(x / right, y / right, z / right); }
	Vector3& operator=(const float& right) { x = right; y = right; z = right; return *this; }
	Vector3& operator+=(const float& right) { x += right; y += right; z += right; return *this; }
	Vector3& operator-=(const float& right) { x -= right; y -= right; z -= right; return *this; }
	Vector3& operator*=(const float& right) { x *= right; y *= right; z *= right; return *this; }
	Vector3& operator/=(const float& right) { x /= right; y /= right; z /= right; return *this; }

	// Get the cross product of two vectors
	Vector3 Cross(const Vector3& right)
	{
		Vector3 result;
		result.x = y * right.z - z * right.y;
		result.y = z * right.x - x * right.z;
		result.z = x * right.y - y * right.x;
		return result;
	}

	// Get the length of the vector
	float Length()
	{
		return sqrtf((x * x) + (y * y) + (z * z));
	}

	// Get the length of the vector
	float Magnitude()
	{
		return Length();
	}

	// Get the dot product of two vectors
	float Dot(const Vector3& right)
	{
		return (x * right.x) + (y * right.y) + (z * right.z);
	}

	// Convert the vector to have a length of 1
	Vector3 Normalize()
	{
		float length = Length();
		if (length == 0.0f) return *this;

		Vector3 copy = *this;
		float norm = 1.0f / length;
		copy.x *= norm;
		copy.y *= norm;
		copy.z *= norm;

		return copy;
	}

	// Linearly interpolate between two vectors by amount t
Vector3 Lerp(const Vector3& from, const Vector3& to, float t)
{
    // Clamp t between 0.0f and 1.0f
    t = std::fmax(std::fmin(t, 1.0f), 0.0f);

    // Interpolate each component separately
    return Vector3(
        from.x + (to.x - from.x) * t,
        from.y + (to.y - from.y) * t,
        from.z + (to.z - from.z) * t
    );
}

	// Reflects a vector off the plane defined by a normal
	Vector3 Reflect(const Vector3& normal)
	{
		float d = Dot(normal);
		return Vector3(x - 2.0f * d * normal.x, y - 2.0f * d * normal.y, z - 2.0f * d * normal.z);
	}

	// Spherically interpolate between two vectors by amount t
	Vector3 Slerp(const Vector3& to, const Vector3& from, float t)
	{
		t = fmaxf(fminf(t, 1.0f), 0.0f);
		float dot = Dot(to);
		float theta = acosf(dot);
		float sinTheta = sinf(theta);

		// Vectors are parallel
		if (sinTheta == 0.0f) {
			return Lerp(to, from, t);
		}

		float w1 = sinf((1.0f - t) * theta) / sinTheta;
		float w2 = sinf(t * theta) / sinTheta;

		return Vector3(x * w1 + to.x * w2, y * w1 + to.y * w2, z * w1 + to.z * w2);
	}

	// Returns the distance between two vectors
	float Distance(const Vector3& other)
	{
		float dx = x - other.x;
		float dy = y - other.y;
		float dz = z - other.z;
		return sqrtf(dx * dx + dy * dy + dz * dz);
	}
};
#pragma warning(default:26495)

struct Vector4
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;
	ALIGN8 float w;
};
struct NotificationStruct {
	alignas(8) int  Duration;
	alignas(8) int  f1;
	alignas(8) int  f2;
	alignas(8) int  f3;
};
struct NotificationStruct2 {
	alignas(8) int  f0;
	alignas(8) const char* Title;
	alignas(8) const char* SubTitle;
	alignas(8) int  f3;
	alignas(8) Hash IconDict;
	alignas(8) Hash Icon;
	alignas(8) int  f6;
	alignas(8) int  f7;
};
struct HelpTextArgs1
{
	alignas(8) int p0;
	alignas(8) int p1;
	alignas(8) int p2;
	alignas(8) int p3;
};
struct HelpTextArgs2
{
	alignas(8) int p0;
	alignas(8) const char* p1;
};
struct HelpTextArgs3
{
	ALIGN8 int p0;
	ALIGN8  const char* p1;
	ALIGN8  const char* p2;
	ALIGN8  int p3;
};
struct HelpTextArgs4
{
	ALIGN8  int p0;
	ALIGN8  const char* p1;
	ALIGN8  const char* p2;
	ALIGN8  Hash p3;
	ALIGN8  int p4;
	ALIGN8  Hash p5;
	ALIGN8  int p6;
};
struct FeedData
{
	alignas(8) int duration; //how long to display the feed item for (milliseconds)
	alignas(8) const char* f_1; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
	alignas(8) const char* f_2; //Used with UIFEED::0xAFF5BE9BA496CE40, seems to set the background colour, untested.
	alignas(8) int f_3;
	alignas(8) int f_4;  //Seems to be a struct for the UIFEED::_0xAFF5BE9BA496CE40 native. 
	alignas(8) int f_5;
	alignas(8) const char* secondary_subtitle; //Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST. Displays another subtitle once the primary once has been displayed (from FeedInfo.subtitle).
	alignas(8) int f_7;
	alignas(8) int f_8;
	alignas(8) int f_9;
	alignas(8) int f_10;
	alignas(8) int f_11;
	alignas(8) int f_12;
};
struct FeedInfo
{
	alignas(8) int f_0;
	alignas(8) const char* title; //The title of the feed item.
	alignas(8) const char* subtitle; //the main subtitle of the feed item.
	alignas(8) const char* secondary_subtitle; //the subtitle that plays after the first subtitle.  Used with natives like UIFEED::_UI_FEED_POST_THREE_TEXT_SHARD
	alignas(8) int texture_dictionary_hash; //the texture dict hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
	alignas(8) int texture_name_hash; //the texture name hash. Used with natives like UIFEED::_UI_FEED_POST_SAMPLE_TOAST
	alignas(8) int f_6;
	alignas(8) int f_7;
};

//Researched by Halen84 (Tuffy) and Ked
struct SampleToastRightStruct1
{
	alignas(8) int Duration; //duration to display right toast (milliseconds).
	alignas(8) const char* SoundSet; // soundset for the sound to play.
	alignas(8) const char* SoundToPlay; // sound to play from soundset.
	alignas(8) int f_3; // unk.
};
//Researched by Halen84 (Tuffy) and Ked
struct SampleToastRightStruct2
{
	alignas(8) int f_0; // unk.
	alignas(8) const char* Title; // title or text that is displayed.
	alignas(8) const char* ImageDictionary; //the image dictionary for the image texture that is displayed.
	alignas(8) Hash ImageHash; // the image texture that is displayed.
	alignas(8) int BounceAmount; //the amount of bounce affect applied to the toast when displayed.
	alignas(8) Hash Color; //color of the text that is displayed.
	alignas(8) int f_6; // unk.
};

struct VolumeSphere
{
	float f_0;
	float f_1;
	float f_2;
	float s_0;
	float s_1;
	float s_2;
	float r_0;
	float r_1;
	float r_2;
};
struct VectorH
{
	ALIGN8 float x;
	ALIGN8 float y;
	ALIGN8 float z;
	ALIGN8 float h;
};
//struct UI_SCRIPT_EVENT
//{
//	alignas(8) eUIScriptEventType eventType;
//	alignas(8) int intParam;
//	alignas(8) Hash hashParam;
//	alignas(8) Hash datastoreParam;
//};

struct ScriptedSpeechParams
{
	const char* speechName;
	const char* voiceName;
	alignas(8) int variation;
	alignas(8) Hash speechParamHash;
	alignas(8) Ped listenerPed;
	alignas(8) BOOL syncOverNetwork;
	alignas(8) int v7;
};


struct date_time
{
	int year;
	int PADDING1;
	int month;
	int PADDING2;
	int day;
	int PADDING3;
	int hour;
	int PADDING4;
	int minute;
	int PADDING5;
	int second;
	int PADDING6;
};

struct StatId
{
	alignas(8) Hash BaseId;
	alignas(8) Hash PermutationId;
};
//struct g_lawData
//{
//	int _currentState = *getGlobalPtr(BASE + 4); // eLBS
//	Hash _currentLawRegion = *getGlobalPtr(BASE + 26);
//	float _lawSeeingRange = *getGlobalPtr(BASE + 69); // To be used with native LAW::_SET_LAW_SEEING_RANGE
//	Hash _mostRecentPursuitBountyState = *getGlobalPtr(BASE + 5);
//	Hash _currentCrime = *getGlobalPtr(BASE + 6);
//	int _currentBounty = *getGlobalPtr(BASE + 1381); // Seems to be inaccurate sometimes
//	int _currentRegion = *getGlobalPtr(BASE + 900);
//
//	struct wantedUIData
//	{
//		int eCurrentUIState = *getGlobalPtr(BASE + 78 + 59);
//	}wantedUIData;
//
//	// Global_1934266.f_78.f_62 = g_lawData.wantedUIData.??? (is a SET value for eCurrentUIState)
//	// Global_1934266.f_78.f_60 = g_lawData.wantedUIDate.??? (sub state)
//private:
//	static const int BASE = 1934266;
//};


static_assert(sizeof(Vector2) == 16, "");
static_assert(sizeof(Vector4) == 32, "");
static_assert(sizeof(VectorH) == 32, "");

struct RaycastResult
{
	int Result;
	BOOL DidHit;
	Entity HitEntity;
	Vector3 HitPosition;
	Vector3 SurfaceNormal;
	Hash MaterialHash;
};

//TuffyTown (Halen84) Inventory Structs https://www.rdr2mods.com/forums/topic/2139-addingremoving-items-to-inventory-using-natives/

struct sGuid
{
	alignas(8) int data1;
	alignas(8) int data2;
	alignas(8) int data3;
	alignas(8) int data4;
};

struct sSlotInfo
{
	alignas(8) sGuid guid;
	alignas(8) int f_1;
	alignas(8) int f_2;
	alignas(8) int f_3;
	alignas(8) int slotId;
};

struct sItemInfo
{
	alignas(8) int f_0;
	alignas(8) int f_1;
	alignas(8) int f_2;
	alignas(8) int f_3;
	alignas(8) int f_4;
	alignas(8) int f_5;
	alignas(8) int f_6;
};

struct DrivableVehicle {
	Vehicle VehicleWagon;
	Object VehicleBody;
	Object FrontLeftWheel;
	Object FrontRightWheel;
	Object RearLeftWheel;
	Object RearRightWheel;
	bool HeadLightsEnabled;
	Object LeftHeadlight;
	Object RightHeadlight;
	Object LeftHeadlightEmitter;
	Object RightHeadlightEmitter;
	bool BrakeLightsEnabled;
	Object LeftBrakeLight;
	Object RightBrakeLight;
	Object LeftBrakeLightEnabled;
	Object RightBrakeLightEnabled;
	Ped DriverClonePed;

	void SetLightsEnabled(DrivableVehicle* veh, bool enabled) {
		veh->HeadLightsEnabled = enabled;
		MessageBox(NULL, "value set", "TEST", NULL);
	}
};

enum eScriptedAnimFlags
{
	AF_LOOPING = (1 << 0),
	AF_HOLD_LAST_FRAME = (1 << 1),
	AF_NOT_INTERRUPTABLE = (1 << 2),
	AF_UPPERBODY = (1 << 3),
	AF_SECONDARY = (1 << 4),
	AF_ABORT_ON_PED_MOVEMENT = (1 << 5),
	AF_ADDITIVE = (1 << 6),
	AF_OVERRIDE_PHYSICS = (1 << 7),
	AF_EXTRACT_INITIAL_OFFSET = (1 << 8),
	AF_EXIT_AFTER_INTERRUPTED = (1 << 9),
	AF_TAG_SYNC_IN = (1 << 10),
	AF_TAG_SYNC_OUT = (1 << 11),
	AF_TAG_SYNC_CONTINUOUS = (1 << 12),
	AF_FORCE_START = (1 << 13),
	AF_USE_KINEMATIC_PHYSICS = (1 << 14),
	AF_USE_MOVER_EXTRACTION = (1 << 15),
	AF_DONT_SUPPRESS_LOCO = (1 << 16),
	AF_ENDS_IN_DEAD_POSE = (1 << 17),
	AF_ACTIVATE_RAGDOLL_ON_COLLISION = (1 << 18),
	AF_DONT_EXIT_ON_DEATH = (1 << 19),
	AF_ABORT_ON_WEAPON_DAMAGE = (1 << 20),
	AF_DISABLE_FORCED_PHYSICS_UPDATE = (1 << 21),
	AF_GESTURE = (1 << 22),
	AF_SKIP_IF_BLOCKED_BY_HIGHER_PRIORITY_TASK = (1 << 23),
	AF_USE_ABSOLUTE_MOVER = (1 << 24),
	AF_0xC57F16E7 = (1 << 25),
	AF_UPPERBODY_TAGS = (1 << 26),
	AF_PROCESS_ATTACHMENTS_ON_START = (1 << 27),
	AF_EXPAND_PED_CAPSULE_FROM_SKELETON = (1 << 28),
	AF_BLENDOUT_WRT_LAST_FRAME = (1 << 29),
	AF_DISABLE_PHYSICAL_ACTIVATION = (1 << 30),
	AF_DISABLE_RELEASE_EVENTS = (1 << 31),
	//AF_0x70F38514 = 32, // ?
};