dofile("data/games/teeworlds/base.lua") -- TODO: fix the path

TestId = 0

function OnInit()
	TestId = engine.Snap_NewId()
	print(SNAPITEM_PICKUP, TestId)
end

function OnTick()
end

function OnPreSnap()
end

function OnSnap(client_id)
	local item = engine.Snap_CreateItem(SNAPITEM_PICKUP, TestId)
	--for k,v in pairs(item) do
	--	print(k,v)
	--end
	item.x = 150
	item.y = 150
	item.type = 0 -- health
	engine.Snap_CommitItem(item)
end

function OnPostSnap()
end

function OnClientConnected(client_id)
	print(client_id, "connected")

	local msg = engine.Msg_Create(MSG_SV_READYTOENTER)
	engine.Msg_Send(msg, client_id)
end

function OnClientEnter(client_id)
	print(client_id, "entered")
end

function OnClientDrop(client_id)
	print(client_id, "dropped")
end