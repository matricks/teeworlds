#include <engine/shared/protocol.h>
#include <engine/graphics.h>
#include <game/scripting.h>

#include "gameclient_lua.h"

class CUnpacker;

class CServerScripting
{
public:
	IClient *m_pClient;

	CScriptHost m_Script;
	CScripting_Resources m_Scripting_Resources;
	CScripting_Graphics m_Scripting_Graphics;
	CScripting_SnapshotTypes m_Scripting_SnapshotTypes;
	CScripting_SnapshotClient m_Scripting_SnapshotClient;
	CScripting_Physics m_Scripting_Physics;
	CScripting_Messaging m_Scripting_Messaging;
	CScripting_Input m_Scripting_Input;
		
	void Reset(IKernel *pKernel)
	{
		m_pClient = pKernel->RequestInterface<IClient>();

		m_Script.Reset();
		m_Scripting_Resources.Register(&m_Script, pKernel->RequestInterface<IResources>());
		m_Scripting_Graphics.Register(&m_Script, &m_Scripting_Resources, pKernel->RequestInterface<IGraphics>());
		m_Scripting_SnapshotTypes.Register(&m_Script);
		m_Scripting_SnapshotClient.Register(&m_Script,  m_pClient, &m_Scripting_SnapshotTypes);
		m_Scripting_Physics.Register(&m_Script);
		m_Scripting_Messaging.Register(&m_Script, m_pClient, NULL);
		m_Scripting_Input.Register(&m_Script, m_pClient, NULL);

		UpdateVariables();

		m_Script.SetVariableInt("client", 1);
		m_Script.DoFile("data/games/teeworlds/client.lua");
	}

	void UpdateVariables()
	{
		m_Script.SetVariableInt("time_prevgametick", m_pClient->PrevGameTick());
		m_Script.SetVariableInt("time_gametick", m_pClient->GameTick());
		m_Script.SetVariableFloat("time_intragametick", m_pClient->IntraGameTick());
		m_Script.SetVariableFloat("time_gameticktime", m_pClient->GameTickTime());
		m_Script.SetVariableFloat("time_localtime", m_pClient->LocalTime());
		m_Script.SetVariableFloat("time_servertickspeed", SERVER_TICK_SPEED);
	}
};

class CGameClient_Lua *gs_TEMP_pLuaClient = 0;

class CGameClient_Lua : public IGameClient
{
	MACRO_INTERFACE("gameclient_lua", 1)
public:
	CServerScripting m_ServerScript;

	virtual void OnConsoleInit()
	{
	}

	virtual void OnRconLine(const char *pLine)
	{
	}

	virtual void OnInit()
	{
		gs_TEMP_pLuaClient = this;
		m_ServerScript.Reset(Kernel());
	}

	virtual void OnNewSnapshot()
	{
	}

	virtual void OnEnterGame()
	{
	}

	virtual void OnShutdown()
	{
	}

	virtual void OnRender()
	{
	}

	virtual void OnStateChange(int NewState, int OldState)
	{
	}

	virtual void OnConnected()
	{
	}

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker)
	{
	}

	virtual void OnPredict()
	{
	}

	virtual void OnActivateEditor()
	{
	}

	virtual int OnSnapInput(int *pData)
	{
		return 0;
	}

	virtual const char *GetItemName(int Type) { return "unknown"; }
	virtual const char *Version() { return "lua1"; }
	virtual const char *NetVersion() { return "lua1"; }
};



void SCRIPT_TEMP_OnRender()
{
	gs_TEMP_pLuaClient->m_ServerScript.UpdateVariables();
	gs_TEMP_pLuaClient->m_ServerScript.m_Script.Call("OnRender", "");
}

IGameClient *CreateGameClientLua(IKernel *pKernel)
{
	CGameClient_Lua *pClient = new CGameClient_Lua();
	pKernel->RegisterInterface(pClient);
	return pClient;
}

/*


		// LAZY ASS HOT RELOAD :D
		{
			static uint64 last_timestamp = 0;
			uint64 t = io_timestamp("data/base.lua");
			if(t != last_timestamp)
			{
				ServerScript_Reset();
				last_timestamp = t;
			}
		}

		if(Input()->KeyPressed(KEY_LCTRL) && Input()->KeyPressed(KEY_LSHIFT) && Input()->KeyDown('r'))
			ServerScript_Reset();



static CClient *g_TEMP_pClient = 0;

void CClient::ServerScript_UpdateTimeVariables()
{
	m_ServerScripting.SetVariableInt("time_prevgametick", PrevGameTick());
	m_ServerScripting.SetVariableInt("time_gametick", GameTick());
	m_ServerScripting.SetVariableFloat("time_intragametick", IntraGameTick());
	m_ServerScripting.SetVariableFloat("time_gameticktime", GameTickTime());
	m_ServerScripting.SetVariableFloat("time_localtime", LocalTime());
	m_ServerScripting.SetVariableFloat("time_servertickspeed", SERVER_TICK_SPEED);
}
	
void CClient::ServerScript_Reset()
{

}


void SCRIPT_TEMP_OnRender()
{
	g_TEMP_pClient->ServerScript_UpdateTimeVariables();
	m_ServerScripting.Call("OnRender", "");
}*/