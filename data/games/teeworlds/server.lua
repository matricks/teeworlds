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
CClient.entity = false
CClient.input = false

-- BASE ENTITY ----------------------------------------------------------------------

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

-- PICKUP ENTITY ----------------------------------------------------------------------

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

-- CHARACTER ENTITY ----------------------------------------------------------------------

CEntity_Character = CEntity:Subclass()
local coretable = engine.Snap_CreateItem(SNAPITEM_CHARACTERCORE, 0) -- gotta clean this up
CEntity_Character.core = TableDeepCopy(coretable)
CEntity_Character.vel_x = 0
CEntity_Character.vel_y = 0

function CEntity_Character:Tick()
	self.core.x = self.x
	self.core.y = self.y

	--[[local input = false
	for k,client in pairs(clients) do
		if client.ent == self then
			input = client.input
			print("got input")
			break
		end
	end]]

	Character_Tick(self.core, self.input)
	Character_Move(self.core)
	Character_Quantize(self.core)

	self.x = self.core.x
	self.y = self.core.y
end

function CEntity_Character:Snap(client)
	local item = engine.Snap_CreateItem(SNAPITEM_CHARACTER, self.id)

	item.tick = engine.time_gametick
	item.x = self.x
	item.y = self.y
	item.vel_x = self.core.vel_x
	item.vel_y = self.core.vel_y
	item.angle = self.core.angle
	item.direction = self.core.direction
	item.jumped = self.core.jumped
	item.hookedplayer = self.core.hookedplayer
	item.hookedstate = self.core.hookedstate
	item.hooktick = self.core.hooktick
	item.hook_x = self.core.hook_x
	item.hook_y = self.core.hook_y
	item.hook_dx = self.core.hook_dx
	item.hook_dy = self.core.hook_dy
	item.playerflags = 0
	item.health = 5
	item.armor = 5
	item.ammocount = 5
	item.weapon = 0
	item.emote = -1
	item.attacktick = 0

	engine.Snap_CommitItem(item)
end

--  ----------------------------------------------------------------------

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
	for _,ent in pairs(world.entities) do
		ent:Tick(client)
	end
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

	local ent = CEntity_Character:New(150, 150)
	world.entities[ent.id] = ent
	client.entity = ent
	

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

function OnClientPredictedInput(client_id, input)
	clients[client_id].entity.input = input
end