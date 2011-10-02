dofile("data/games/teeworlds/base.lua") -- TODO: fix the path

function OnInit()
end

function OnTick()
end

function OnPreSnap()
end

function OnSnap(client_id)
end

function OnPostSnap()
end

function OnClientConnected(client_id)
	print(client_id, "connected")
end

function OnClientEnter(client_id)
	print(client_id, "entered")
end

function OnClientDrop(client_id)
	print(client_id, "dropped")
end