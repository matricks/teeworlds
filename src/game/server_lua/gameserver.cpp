#include <engine/server.h>
#include <engine/scripting.h>

class CGameServer_Lua : public IGameServer
{
public:
	MACRO_INTERFACE("gameserver", 0)
protected:
	CScriptHost m_Script;
public:
	virtual void OnInit()
	{
		m_Script.Reset();
		m_Script.DoFile("data/games/teeworlds/server.lua");

		m_Script.Call("OnInit");
	}

	virtual void OnConsoleInit()
	{
	}

	virtual void OnShutdown()
	{
	}

	virtual void OnTick()
	{
		m_Script.Call("OnTick");
	}

	virtual void OnPreSnap()
	{
		m_Script.Call("OnPreSnap");
	}

	virtual void OnSnap(int ClientID)
	{
		m_Script.Call("OnSnap"); // TODO: client id needed
	}

	virtual void OnPostSnap()
	{
		m_Script.Call("OnPostSnap");
	}

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
	{
	}

	virtual void OnClientConnected(int ClientID)
	{
	}

	virtual void OnClientEnter(int ClientID)
	{
	}

	virtual void OnClientDrop(int ClientID, const char *pReason)
	{
	}

	virtual void OnClientDirectInput(int ClientID, void *pInput)
	{
	}

	virtual void OnClientPredictedInput(int ClientID, void *pInput)
	{
	}

	virtual bool IsClientReady(int ClientID)
	{
		return false;
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
