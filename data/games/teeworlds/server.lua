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
	instance.class = self
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

function CClient:Snap(client)
	local item = engine.Snap_CreateItem(SNAPITEM_PLAYERINFO, self.id)

	item.islocal = 1
	item.clientid = self.id
	item.team = 0
	item.score = 0
	item.latency = 0

	engine.Snap_CommitItem(item)
end

-- BASE ENTITY ----------------------------------------------------------------------

CEntity = CClass:Subclass()
CEntity.id = 0
CEntity.x = 0
CEntity.y = 0
CEntity.proximity_radius = 32

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
CEntity_Pickup.pickupsnaptype = 0
CEntity_Pickup.spawn_tick = -1
CEntity.proximity_radius = 14

function CEntity_Pickup:Init(x, y)
	CEntity.Init(self, x, y) -- why can't I do self.super.Init??
end

function CEntity_Pickup:SetRespawn(time)
	self.spawn_tick = engine.time_gametick + engine.time_tickspeed*time
end

function CEntity_Pickup:IsSpawned()
	return engine.time_gametick > self.spawn_tick
end

function CEntity_Pickup:Pickup(character)
	self:SetRespawn(1)
end

function CEntity_Pickup:Snap(client)
	if self:IsSpawned() then
		local item = engine.Snap_CreateItem(SNAPITEM_PICKUP, self.id)
		item.x = self.x
		item.y = self.y
		item.type = self.pickupsnaptype
		engine.Snap_CommitItem(item)
	end
end

-----

CEntity_Pickup_Armor = CEntity_Pickup:Subclass()
CEntity_Pickup_Armor.pickupsnaptype = 1
function CEntity_Pickup_Armor:Pickup(character)
	if character.armor < 10 then
		character.armor = character.armor + 1
		self:SetRespawn(15)
	end
end

CEntity_Pickup_Health = CEntity_Pickup:Subclass()
CEntity_Pickup_Health.pickupsnaptype = 0
function CEntity_Pickup_Health:Pickup(character)
	if character.health < 10 then
		character.health = character.health + 1
		self:SetRespawn(15)
	end
end

-- CHARACTER ENTITY ----------------------------------------------------------------------

CEntity_Character = CEntity:Subclass()
local coretable = engine.Snap_CreateItem(SNAPITEM_CHARACTERCORE, 0) -- gotta clean this up
CEntity_Character.core = TableDeepCopy(coretable)
CEntity_Character.client = 0
CEntity_Character.vel_x = 0
CEntity_Character.vel_y = 0

CEntity_Character.health = 10
CEntity_Character.armor = 0

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

	-- check for collisions, do this here fo rnow
	for _,ent in pairs(world.entities) do
		if vLength(self.x - ent.x, self.y - ent.y) < self.proximity_radius+ent.proximity_radius then
			-- check for pickup
			if ent.Pickup and ent:IsSpawned() then
				ent:Pickup(self)
			end
		end
	end	
end

function CEntity_Character:Snap(client)
	local item = engine.Snap_CreateItem(SNAPITEM_CHARACTER, client.id)

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
	item.health = self.health
	item.armor = self.armor
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
				ent = CEntity_Pickup_Health:New(x, y)
			elseif tile == MAPTILE_ENTITY_ARMOR_1 then
				ent = CEntity_Pickup_Armor:New(x, y)
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
	local snapclient = clients[client_id]

	for _,client in pairs(clients) do
		client:Snap(snapclient)
	end

	for _,ent in pairs(world.entities) do
		if not ent:NetworkClip(snapclient) then
			ent:Snap(snapclient)
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
	ent.client = client
	

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


function OnClientDirectInput(client_id, input)
	clients[client_id].entity.input = input
end