#include <base/vmath.h>
#include <base/system.h>
#include <engine/message.h>
#include "scripting.h"

extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

void *CScriptHost::LuaAllocator(void *pUser, void *pPtr, size_t OldSize, size_t NewSize)
{
	CScriptHost *pThis = (CScriptHost *)pUser;
	pThis->m_Mem_Calls++;
	(void)OldSize;  /* not used */
	if (NewSize == 0)
	{
		pThis->m_Mem_NumFree++;
		free(pPtr);
		return NULL;
	}
	else
	{
		if(pPtr)
			pThis->m_Mem_NumReallocs++;
		else
			pThis->m_Mem_NumAllocs++;
		return realloc(pPtr, NewSize);
	}
}

CScriptHost *CScriptHost::GetThis(lua_State *pLua)
{
	CScriptHost *pThis;
	lua_getallocf(pLua, (void**)&pThis);
	return pThis;
}

/* ** */
void CScriptHost::PrintLuaValue(lua_State *L, int i)
{
	if(lua_type(L,i) == LUA_TNIL)
		printf("nil");
	else if(lua_type(L,i) == LUA_TSTRING)
		printf("'%s'", lua_tostring(L,i));
	else if(lua_type(L,i) == LUA_TNUMBER)
		printf("%f", lua_tonumber(L,i));
	else if(lua_type(L,i) == LUA_TBOOLEAN)
	{
		if(lua_toboolean(L,i))
			printf("true");
		else
			printf("false");
	}
	else if(lua_type(L,i) == LUA_TTABLE)
	{
		printf("{...}");
	}
	else
		printf("%p (%s (%d))", lua_topointer(L,i), lua_typename(L,lua_type(L,i)), lua_type(L,i));
}


/* error function */
int CScriptHost::LF_ErrorFunc(lua_State *L)
{
	int depth = 0;
	//int frameskip = 0;
	lua_Debug frame;

	if(true) //session.report_color)
		printf("\033[01;31m%s\033[00m\n", lua_tostring(L,-1));
	else
		printf("%s\n", lua_tostring(L,-1));
	
	if(true) //session.lua_backtrace)
	{
		printf("backtrace:\n");
		while(lua_getstack(L, depth, &frame) == 1)
		{
			depth++;
			
			lua_getinfo(L, "nlSf", &frame);

			/* check for functions that just report errors. these frames just confuses more then they help */
			/*if(frameskip && str_comp(frame.short_src, "[C]") == 0 && frame.currentline == -1)
				continue;
			frameskip = 0;*/
			
			/* print stack frame */
			printf("  %s(%d): %s %s\n", frame.short_src, frame.currentline, frame.name, frame.namewhat);
			
			/* print all local variables for the frame */
			if(true) //session.lua_locals)
			{
				int i;
				const char *name = 0;
				
				i = 1;
				while((name = lua_getlocal(L, &frame, i)) != NULL)
				{
					printf("    %s = ", name);
					PrintLuaValue(L,-1);
					printf("\n");
					lua_pop(L,1);
					i++;
				}
				
				i = 1;
				while((name = lua_getupvalue(L, -1, i)) != NULL)
				{
					printf("    upvalue #%d: %s ", i-1, name);
					PrintLuaValue(L, -1);
					lua_pop(L,1);
					i++;
				}
			}
		}
	}
	
	return 1;
}


void CScriptHost::Error(const char *pFormat, ...)
{
	char aBuffer[1024];
	va_list ap;
	va_start(ap, pFormat);
#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuffer, sizeof(aBuffer), pFormat, ap);
#else
	vsnprintf(aBuffer, sizeof(aBuffer), pFormat, ap);
#endif
	va_end(ap);

	aBuffer[sizeof(aBuffer)-1] = 0;

	lua_pushlstring(m_pLua, aBuffer, str_length(aBuffer));
	lua_error(m_pLua);
}

void CScriptHost::Assert(bool Check)
{
	if(Check)
		return;

	int Top = lua_gettop (m_pLua);
	for(int i = 0; i < Top; i++)
	{
		printf("#%3d ", i);
		PrintLuaValue(m_pLua, i);
	}
	dbg_assert(false, "script assert");
}

CScriptHost::CScriptHost()
{
	m_pLua = 0;
}

void CScriptHost::Reset()
{
	m_Mem_NumAllocs = 0;
	m_Mem_NumReallocs = 0;
	m_Mem_NumFree = 0;

	if(m_pLua)
	{
		lua_close(m_pLua);
		m_pLua = 0;
	}

	m_pLua = lua_newstate(LuaAllocator, this);
	lua_checkstack(m_pLua, 128);

	lua_atpanic(m_pLua, LF_ErrorFunc);
	luaopen_base(m_pLua);
	luaopen_math(m_pLua);

	lua_register(m_pLua, "__errorfunc", LF_ErrorFunc);

	// kill everything that touches disc
	lua_pushnil(m_pLua); lua_setglobal(m_pLua, "load");
	lua_pushnil(m_pLua); lua_setglobal(m_pLua, "loadfile");
	lua_pushnil(m_pLua); lua_setglobal(m_pLua, "loadstring");
	lua_pushnil(m_pLua); lua_setglobal(m_pLua, "module");
	lua_pushnil(m_pLua); lua_setglobal(m_pLua, "require");

	// allow this for now
	//lua_pushnil(m_pLua); lua_setglobal(m_pLua, "dofile"); 

	// register functions
	lua_newtable(m_pLua); // create snaps table
	lua_setglobal(m_pLua, "engine");
}

void CScriptHost::RunGC()
{
	lua_gc(m_pLua, LUA_GCCOLLECT, 0);
}

void CScriptHost::DoFile(const char *pFilename)
{
	//
	if(luaL_dofile(m_pLua, pFilename) != 0)
	{
#define MACRO_ERROR_STR(x) "\033[01;31m" x "\033[00m"
		dbg_msg("script", MACRO_ERROR_STR("error loading script"));
		dbg_msg("script", MACRO_ERROR_STR("error: %s"), lua_tostring(m_pLua, -1));
	}
}


int CScriptHost::LF_Wrapper(lua_State *pLua)
{
	CScriptHost *pThis = GetThis(pLua);
	SCRIPTFUNC pfnFunc = (SCRIPTFUNC)lua_topointer(pLua, lua_upvalueindex(1));
	const void *pData = lua_topointer(pLua, lua_upvalueindex(2));
	return pfnFunc(pThis, (void *)pData);
}

void CScriptHost::RegisterFunction(const char *pName, SCRIPTFUNC pfnFunc, void *pValue)
{
	lua_getglobal(m_pLua, "engine");
		lua_pushlightuserdata(m_pLua, (void*)pfnFunc);
		lua_pushlightuserdata(m_pLua, pValue);
		lua_pushcclosure(m_pLua, LF_Wrapper, 2);
		lua_setfield(m_pLua, -2, pName);
	lua_pop(m_pLua, 1);
}

void CScriptHost::SetVariableDouble(const char *pName, double Value)
{
	lua_getglobal(m_pLua, "engine");
	lua_pushnumber(m_pLua, Value);
	lua_setfield(m_pLua, -2, pName);
	lua_pop(m_pLua, 1);		
}

void CScriptHost::SetVariableInt(const char *pName, int Value)
{
	SetVariableDouble(pName, (double)Value);
}

void CScriptHost::SetVariableFloat(const char *pName, float Value)
{
	SetVariableDouble(pName, (double)Value);
}

#include <stdarg.h>

void CScriptHost::Call(const char *pFunctionName, const char *pArgs, ...)
{
	unsigned Before = m_Mem_Calls;

	// call global on render function
	lua_getglobal(m_pLua, "__errorfunc");
	lua_getglobal(m_pLua, pFunctionName);
	int NumArgs = 0;

	{
		va_list ArgList;

		va_start(ArgList, pArgs);
		for(int i = 0; pArgs[i]; i++)
		{
			if(pArgs[i] == 'i')
				lua_pushinteger(m_pLua, va_arg(ArgList, int));
			else if(pArgs[i] == 'f')
				lua_pushnumber(m_pLua, va_arg(ArgList, double));
			else
			{
				dbg_assert(false, "invalid type");
			}
			NumArgs++;
		}

		va_end(ArgList);
	}


	if(lua_pcall(m_pLua, NumArgs, 0, -2 - NumArgs) != 0)
	{
		dbg_msg("script", "error calling: %s\n\t%s", pFunctionName, lua_tostring(m_pLua, -1));
		//LF_ErrorFunc(m_pLua);
	}

	if(Before != m_Mem_Calls)
		dbg_msg("script", "Warning: %s called the memory allocator %u times", pFunctionName, m_Mem_Calls - Before);
}


/*******************************/


void CObjectTypes::PushNewTable(CScriptHost *pHost, int iType)
{
	lua_State *pLua = pHost->Lua();

	lua_newtable(pLua); // object table

	// set the type
	lua_pushinteger(pLua, iType);
	lua_setfield(pLua, -2, "_type");

	// register the fields
	const CType *pType = m_lpTypes[iType];
	for(int i = 0; i < pType->m_NumFields; i++)
	{
		lua_pushnumber(pLua, 0);
		lua_setfield(pLua, -2, pType->m_aFields[i].m_aName);
	}
}

int CObjectTypes::RegisterType(CScriptHost *pHost)
{
	lua_State *pLua = pHost->Lua();

	CType *pType = new CType;
	int iType = m_lpTypes.add(pType);
	int iField = 0;

	lua_pushnil(pLua); //
	while(lua_next(pLua, -2) != 0)
	{
		// -2 == key 
		// -1 == value (table)
		CType::CField *pField = &pType->m_aFields[iField];

		// get name
		lua_getfield(pLua, -1, "name");
		str_copy(pField->m_aName, lua_tostring(pLua, -1), sizeof(pField->m_aName));
		lua_pop(pLua, 1);

		// get scale
		pField->m_Scale = 1.0f;
		lua_getfield(pLua, -1, "scale");
		if(lua_isnumber(pLua, -1))
			pField->m_Scale = lua_tonumber(pLua, -1);
		lua_pop(pLua, 1);

		// get type
		pField->m_Type = CType::TYPE_UNKNOWN;
		/*
		lua_getfield(pLua, -1, "type");
		if(lua_isnumber(pLua, -1))
			pField->m_Scale = lua_tointe(pLua, -1);
		lua_pop(pLua, 1);
		*/

		// pop value
		iField++;
		lua_pop(pLua, 1);
	}

	pType->m_NumFields = iField;

	// do a register function for this
	lua_getglobal(pLua, m_pCacheTableName);
		lua_newtable(pLua); // table of objects
			for(unsigned k = 0; k < CACHE_SIZE; k++)
			{
				PushNewTable(pHost, iType);
				lua_rawseti(pLua, -2, k); // __snaps[iType] = table of objects
			}
		lua_rawseti(pLua, -2, iType); // __snaps[iType] = table of objects
	lua_pop(pLua, 1);

	/*
	const char *pResourceName = lua_tostring(pLua, 1);
	IResource *pRes = pThis->m_pResources->GetResourceByName(pResourceName);

	lua_pushlightuserdata(pLua, GenerateLightData(TYPE_RESOURCE, pRes->Slot().Slot()));
	*/
	//lua_pushinteger(pLua, iType);
	return iType;
}


void CObjectTypes::PushCachedTable(CScriptHost *pHost, int iType)
{
	lua_State *pLua = pHost->Lua();
	lua_getglobal(pLua, m_pCacheTableName); // TODO: name
	lua_rawgeti(pLua, -1, iType);
	CType *pType = m_lpTypes[iType];
	pType->m_LastUsedIndex = (pType->m_LastUsedIndex+1) % CACHE_SIZE;
	lua_rawgeti(pLua, -1, pType->m_LastUsedIndex);
	lua_remove(pLua, -2); // remove objects table
	lua_remove(pLua, -2); // remove __snaps

	pHost->Assert(lua_istable(pLua, -1));
	
}

void CObjectTypes::Register(CScriptHost *pHost, const char *pCacheTableName)
{
	m_pCacheTableName = pCacheTableName;
	m_lpTypes.delete_all();

	lua_newtable(pHost->Lua()); // create cache table
	lua_setglobal(pHost->Lua(), m_pCacheTableName);
}


/*******************************/

int CScripting_SnapshotTypes::LF_Snap_RegisterItemType(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotTypes *pThis = (CScripting_SnapshotTypes *)pData;
	lua_pushinteger(pHost->Lua(), pThis->m_Types.RegisterType(pHost));
	return 1;
};

void CScripting_SnapshotTypes::Register(CScriptHost *pHost)
{
	pHost->RegisterFunction("Snap_RegisterItemType", LF_Snap_RegisterItemType, this);
	m_Types.Register(pHost, "__snapshottypes");

	// nr 0 first is invalid
	lua_newtable(pHost->Lua());
	m_Types.RegisterType(pHost);
	lua_pop(pHost->Lua(), 1);
}

/*******************************/

int CScripting_Messaging::LF_Msg_Register(CScriptHost *pHost, void *pData)
{
	CScripting_Messaging *pThis = (CScripting_Messaging *)pData;
	lua_pushinteger(pHost->Lua(), pThis->m_Types.RegisterType(pHost));
	return 1;
}

int CScripting_Messaging::LF_Msg_Create(CScriptHost *pHost, void *pData)
{
	CScripting_Messaging *pThis = (CScripting_Messaging *)pData;
	pThis->m_Types.PushCachedTable(pHost, lua_tointeger(pHost->Lua(), 1));
	return 1;
}

#include <engine/client.h>
#include <engine/server.h>
#include <engine/shared/protocol.h>

// Msg_Send(msg, [client])
int CScripting_Messaging::LF_Msg_Send(CScriptHost *pHost, void *pData)
{
	CScripting_Messaging *pThis = (CScripting_Messaging *)pData;

	lua_getfield(pHost->Lua(), 1, "_type");
	int TypeId = lua_tointeger(pHost->Lua(), -1);
	lua_pop(pHost->Lua(), 1);

	CObjectTypes::CType *pType = pThis->m_Types.GetType(TypeId);
	if(!pType)
		return 0;

	dbg_msg("script", "sending message %d", TypeId);

	// build message
	CMsgPacker Packer(TypeId);
	for(int i = 0; i < pType->m_NumFields; i++)
	{
		CObjectTypes::CType::CField *pField = &pType->m_aFields[i];
		
		lua_getfield(pHost->Lua(), 1, pField->m_aName);
		if(pField->m_Type == CObjectTypes::CType::TYPE_INT)
		{
			dbg_msg("script", "\t%s = %d", pField->m_aName, lua_tointeger(pHost->Lua(), -1));
			Packer.AddInt(lua_tointeger(pHost->Lua(), -1));
		}
		else if(pField->m_Type == CObjectTypes::CType::TYPE_STRING)
		{
			dbg_msg("script", "\t%s = '%s'", pField->m_aName, lua_tolstring(pHost->Lua(), -1, NULL));
			Packer.AddString(lua_tolstring(pHost->Lua(), -1, NULL), 128); // TODO: limit for now
		}
		else
			pHost->Error("Field '%s' has unsupported type %d", pField->m_aName, pField->m_Type); // TODO: improve error message
		
		lua_pop(pHost->Lua(), 1);
	}

	if(pThis->m_pClient)
	{
		pThis->m_pClient->SendMsg(&Packer, MSGFLAG_VITAL);
	}
	else if(pThis->m_pServer)
	{
		int ClientId = lua_tointeger(pHost->Lua(), 2);
		pThis->m_pServer->SendMsg(&Packer, MSGFLAG_VITAL, ClientId);
	}
	else
	{
		// error here
	}

		/*

		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 0;
		Msg.m_ClientID = -1;
		Msg.m_pMessage = pText;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);

	template<class T>
	int SendPackMsg(T *pMsg, int Flags, int ClientID)
	{
		CMsgPacker Packer(pMsg->MsgID());
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags, ClientID);
	}*/

	//IClient *m_pClient;
	//IServer *m_pServer;

	return 0;
}

void CScripting_Messaging::Register(CScriptHost *pHost, IClient *pClient, IServer *pServer)
{
	m_pClient = pClient;
	m_pServer = pServer;

	// TODO: check that we have either server or client, not both, and not neither of em

	pHost->RegisterFunction("Msg_Create", LF_Msg_Create, this);
	pHost->RegisterFunction("Msg_Send", LF_Msg_Send, this);
	pHost->RegisterFunction("Msg_Register", LF_Msg_Register, this);
	m_Types.Register(pHost, "__messages");

	// nr 0 first is invalid
	lua_newtable(pHost->Lua());
	m_Types.RegisterType(pHost);
	lua_pop(pHost->Lua(), 1);
}

/*******************************/

#include <engine/client.h>

void CScripting_SnapshotClient::FillSnapItem(CScriptHost *pHost, int iType, const int *pData, int Count)
{
	lua_State *pLua = pHost->Lua();
	CObjectTypes::CType *pType = m_pScriptingSnapshotTypes->GetType(iType);
	if(!pType)
		return;

	for(int i = 0; i < Count; i++)
	{
		const char *pFieldName = pType->m_aFields[i].m_aName;

		lua_pushlstring(pLua, pFieldName, str_length(pFieldName));
		lua_pushnumber(pLua, pData[i] / pType->m_aFields[i].m_Scale);
		lua_rawset(pLua, -3);
	}
}

int CScripting_SnapshotClient::LF_Snap_NumItems(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotClient *pThis = (CScripting_SnapshotClient *)pData;
	lua_State *pLua = pHost->Lua();
	lua_pushinteger(pLua, pThis->m_pClient->SnapNumItems(IClient::SNAP_CURRENT));
	return 1;
}

int CScripting_SnapshotClient::LF_Snap_GetItem(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotClient *pThis = (CScripting_SnapshotClient *)pData;
	lua_State *pLua = pHost->Lua();

	int Idx = lua_tointeger(pLua, 1);
	
	IClient::CSnapItem Item;
	const int *pItemData = (const int *)pThis->m_pClient->SnapGetItem(IClient::SNAP_CURRENT, Idx, &Item);
	const int *pOldItemData = (const int *)pThis->m_pClient->SnapFindItem(IClient::SNAP_PREV, Item.m_Type, Item.m_ID);

	if(pOldItemData)
	{
		pThis->m_pScriptingSnapshotTypes->m_Types.PushCachedTable(pHost, Item.m_Type);
		pThis->FillSnapItem(pHost, Item.m_Type, pOldItemData, Item.m_DataSize/4);
	}
	else
		lua_pushnil(pLua);			

	if(pItemData)
	{
		pThis->m_pScriptingSnapshotTypes->m_Types.PushCachedTable(pHost, Item.m_Type);
		pThis->FillSnapItem(pHost, Item.m_Type, pItemData, Item.m_DataSize/4);
	}
	else
		lua_pushnil(pLua);
	return 2;
}

void CScripting_SnapshotClient::Register(CScriptHost *pHost, IClient *pClient, CScripting_SnapshotTypes *pScriptingSnapshotTypes)
{
	m_pClient = pClient;
	m_pScriptingSnapshotTypes = pScriptingSnapshotTypes;
	pHost->RegisterFunction("Snap_NumItems", LF_Snap_NumItems, this);
	pHost->RegisterFunction("Snap_GetItem", LF_Snap_GetItem, this);
}


int CScripting_SnapshotServer::LF_Snap_NewId(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotServer *pThis = (CScripting_SnapshotServer *)pData;
	lua_pushinteger(pHost->Lua(), pThis->m_pServer->SnapNewID());
	return 1;
}

//static int LF_Snap_FreeId(CScriptHost *pHost, void *pData); // GC:ed?
int CScripting_SnapshotServer::LF_Snap_CreateItem(CScriptHost *pHost, void *pData)
{
	// TODO: Error checking
	CScripting_SnapshotServer *pThis = (CScripting_SnapshotServer *)pData;
	pThis->m_pScriptingSnapshotTypes->m_Types.PushCachedTable(pHost, lua_tointeger(pHost->Lua(), 1));
	lua_pushinteger(pHost->Lua(), lua_tointeger(pHost->Lua(), 2));
	lua_setfield(pHost->Lua(), -2, "_itemid");
	return 1;
}

int CScripting_SnapshotServer::LF_Snap_CommitItem(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotServer *pThis = (CScripting_SnapshotServer *)pData;

	lua_getfield(pHost->Lua(), 1, "_type");
	int TypeId = lua_tointeger(pHost->Lua(), -1);
	lua_pop(pHost->Lua(), 1);

	lua_getfield(pHost->Lua(), 1, "_itemid");
	int SnapId = lua_tointeger(pHost->Lua(), -1);
	lua_pop(pHost->Lua(), 1);

	CObjectTypes::CType *pType = pThis->m_pScriptingSnapshotTypes->m_Types.GetType(TypeId);

	if(!pType)
		pHost->Error("unknown object type %d", TypeId);

	//dbg_msg("scripting", "%d %d %d", TypeId, SnapId, pType->m_NumFields);
	int *pItem = (int *)pThis->m_pServer->SnapNewItem(TypeId, SnapId, pType->m_NumFields*sizeof(int));

	// fill the item
	for(int i = 0; i < pType->m_NumFields; i++)
	{
		lua_getfield(pHost->Lua(), 1, pType->m_aFields[i].m_aName);
		float FloatValue = (float)lua_tonumber(pHost->Lua(), -1);
		int IntValue = (int)(FloatValue / pType->m_aFields[i].m_Scale);
		lua_pop(pHost->Lua(), 1);

		//dbg_msg("scripting", "%s %d", pType->m_aFields[i].m_aName, IntValue);
		pItem[i] = IntValue;
	}


	return 0;
}

void CScripting_SnapshotServer::Register(CScriptHost *pHost,  IServer *pServer, CScripting_SnapshotTypes *pScriptingSnapshotTypes)
{
	m_pServer = pServer;
	m_pScriptingSnapshotTypes = pScriptingSnapshotTypes;
	pHost->RegisterFunction("Snap_NewId", LF_Snap_NewId, this);
	pHost->RegisterFunction("Snap_CreateItem", LF_Snap_CreateItem, this);
	pHost->RegisterFunction("Snap_CommitItem", LF_Snap_CommitItem, this);
}


/*******************************/

int CScripting_Resources::LF_Resource_Get(CScriptHost *pHost, void *pData)
{
	CScripting_Resources *pThis = (CScripting_Resources *)pData;
	lua_State *pLua = pHost->Lua();

	const char *pResourceName = lua_tostring(pLua, 1);
	IResource *pRes = pThis->m_pResources->GetResourceByName(pResourceName);

	lua_pushlightuserdata(pLua, GenerateLightData(TYPE_RESOURCE, pRes->Slot().Slot()));
	return 1;
}

IResource *CScripting_Resources::ToResource(lua_State *pLua, int Index)
{
	ptrdiff_t Value = (ptrdiff_t)lua_topointer(pLua, 1);
	if((Value&LD_TYPEMASK) != TYPE_RESOURCE)
		return NULL;
	return m_pResources->GetResourceBySlot(CResourceSlot(Value&LD_INDEXMASK));
}
	
void CScripting_Resources::Register(CScriptHost *pHost, IResources *pResources)
{
	m_pResources = pResources;
	pHost->RegisterFunction("Resource_Get", LF_Resource_Get, this);
}


/*******************************/

#include <engine/graphics.h>

int CScripting_Graphics::LF_Graphics_DrawQuad(CScriptHost *pHost, void *pData)
{
	CScripting_Graphics *pThis = (CScripting_Graphics *)pData;
	lua_State *pLua = pHost->Lua();

	IResource *pTexture = pThis->m_pScriptResources->ToResource(pLua, 1);
	float x = lua_tonumber(pLua, 2);
	float y = lua_tonumber(pLua, 3);
	float w = lua_tonumber(pLua, 4);
	float h = lua_tonumber(pLua, 5);
	float r = lua_tonumber(pLua, 6);

	IGraphics *pGraphics = pThis->m_pGraphics;

	pGraphics->TextureSet(pTexture);	
	pGraphics->QuadsBegin();
	pGraphics->QuadsSetRotation(r);
	pGraphics->QuadsSetSubset(pThis->m_aUVs[0], pThis->m_aUVs[1], pThis->m_aUVs[2], pThis->m_aUVs[3]);
	IGraphics::CQuadItem QuadItem(x, y, w,h);
	pGraphics->QuadsDraw(&QuadItem, 1);
	pGraphics->QuadsEnd();
	return 0;
}

int CScripting_Graphics::LF_Graphics_ResetUV(CScriptHost *pHost, void *pData)
{
	CScripting_Graphics *pThis = (CScripting_Graphics *)pData;
	pThis->m_aUVs[0] = 0;
	pThis->m_aUVs[1] = 0;
	pThis->m_aUVs[2] = 1;
	pThis->m_aUVs[3] = 1;
	return 0;
}

int CScripting_Graphics::LF_Graphics_SetUV(CScriptHost *pHost, void *pData)
{
	CScripting_Graphics *pThis = (CScripting_Graphics *)pData;
	lua_State *pLua = pHost->Lua();
	pThis->m_aUVs[0] = lua_tonumber(pLua, 1);
	pThis->m_aUVs[1] = lua_tonumber(pLua, 2);
	pThis->m_aUVs[2] = lua_tonumber(pLua, 3);
	pThis->m_aUVs[3] = lua_tonumber(pLua, 4);
	return 0;
}

void CScripting_Graphics::Register(CScriptHost *pHost, CScripting_Resources *pScriptResources, IGraphics *pGraphics)
{
	m_pScriptResources = pScriptResources;
	m_pGraphics = pGraphics;
	pHost->RegisterFunction("Graphics_SetUV", LF_Graphics_SetUV, this);
	pHost->RegisterFunction("Graphics_ResetUV", LF_Graphics_ResetUV, this);
	pHost->RegisterFunction("Graphics_DrawQuad", LF_Graphics_DrawQuad, this);

	// talk about lazy
	LF_Graphics_ResetUV(pHost, this);
}


/*******************************/

extern bool SCRIPT_TEMP_Physics_CheckPoint(float x, float y);
extern void SCRIPT_TEMP_Physics_MoveBox(vec2 *pInoutPos, vec2 *pInoutVel, vec2 Size, float Elasticity);

int CScripting_Physics::LF_Physics_CheckPoint(CScriptHost *pHost, void *pData)
{
	lua_State *pLua = pHost->Lua();
	lua_pushboolean(pLua, SCRIPT_TEMP_Physics_CheckPoint(lua_tonumber(pLua, 1), lua_tonumber(pLua, 2)));
	return 1;
}

int CScripting_Physics::LF_Physics_MoveBox(CScriptHost *pHost, void *pData)
{
	lua_State *pLua = pHost->Lua();

	vec2 Pos, Vel, BoxSize;
	float Elast;
	Pos.x = lua_tonumber(pLua, 1);
	Pos.y = lua_tonumber(pLua, 2);
	Vel.x = lua_tonumber(pLua, 3);
	Vel.y = lua_tonumber(pLua, 4);
	BoxSize.x = lua_tonumber(pLua, 5);
	BoxSize.y = lua_tonumber(pLua, 6);
	Elast = lua_tonumber(pLua, 7);
	SCRIPT_TEMP_Physics_MoveBox(&Pos, &Vel, BoxSize, Elast);
	lua_pushnumber(pLua, Pos.x);
	lua_pushnumber(pLua, Pos.y);
	lua_pushnumber(pLua, Vel.x);
	lua_pushnumber(pLua, Vel.y);
	return 4;
}

// TODO: add the needed physics object
void CScripting_Physics::Register(CScriptHost *pHost) 
{
	pHost->RegisterFunction("Physics_CheckPoint", LF_Physics_CheckPoint, this);
	pHost->RegisterFunction("Physics_MoveBox", LF_Physics_MoveBox, this);
}	
