
#include <base/vmath.h>
#include <base/system.h>
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
		//dbg_msg("script", "error: %s", lua_tostring(m_pLua, -1));
		//LF_ErrorFunc(m_pLua);
	}

	if(Before != m_Mem_Calls)
		dbg_msg("script", "Warning: %s called the memory allocator %u times", pFunctionName, m_Mem_Calls - Before);
}

void CScripting_SnapshotTypes::GetSnapItemTable(CScriptHost *pHost, int iType)
{
	lua_State *pLua = pHost->Lua();
	lua_getglobal(pLua, "__snaps");
	lua_rawgeti(pLua, -1, iType);
	CSnapType *pSnapType = &m_aSnapTypes[iType];
	pSnapType->m_LastUsedIndex = (pSnapType->m_LastUsedIndex+1) % NUM_SNAPOBJECTS;
	lua_rawgeti(pLua, -1, pSnapType->m_LastUsedIndex);
	lua_remove(pLua, -2); // remove objects table
	lua_remove(pLua, -2); // remove __snaps
}

void CScripting_SnapshotTypes::CreateSnapItemTable(CScriptHost *pHost, int iType)
{
	lua_State *pLua = pHost->Lua();

	lua_newtable(pLua); // object table

	// set the type
	lua_pushinteger(pLua, iType);
	lua_setfield(pLua, -2, "_type");

	// register the fields
	const CSnapType *pType = &m_aSnapTypes[iType];
	for(int i = 0; i < pType->m_NumFields; i++)
	{
		lua_pushnumber(pLua, 0);
		lua_setfield(pLua, -2, pType->m_aFields[i].m_aName);
	}
}

int CScripting_SnapshotTypes::LF_Snap_RegisterItemType(CScriptHost *pHost, void *pData)
{
	CScripting_SnapshotTypes *pThis = (CScripting_SnapshotTypes *)pData;
	lua_State *pLua = pHost->Lua();

	int iType = pThis->m_NumSnapTypes++;
	int iField = 0;

	//printf("%d\n", iType);

	lua_pushnil(pLua); //
	while(lua_next(pLua, -2) != 0)
	{
		// -2 == key 
		// -1 == value (table)
		CSnapType::CField *pField = &pThis->m_aSnapTypes[iType].m_aFields[iField];

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


		// pop value
		iField++;
		lua_pop(pLua, 1);
	}

	pThis->m_aSnapTypes[iType].m_NumFields = iField;

	// do a register function for this
	lua_getglobal(pLua, "__snaps");
		lua_newtable(pLua); // table of objects
			for(unsigned k = 0; k < NUM_SNAPOBJECTS; k++)
			{
				pThis->CreateSnapItemTable(pHost, iType);
				lua_rawseti(pLua, -2, k); // __snaps[iType] = table of objects
			}
			lua_rawseti(pLua, -2, iType); // __snaps[iType] = table of objects
	lua_pop(pLua, 1);

	/*
	const char *pResourceName = lua_tostring(pLua, 1);
	IResource *pRes = pThis->m_pResources->GetResourceByName(pResourceName);

	lua_pushlightuserdata(pLua, GenerateLightData(TYPE_RESOURCE, pRes->Slot().Slot()));
	*/
	lua_pushinteger(pLua, iType);
	return 1;
}

void CScripting_SnapshotTypes::Register(CScriptHost *pHost)
{
	mem_zero(m_aSnapTypes, sizeof(m_aSnapTypes));
	m_NumSnapTypes = 1; // we start at 1

	pHost->RegisterFunction("Snap_RegisterItemType", LF_Snap_RegisterItemType, this);

	lua_newtable(pHost->Lua()); // create snaps table
	lua_setglobal(pHost->Lua(), "__snaps");
}


#include <engine/client.h>

void CScripting_SnapshotClient::FillSnapItem(CScriptHost *pHost, int iType, const int *pData, int Count)
{
	lua_State *pLua = pHost->Lua();
	CScripting_SnapshotTypes::CSnapType *pSnapType = &m_pScriptingSnapshotTypes->m_aSnapTypes[iType];

	for(int i = 0; i < Count; i++)
	{
		const char *pFieldName = pSnapType->m_aFields[i].m_aName;

		lua_pushlstring(pLua, pFieldName, str_length(pFieldName));
		lua_pushnumber(pLua, pData[i] / pSnapType->m_aFields[i].m_Scale);

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
		pThis->m_pScriptingSnapshotTypes->GetSnapItemTable(pHost, Item.m_Type);
		pThis->FillSnapItem(pHost, Item.m_Type, pOldItemData, Item.m_DataSize/4);
	}
	else
		lua_pushnil(pLua);			

	if(pItemData)
	{
		pThis->m_pScriptingSnapshotTypes->GetSnapItemTable(pHost, Item.m_Type);
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
