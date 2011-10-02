#include <engine/shared/protocol.h>
#include <engine/server.h>
#include <game/scripting.h>

class CGameServer_Lua : public IGameServer
{
public:
	MACRO_INTERFACE("gameserver", 0)
protected:
	CScriptHost m_Script;
	CScripting_Resources m_Scripting_Resources;
	CScripting_SnapshotTypes m_Scripting_SnapshotTypes;
	CScripting_Physics m_Scripting_Physics;

public:
	virtual void OnInit()
	{
		m_Script.Reset();
		//m_Scripting_Resources.Register(&m_Script, m_pResources);
		m_Scripting_SnapshotTypes.Register(&m_Script);
		m_Scripting_Physics.Register(&m_Script);

		m_Script.SetVariableInt("server", 1);
		m_Script.SetVariableFloat("time_servertickspeed", SERVER_TICK_SPEED);

		m_Script.DoFile("data/games/teeworlds/server.lua");

		m_Script.Call("OnInit", "");
	}

	virtual void OnConsoleInit()
	{
	}

	virtual void OnShutdown()
	{
	}

	virtual void OnTick()
	{
		m_Script.Call("OnTick", "");
	}

	virtual void OnPreSnap()
	{
		m_Script.Call("OnPreSnap", "");
	}

	virtual void OnSnap(int ClientID)
	{
		m_Script.Call("OnSnap", "i", ClientID);
	}

	virtual void OnPostSnap()
	{
		m_Script.Call("OnPostSnap", "");
	}

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
	{
	}

	virtual void OnClientConnected(int ClientID)
	{
		m_Script.Call("OnClientConnected", "i", ClientID);
	}

	virtual void OnClientEnter(int ClientID)
	{
		m_Script.Call("OnClientEnter", "i", ClientID);
	}

	virtual void OnClientDrop(int ClientID, const char *pReason)
	{
		m_Script.Call("OnClientDrop", "i", ClientID); // TODO: add reason
	}

	virtual void OnClientDirectInput(int ClientID, void *pInput)
	{
	}

	virtual void OnClientPredictedInput(int ClientID, void *pInput)
	{
	}

	virtual bool IsClientReady(int ClientID)
	{
		return true;
	}

	virtual bool IsClientPlayer(int ClientID)
	{
		return false;
	}

	virtual const char *GameType()
	{
		return "lua";
	}

	virtual const char *Version()
	{
		return "lua1";
	}

	virtual const char *NetVersion()
	{
		return "lua1";
	}
};

IGameServer *CreateGameServer() { return new CGameServer_Lua; }
