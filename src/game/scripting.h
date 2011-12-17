#include <stdlib.h> // ugly, it's for size_t
#include <base/tl/array.h>

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

	int m_CallNumArgs;
	const char *m_pCallFunctionName;
public:
	CScriptHost();
	lua_State *Lua() { return m_pLua; }

	void Reset();
	void RunGC();

	void DoFile(const char *pFilename);

	void Error(const char *pFormat, ...); // will cause a long jump
	void Assert(bool Check);

	typedef int (*SCRIPTFUNC)(CScriptHost *pHost, void *pData);

	void RegisterFunction(const char *pName, SCRIPTFUNC pfnFunc, void *pValue);

	void SetVariableDouble(const char *pName, double Value);
	void SetVariableInt(const char *pName, int Value);
	void SetVariableFloat(const char *pName, float Value);

	void CallSetup(const char *pFunctionName);
	void CallIntegerArg(int Value);
	void CallFloatArg(float Value);
	void CallCustomArg() { m_CallNumArgs++; }
	void CallPerform();

	void Call(const char *pFunctionName, const char *pArgs, ...);
};
/*
class CResource_Script : public IResource
{
	char *m_pSource;
public:
	CResource_Script()
	{
		m_pSource = 0;
	}

	virtual ~CResource_Script()
	{
		delete [] m_pSource;
	}
};

class CResourceHandler_Scripting : public IResources::IHandler
{
public:
	virtual IResource *Create(IResources::CResourceId Id) { return new CResource_Script(); }
	virtual bool Load(IResource *pResource, void *pData, unsigned DataSize)
	{
		CResource_Script 
	}

	virtual bool Insert(IResource *pResource);
	virtual bool Destroy(IResource *pResource);
};*/

class CObjectTypes
{
public:
	class CType
	{
	public:
		CType()
		{
			m_NumFields = 0;
			m_LastUsedIndex = 0;
		}

		enum
		{
			TYPE_UNKNOWN = 0,
			TYPE_INT,
			TYPE_FLOAT,
			TYPE_STRING,
		};

		class CField
		{
		public:
			int m_Type;
			float m_Scale;
			char m_aName[32];
		};

		CField m_aFields[32];
		int m_NumFields;

		int m_LastUsedIndex;
	};

	enum
	{
		CACHE_SIZE = 16,
	};

	array<CType*> m_lpTypes;
	const char *m_pCacheTableName;
	
	//void GetObjectTable(CScriptHost *pHost, int iType);
	int RegisterType(CScriptHost *pHost);
public:
	void Reset() { m_lpTypes.delete_all(); }
	void Register(CScriptHost *pHost, const char *pCacheTableName);
	int RegisterObjectType(CScriptHost *pHost);

	CType *GetType(int iType) { return iType >= 0 && iType < m_lpTypes.size() ? m_lpTypes[iType] : 0x0; }
	void PushCachedTable(CScriptHost *pHost, int iType);
	void PushNewTable(CScriptHost *pHost, int iType);

	void PushCachedTableWithData(CScriptHost *pHost, int iType, const int *pData, int Count);
};

class IClient;
class IServer;

class CScripting_Messaging
{
public:
	IClient *m_pClient;
	IServer *m_pServer;
	CObjectTypes m_Types;
	static int LF_Msg_Register(CScriptHost *pHost, void *pData);
	static int LF_Msg_Create(CScriptHost *pHost, void *pData);
	static int LF_Msg_Send(CScriptHost *pHost, void *pData);
public:
	CObjectTypes::CType *GetType(int iType) { return m_Types.GetType(iType); }
	void Register(CScriptHost *pHost, IClient *pClient, IServer *pServer);
};

class CScripting_SnapshotTypes
{
public:
	CObjectTypes m_Types;
	static int LF_Snap_RegisterItemType(CScriptHost *pHost, void *pData);
public:
	CObjectTypes::CType *GetType(int iType) { return m_Types.GetType(iType); }
	void Register(CScriptHost *pHost);
};

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


class CScripting_SnapshotServer
{
	CScripting_SnapshotTypes *m_pScriptingSnapshotTypes;
	IServer *m_pServer;

	static int LF_Snap_NewId(CScriptHost *pHost, void *pData); // GC takes care of freeing it?
	//static int LF_Snap_FreeId(CScriptHost *pHost, void *pData); // GC:ed?
	static int LF_Snap_CreateItem(CScriptHost *pHost, void *pData); // itemtype, id
	static int LF_Snap_CommitItem(CScriptHost *pHost, void *pData); // item
public:
	void Register(CScriptHost *pHost, IServer *pServer, CScripting_SnapshotTypes *pScriptingSnapshotTypes);
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
		return (void *)(size_t)(Type | (Index&LD_INDEXMASK));
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

struct CMapItemLayerTilemap;
class CTile;

class CScripting_Map
{
	CMapItemLayerTilemap *m_pTileMap;
	CTile *m_pTiles;
	static int LF_Map_GetSize(CScriptHost *pHost, void *pData);
	static int LF_Map_GetTile(CScriptHost *pHost, void *pData);
public:
	void Register(CScriptHost *pHost, CMapItemLayerTilemap *pTileMap, CTile *pTiles);
};

class CScripting_Input
{
public:
	IClient *m_pClient;
	IServer *m_pServer;
	CObjectTypes m_Types;
	static int LF_Input_Register(CScriptHost *pHost, void *pData);
	static int LF_Input_Create(CScriptHost *pHost, void *pData);
public:
	CObjectTypes::CType *GetType(int iType) { return m_Types.GetType(iType); }
	void Register(CScriptHost *pHost, IClient *pClient, IServer *pServer);
	void PushInput(CScriptHost *pHost, const int *pData, int Count);
};
