dofile("data/games/teeworlds/base.lua") -- TODO: fix the path


-- CLASS SYSTEM
-- Lookups are fast, creation is not
-- :Init is called when it's created

CClass = {}

function CClass:Subclass()
	return TableDeepCopy(self)
end

function CClass:New(...)
	local instance = TableDeepCopy(self)
	if instance.Init then
		instance:Init(...)
	end
	return instance
end


CClient = CClass:Subclass()
CClient.id = 0
CClient.name = ""
CClient.view_x = 0
CClient.view_y = 0



CEntity = CClass:Subclass()
CEntity.id = 0
CEntity.x = 0
CEntity.y = 0

function CEntity:Init(x, y)
	self.x = x
	self.y = y
	self.id = engine.Snap_NewId() -- TODO: this must be released some how
end

function CEntity:Tick()
end

function CEntity:NetworkClip(client)
	return false
end

function CEntity:Snap(client)
end

CEntity_Pickup = CEntity:Subclass()
CEntity_Pickup.type = 0

function CEntity_Pickup:Init(x, y, pickuptype)
	CEntity.Init(self, x, y) -- why can't I do self.super.Init??
	self.type = pickuptype
end

function CEntity_Pickup:Snap(client)
	local item = engine.Snap_CreateItem(SNAPITEM_PICKUP, self.id)
	item.x = self.x
	item.y = self.y
	item.type = self.type
	engine.Snap_CommitItem(item)
end


-- 
clients = {}
world = {}
world.entities = {}

function OnInit()
	local map_width, map_height = engine.Map_GetSize()
	print("Map size:", map_width, map_height)

	for ty = 0, map_height-1 do
		local y = ty*32 + 16
		for tx = 0, map_width-1 do
			local tile = engine.Map_GetTile(tx, ty)
			local x = tx*32 + 16

			local ent = 0
			if tile == MAPTILE_ENTITY_HEALTH_1 then
				ent = CEntity_Pickup:New(x, y, 0)
			elseif tile == MAPTILE_ENTITY_ARMOR_1 then
				ent = CEntity_Pickup:New(x, y, 1)
			end

			if not (ent == 0) then
				world.entities[ent.id] = ent
			end

		end
	end
end

function OnTick()
end

function OnPreSnap()
end

function OnSnap(client_id)
	local client = clients[client_id]

	for _,ent in pairs(world.entities) do
		if not ent:NetworkClip(client) then
			ent:Snap(client)
		end
	end
end

function OnPostSnap()
end

function OnClientConnected(client_id)
	local client = CClient:New()
	client.id = client_id

	clients[client.id] = client
	print(client_id, "connected")

	-- send enter message
	local msg = engine.Msg_Create(MSG_SV_READYTOENTER)
	engine.Msg_Send(msg, client_id)
end

function OnClientEnter(client_id)
	print(client_id, "entered")
end

function OnClientDrop(client_id)
	print(client_id, "dropped")
	clients[client_id] = nil
end