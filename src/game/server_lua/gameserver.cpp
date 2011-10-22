#include <engine/shared/protocol.h>
#include <engine/server.h>
#include <engine/map.h>
#include <game/scripting.h>
#include <game/mapitems.h>

// this is needed for now
#include <game/generated/protocol.h>

#include <game/layers.h> // for map layers
#include <game/collision.h> // for map layers

class CGameServer_Lua : public IGameServer
{
public:
	MACRO_INTERFACE("gameserver", 0)
protected:
	IServer *m_pServer;

	CScriptHost m_Script;
	CScripting_Resources m_Scripting_Resources;
	CScripting_SnapshotTypes m_Scripting_SnapshotTypes;
	CScripting_SnapshotServer m_Scripting_SnapshotServer;
	CScripting_Physics m_Scripting_Physics;
	CScripting_Messaging m_Scripting_Messaging;
	CScripting_Map m_Scripting_Map;
	CScripting_Input m_Scripting_Input;

	CLayers m_Layers;
	CCollision m_Collision;

	// this is needed for now, we need the sizes of the snap objects.... crap
	CNetObjHandler m_NetObjHandler;
public:
	virtual void OnInit()
	{
		m_pServer = Kernel()->RequestInterface<IServer>();

		m_Layers.Init(Kernel());
		m_Collision.Init(&m_Layers);

		IMap *pMap = Kernel()->RequestInterface<IMap>();

		// dig out the images that the map uses
		{
			
			int Start, Count;
			pMap->GetType(MAPITEMTYPE_IMAGE, &Start, &Count);

			// load new textures
			for(int i = 0; i < Count; i++)
			{
				CMapItemImage *pImg = (CMapItemImage *)pMap->GetItem(Start+i, 0, 0);
				if(pImg->m_External)
				{
					char Buf[256];
					str_format(Buf, sizeof(Buf), "mapres/%s.png", (char *)pMap->GetData(pImg->m_ImageName));
					m_pServer->LoadResource(Buf);
				}
			}
		}

		// do this for now, should be moved into the scripting parts
		for(int i = 0; i < NUM_NETOBJTYPES; i++)
			m_pServer->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

		m_Script.Reset();
		//m_Scripting_Resources.Register(&m_Script, m_pResources);
		m_Scripting_SnapshotTypes.Register(&m_Script);
		m_Scripting_SnapshotServer.Register(&m_Script, m_pServer, &m_Scripting_SnapshotTypes);
		m_Scripting_Physics.Register(&m_Script);
		m_Scripting_Messaging.Register(&m_Script, NULL, m_pServer);
		m_Scripting_Input.Register(&m_Script, NULL, m_pServer);

		CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
		m_Scripting_Map.Register(&m_Script, pTileMap, (CTile *)pMap->GetData(pTileMap->m_Data));

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
		m_Script.SetVariableInt("time_gametick", m_pServer->Tick());
		m_Script.SetVariableFloat("time_servertickspeed", m_pServer->TickSpeed());

		m_Script.Call("OnTick", "");
	}

	virtual void OnPreSnap()
	{
		m_Script.Call("OnPreSnap", "");
	}

	virtual void OnSnap(int ClientId)
	{
		m_Script.Call("OnSnap", "i", ClientId);
	}

	virtual void OnPostSnap()
	{
		m_Script.Call("OnPostSnap", "");
	}

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientId)
	{
	}

	virtual void OnClientConnected(int ClientId)
	{
		m_Script.Call("OnClientConnected", "i", ClientId);
	}

	virtual void OnClientEnter(int ClientId)
	{
		m_Script.Call("OnClientEnter", "i", ClientId);
	}

	virtual void OnClientDrop(int ClientId, const char *pReason)
	{
		m_Script.Call("OnClientDrop", "i", ClientId); // TODO: add reason
	}

	virtual void OnClientDirectInput(int ClientId, void *pInput, int Count)
	{
	}

	virtual void OnClientPredictedInput(int ClientId, void *pInput, int Count)
	{
		m_Script.CallSetup("OnClientPredictedInput");
		m_Script.CallIntegerArg(ClientId);
		m_Scripting_Input.PushInput(&m_Script, (int *)pInput, Count);
		m_Script.CallCustomArg();
		m_Script.CallPerform();
	}

	virtual bool IsClientReady(int ClientID)
	{
		return true;
	}

	virtual bool IsClientPlayer(int ClientId)
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
