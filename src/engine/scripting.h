#include <stdlib.h> // ugly, it's for size_t

typedef struct lua_State lua_State;

class CScriptHost
{
	lua_State *m_pLua;

	unsigned m_Mem_Calls;
	unsigned m_Mem_NumAllocs;
	unsigned m_Mem_NumReallocs;
	unsigned m_Mem_NumFree;

	// Light data handling
	// we store type + index in a light data pointer inorder to do safe fetches
	enum
	{
		LD_TYPEMASK = 0xff000000,
		LD_INDEXMASK = 0xffffff,

		TYPE_RESOURCE = 0x01000000,
	};

	static CScriptHost *GetThis(lua_State *pLua);
	static void *GenerateLightData(unsigned Type, int Index);
	static void PrintLuaValue(lua_State *L, int i);

	static void *LuaAllocator(void *pUser, void *pPtr, size_t OldSize, size_t NewSize);

	/* error function */
	static int LF_ErrorFunc(lua_State *L);
	static int LF_Wrapper(lua_State *pLua);
public:
	CScriptHost();
	lua_State *Lua() { return m_pLua; }

	void Reset();

	void DoFile(const char *pFilename);

	typedef int (*SCRIPTFUNC)(CScriptHost *pHost, void *pData);

	void RegisterFunction(const char *pName, SCRIPTFUNC pfnFunc, void *pValue);

	void SetVariableDouble(const char *pName, double Value);
	void SetVariableInt(const char *pName, int Value);
	void SetVariableFloat(const char *pName, float Value);

	void Call(const char *pFunctionName);
};

class CScripting_SnapshotTypes
{
public:
	class CSnapType
	{
	public:
		class CField
		{
		public:
			float m_Scale;
			char m_aName[32];
		};

		CField m_aFields[32];
		int m_NumFields;

		int m_LastUsedIndex;
	};

	CSnapType m_aSnapTypes[64];
	int m_NumSnapTypes;
	
	enum
	{
		NUM_SNAPOBJECTS = 16, // how many tables of each snapshot item type we should create
	};

	void GetSnapItemTable(CScriptHost *pHost, int iType);
	void CreateSnapItemTable(CScriptHost *pHost, int iType);

	static int LF_Snap_RegisterItemType(CScriptHost *pHost, void *pData);
public:
	void Register(CScriptHost *pHost);
};

class IClient;

class CScripting_SnapshotClient
{
	CScripting_SnapshotTypes *m_pScriptingSnapshotTypes;
	IClient *m_pClient;

	void FillSnapItem(CScriptHost *pHost, int iType, const int *pData, int Count);

	static int LF_Snap_NumItems(CScriptHost *pHost, void *pData);
	static int LF_Snap_GetItem(CScriptHost *pHost, void *pData);
public:
	void Register(CScriptHost *pHost, IClient *pClient, CScripting_SnapshotTypes *pScriptingSnapshotTypes);
};


class IResources;
class IResource;


class CScripting_Resources
{
public:
	// Light data handling
	// we store type + index in a light data pointer inorder to do safe fetches
	// TODO: this shouldn't be here
	enum
	{
		LD_TYPEMASK = 0xff000000,
		LD_INDEXMASK = 0xffffff,

		TYPE_RESOURCE = 0x01000000,
	};
		
	IResources *m_pResources;

	static void *GenerateLightData(unsigned Type, int Index)
	{
		return (void *)(Type | (Index&LD_INDEXMASK));
	}

	static int LF_Resource_Get(CScriptHost *pHost, void *pData);
public:
	IResource *ToResource(lua_State *pLua, int Index);
		
	void Register(CScriptHost *pHost, IResources *pResources);
};

class IGraphics;

class CScripting_Graphics
{
	CScripting_Resources *m_pScriptResources; // this is a bit silly
	IGraphics *m_pGraphics;
	float m_aUVs[4];

//	m_pGraphics = m_pKernel->RequestInterface<IGraphics>();

	static int LF_Graphics_DrawQuad(CScriptHost *pHost, void *pData);
	static int LF_Graphics_ResetUV(CScriptHost *pHost, void *pData);
	static int LF_Graphics_SetUV(CScriptHost *pHost, void *pData);
public:
	void Register(CScriptHost *pHost, CScripting_Resources *pScriptResources, IGraphics *pGraphics);
};

class CScripting_Physics
{
	static int LF_Physics_CheckPoint(CScriptHost *pHost, void *pData);
	static int LF_Physics_MoveBox(CScriptHost *pHost, void *pData);
public:
	// TODO: add the needed physics object
	void Register(CScriptHost *pHost);
};
