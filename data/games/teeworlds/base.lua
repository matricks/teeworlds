function TableLock(tbl)
	local mt = getmetatable(tbl)
	if not mt then mt = {} end
	mt.__newindex = function(tbl, key, value)
		error("trying to create key '" .. key .. "' on a locked table")
	end
	setmetatable(tbl, mt)
end

function TableDeepCopy(object)
	local lookup_table = {}
	local function _copy(object)
		if type(object) ~= "table" then
			return object
		elseif lookup_table[object] then
			return lookup_table[object]
		end
		local new_table = {}
		lookup_table[object] = new_table
		for index, value in pairs(object) do
			new_table[_copy(index)] = _copy(value)
		end
		return setmetatable(new_table, getmetatable(object))
	end
	return _copy(object)
end

function DefineClass(table)
	local newstruct = {
		t = table,
		New = function(self)
			local new = TableDeepCopy(self.t)
			return new
		end
	}
	TableLock(newstruct)
	return newstruct
end

data = {}
if engine.client then -- for now
	data.texture_flagblue = engine.Resource_Get("gfx/flag_blue.png")
	data.texture_pickuphealth = engine.Resource_Get("gfx/pickup_health.png")
	data.texture_pickuparmor = engine.Resource_Get("gfx/pickup_armor.png")

	data.texture_weapon_hammer = engine.Resource_Get("gfx/weapon_hammer.png")
	data.texture_weapon_grenade = engine.Resource_Get("gfx/weapon_grenade.png")
	data.texture_weapon_gun = engine.Resource_Get("gfx/weapon_gun.png")
	data.texture_weapon_rifle = engine.Resource_Get("gfx/weapon_rifle.png")
	data.texture_weapon_shotgun = engine.Resource_Get("gfx/weapon_shotgun.png")

	data.texture_bullet_gun = engine.Resource_Get("gfx/bullet_0.png")
	data.texture_bullet_shotgun = engine.Resource_Get("gfx/bullet_1.png")
	data.texture_bullet_grenade = engine.Resource_Get("gfx/bullet_2.png")

	data.texture_tee = engine.Resource_Get("skins/default.png")
end

data.tee = {
	body = {0, 0, 3/8, 3/4},
	body_outline = {3/8, 0, 3/8+3/8, 3/4},
	eye = {2/8, 3/4, 2/8 + 1/8, 1},
	foot = {6/8, 1/4, 1, 1/4 + 1/4},
	foot_outline = {6/8, 2/4, 1, 2/4 + 1/4},
}

---------- VERY IMPORANT THAT THIS SECTION MATCHES datasrc/network.py ----------
data.pickups = {
	{tex = data.texture_pickuphealth, w = 48, h = 48},
	{tex = data.texture_pickuparmor, w = 48, h = 48},
	{tex = nil, w = 1, h = 1}, -- TODO: missing ninja
	{tex = data.texture_weapon_hammer, w = 96, h = 24},
	{tex = data.texture_weapon_gun, w = 96, h = 24},
	{tex = data.texture_weapon_shotgun, w = 96, h = 24},
	{tex = data.texture_weapon_grenade, w = 96, h = 24},
	{tex = data.texture_weapon_rifle, w = 96, h = 24},
}

WEAPON_HAMMER = 0
WEAPON_GUN = 1
WEAPON_SHOTGUN = 2
WEAPON_GRENADE = 3
WEAPON_RIFLE = 4

MAPTILE_ENTITY_NULL = 0
MAPTILE_ENTITY_SPAWN = 1
MAPTILE_ENTITY_SPAWN_RED = 2
MAPTILE_ENTITY_SPAWN_BLUE = 3
MAPTILE_ENTITY_FLAGSTAND_RED = 4
MAPTILE_ENTITY_FLAGSTAND_BLUE = 5
MAPTILE_ENTITY_ARMOR_1 = 6
MAPTILE_ENTITY_HEALTH_1 = 7
MAPTILE_ENTITY_WEAPON_SHOTGUN = 8
MAPTILE_ENTITY_WEAPON_GRENADE = 9
MAPTILE_ENTITY_POWERUP_NINJA = 10
MAPTILE_ENTITY_WEAPON_RIFLE = 11

-- TUNING ----------------------------------------------------------------------

tuning = {}
tuning.groundcontrolspeed = 10.0
tuning.groundcontrolaccel = 100.0 / engine.time_servertickspeed
tuning.groundfriction = 0.5
tuning.groundjumpimpulse = 13.2
tuning.airjumpimpulse = 12.0
tuning.aircontrolspeed = 250.0 / engine.time_servertickspeed
tuning.aircontrolaccel = 1.5
tuning.airfriction = 0.95
tuning.hooklength = 380.0
tuning.hookfirespeed = 80.0
tuning.hookdragaccel = 3.0
tuning.hookdragspeed = 15.0
tuning.gravity = 0.5

tuning.velramp_start = 550
tuning.velramp_range = 2000
tuning.velramp_curvature = 1.4

tuning.weapon_gun_speed = 2200
tuning.weapon_gun_curvature = 1.25

tuning.weapon_shotgun_speed = 2750
tuning.weapon_shotgun_curvature = 1.25

tuning.weapon_grenade_speed = 1000
tuning.weapon_grenade_curvature = 7

-- SNAPSHOT ITEMS ----------------------------------------------------------------------

SNAPITEM_PLAYERINPUT = engine.Snap_RegisterItemType({
	{name = "direction"},
	{name = "target_x"},
	{name = "target_y"},
	{name = "jump"},
	{name = "fire"},
	{name = "hook"},
	{name = "playerflags"},
	{name = "wantedweapon"},
	{name = "nextweapon"},
	{name = "prevweapon"},
})

SNAPITEM_PROJECTILE = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "vel_x"},
	{name = "vel_y"},
	{name = "type"},
	{name = "starttick"},
})

SNAPITEM_LASER = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "from_x"},
	{name = "from_y"},
	{name = "starttick"},
})

SNAPITEM_PICKUP = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "type"},
})

SNAPITEM_FLAG = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "team"},
})

SNAPITEM_GAMEINFO = engine.Snap_RegisterItemType({
	{name = "gameflags"},
	{name = "gamestateflags"},
	{name = "roundstarttick"},
	{name = "warmuptimer"},
	{name = "scorelimit"},
	{name = "timelimit"},
	{name = "roundnum"},
	{name = "roundcurrent"},
})

SNAPITEM_GAMEDATA = engine.Snap_RegisterItemType({
	{name = "teamscore_red"},
	{name = "teamscore_blue"},
	{name = "flagcarrier_red"},
	{name = "flagcarrier_blue"},
})

SNAPITEM_CHARACTERCORE = engine.Snap_RegisterItemType({
	{name = "tick"},
	{name = "x"},
	{name = "y"},
	{name = "vel_x", scale = 256},
	{name = "vel_y", scale = 256},
	{name = "angle", scale = 256},
	{name = "direction"},
	{name = "jumped"},
	{name = "hookedplayer"},
	{name = "hookedstate"},
	{name = "hooktick"},
	{name = "hook_x"},
	{name = "hook_y"},
	{name = "hook_dx"},
	{name = "hook_dy"},
})

SNAPITEM_CHARACTER = engine.Snap_RegisterItemType({
	{name = "tick"},
	{name = "x"},
	{name = "y"},
	{name = "vel_x", scale = 256},
	{name = "vel_y", scale = 256},
	{name = "angle", scale = 256},
	{name = "direction"},
	{name = "jumped"},
	{name = "hookedplayer"},
	{name = "hookedstate"},
	{name = "hooktick"},
	{name = "hook_x"},
	{name = "hook_y"},
	{name = "hook_dx"},
	{name = "hook_dy"},
	{name = "playerflags"},
	{name = "health"},
	{name = "armor"},
	{name = "ammocount"},
	{name = "weapon"},
	{name = "emote"},
	{name = "attacktick"},
})

SNAPITEM_PLAYERINFO = engine.Snap_RegisterItemType({
	{name = "local"},
	{name = "clientid"},
	{name = "team"},
	{name = "score"},
	{name = "latency"},
})

SNAPITEM_CLIENTINFO = engine.Snap_RegisterItemType({
	{name = "name0"},
	{name = "name1"},
	{name = "name2"},
	{name = "name3"},
	{name = "clan0"},
	{name = "clan1"},
	{name = "clan2"},
	{name = "country"},
	{name = "skin0"},
	{name = "skin1"},
	{name = "skin2"},
	{name = "skin3"},
	{name = "skin4"},
	{name = "skin5"},
	{name = "usecustomcolor"},
	{name = "colorbody"},
	{name = "colorfeet"},
})

SNAPITEM_SPECTATORINFO = engine.Snap_RegisterItemType({
	{name = "spectatorid"},
	{name = "x"},
	{name = "y"},
})

SNAPITEM_EVENT_COMMON = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
})

SNAPITEM_EVENT_EXPLOSION = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
})

SNAPITEM_EVENT_SPAWN = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
})

SNAPITEM_EVENT_HAMMERHIT = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
})

SNAPITEM_EVENT_DEATH = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "clientid"},
})

SNAPITEM_EVENT_SOUNDWORLD = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "soundid"},
})

SNAPITEM_EVENT_DAMAGEIND = engine.Snap_RegisterItemType({
	{name = "x"},
	{name = "y"},
	{name = "angle"},
})

MSG_SV_MOTD = engine.Msg_Register({
	{type="string", name = "message"},
})

MSG_SV_BROADCAST = engine.Msg_Register({
	{type="string", name = "message"},
})

MSG_SV_CHAT = engine.Msg_Register({
	{type="int", name = "team"},
	{type="int", name = "clientid"},
	{type="string", name = "message"},
})

MSG_SV_KILLMSG = engine.Msg_Register({
	{type="int", name = "killer"},
	{type="int", name = "victim"},
	{type="int", name = "weapon"},
	{type="int", name = "modespecial"},
})

MSG_SV_SOUNDGLOBAL = engine.Msg_Register({
	{type="resource", name = "soundid"},
})

MSG_SV_TUNEPARAMS = engine.Msg_Register({
})

MSG_SV_EXTRAPROJECTILE = engine.Msg_Register({
})

MSG_SV_READYTOENTER = engine.Msg_Register({
})



--[[
msg = engine.Msg_Create(MSG_SV_READYTOENTER)
msg.x = y
engine.Msg_Send(-1, msg)
]]

--[[
	NetMessage("Sv_WeaponPickup", [
		NetIntRange("m_Weapon", 0, 'NUM_WEAPONS-1'),
	]),

	NetMessage("Sv_Emoticon", [
		NetIntRange("m_ClientID", 0, 'MAX_CLIENTS-1'),
		NetIntRange("m_Emoticon", 0, 'NUM_EMOTICONS-1'),
	]),

	NetMessage("Sv_VoteClearOptions", [
	]),

	NetMessage("Sv_VoteOptionListAdd", [
		NetIntRange("m_NumOptions", 1, 15),
		NetStringStrict("m_pDescription0"), NetStringStrict("m_pDescription1"),	NetStringStrict("m_pDescription2"),
		NetStringStrict("m_pDescription3"),	NetStringStrict("m_pDescription4"),	NetStringStrict("m_pDescription5"),
		NetStringStrict("m_pDescription6"), NetStringStrict("m_pDescription7"), NetStringStrict("m_pDescription8"),
		NetStringStrict("m_pDescription9"), NetStringStrict("m_pDescription10"), NetStringStrict("m_pDescription11"),
		NetStringStrict("m_pDescription12"), NetStringStrict("m_pDescription13"), NetStringStrict("m_pDescription14"),
	]),

	NetMessage("Sv_VoteOptionAdd", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteOptionRemove", [
		NetStringStrict("m_pDescription"),
	]),

	NetMessage("Sv_VoteSet", [
		NetIntRange("m_Timeout", 0, 60),
		NetStringStrict("m_pDescription"),
		NetStringStrict("m_pReason"),
	]),

	NetMessage("Sv_VoteStatus", [
		NetIntRange("m_Yes", 0, 'MAX_CLIENTS'),
		NetIntRange("m_No", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Pass", 0, 'MAX_CLIENTS'),
		NetIntRange("m_Total", 0, 'MAX_CLIENTS'),
	]),

	### Client messages
	NetMessage("Cl_Say", [
		NetBool("m_Team"),
		NetString("m_pMessage"),
	]),

	NetMessage("Cl_SetTeam", [
		NetIntRange("m_Team", 'TEAM_SPECTATORS', 'TEAM_BLUE'),
	]),

	NetMessage("Cl_SetSpectatorMode", [
		NetIntRange("m_SpectatorID", 'SPEC_FREEVIEW', 'MAX_CLIENTS-1'),
	]),

	NetMessage("Cl_StartInfo", [
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetStringStrict("m_pSkin"),
		NetBool("m_UseCustomColor"),
		NetIntAny("m_ColorBody"),
		NetIntAny("m_ColorFeet"),
	]),

	NetMessage("Cl_ChangeInfo", [
		NetStringStrict("m_pName"),
		NetStringStrict("m_pClan"),
		NetIntAny("m_Country"),
		NetStringStrict("m_pSkin"),
		NetBool("m_UseCustomColor"),
		NetIntAny("m_ColorBody"),
		NetIntAny("m_ColorFeet"),
	]),

	NetMessage("Cl_Kill", []),

	NetMessage("Cl_Emoticon", [
		NetIntRange("m_Emoticon", 0, 'NUM_EMOTICONS-1'),
	]),

	NetMessage("Cl_Vote", [
		NetIntRange("m_Vote", -1, 1),
	]),

	NetMessage("Cl_CallVote", [
		NetStringStrict("m_Type"),
		NetStringStrict("m_Value"),
		NetStringStrict("m_Reason"),
	]),
]]

--MSG_ = engine.Msg_Register({
--	{name = "x"},
	--{name = "y"},
	--{name = "angle"},
--})

---------------------------- END SECTION ----------------------------------------


-- general math ----------------------------------------------------------------------

function Round(v)
	if v > 0 then
		return math.floor(v + 0.5)
	end
	return math.floor(v - 0.5)
end


function Lerp(s, d, a)
	return s + (d - s) * a
end

-- vector math ----------------------------------------------------------------------

function vAngle(x, y)
	if x == 0 and y == 0 then
		return 0
	end

	local a = math.atan(y/x);
	if x < 0 then
		a = a + math.pi;
	end
	return a;
end

function vLength(x,y)
	return math.sqrt(x*x+y*y)
end

function vNormalize(x,y)
	local l = vLength(x, y)
	return x/l, y/l
end


-- "game math" ----------------------------------------------------------------------

function SaturatedAdd(min, max, current, modifier)
	if modifier < 0 then
		if current < min then
			return current;
		end
		current = current + modifier
		if current < min then
			current = min
		end
		return current
	else
		if current > max then
			return current
		end
		current = current + modifier;
		if current > max then
			current = max
		end
		return current;
	end
end

function CalculateCurvedPosition(x, y, vx, vy, curvature, speed, time)
	time = time * speed;
	local nx = x + vx*time;
	local ny = y + vy*time + curvature/10000*(time*time);
	return nx, ny;
end

function VelocityRamp(value, start, range, curvature)
	if value < start then
		return 1.0
	end
	return 1.0 / math.pow(curvature, (value-start)/range)
end


-- character stuff -----------------------------------------------------------------

function Character_Move(char)
	local rampvalue = VelocityRamp(vLength(char.vel_x, char.vel_y)*50, tuning.velramp_start, tuning.velramp_range, tuning.velramp_curvature)

	char.vel_x = char.vel_x * rampvalue;

	--local nx,ny = char.x, char.y
	local nx,ny,nvx,nvy = engine.Physics_MoveBox(char.x, char.y, char.vel_x, char.vel_y, 28, 28, 0)

	nvx = nvx * (1.0 / rampvalue)

	--[[
	if(m_pWorld && m_pWorld->m_Tuning.m_PlayerCollision)
	{
		// check player collision
		float Distance = distance(m_Pos, NewPos);
		int End = Distance+1;
		vec2 LastPos = m_Pos;
		for(int i = 0; i < End; i++)
		{
			float a = i/Distance;
			vec2 Pos = mix(m_Pos, NewPos, a);
			for(int p = 0; p < MAX_CLIENTS; p++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[p];
				if(!pCharCore || pCharCore == this)
					continue;
				float D = distance(Pos, pCharCore->m_Pos);
				if(D < 28.0f && D > 0.0f)
				{
					if(a > 0.0f)
						m_Pos = LastPos;
					else if(distance(NewPos, pCharCore->m_Pos) > D)
						m_Pos = NewPos;
					return;
				}
			}
			LastPos = Pos;
		}
	}]]--

	char.vel_x = nvx
	char.vel_y = nvy
	char.x = nx
	char.y = ny
end

function Character_Quantize(char)
	char.x = Round(char.x)
	char.y = Round(char.y)
	char.vel_x = Round(char.vel_x*256)/256
	char.vel_y = Round(char.vel_y*256)/256
end


function Character_Tick(char, input, use_input)
	local PHYS_SIZE = 28.0
	--m_TriggeredEvents = 0;

	-- get ground state
	local grounded = false
	if engine.Physics_CheckPoint(char.x+PHYS_SIZE/2, char.y+PHYS_SIZE/2+5) then
		grounded = true
	elseif engine.Physics_CheckPoint(char.x-PHYS_SIZE/2, char.y+PHYS_SIZE/2+5) then
		grounded = true
	end
	

	local target_dir_x = 1
	local target_dir_y = 0
	if input then
		target_dir_x, target_dir_y = vNormalize(input.target_x, input.target_y)
	end

	char.vel_y = char.vel_y + tuning.gravity

	local maxspeed = tuning.aircontrolspeed
	local accel = tuning.aircontrolaccel
	local friction = tuning.airfriction

	if grounded then
		maxspeed = tuning.groundcontrolspeed
		accel = tuning.groundcontrolaccel
		friction = tuning.groundfriction
	end

	--[[ handle input ]]--
	if input then
		char.direction = input.direction;

		-- setup angle
		local a = 0
		if input.target_x == 0 then
			a = math.atan(input.target_y)
		else
			a = math.atan(input.target_y / input.target_x)
		end

		if input.target_x < 0 then
			a = a + pi
		end

		char.angle = a*256.0

		-- handle jump
		--[[
		if(input.jump)
		{
			if(!(m_Jumped&1))
			{
				if(Grounded)
				{
					--m_TriggeredEvents |= COREEVENT_GROUND_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning.m_GroundJumpImpulse;
					m_Jumped |= 1;
				}
				else if(!(m_Jumped&2))
				{
					m_TriggeredEvents |= COREEVENT_AIR_JUMP;
					m_Vel.y = -m_pWorld->m_Tuning.m_AirJumpImpulse;
					m_Jumped |= 3;
				}
			}
		}
		else
			m_Jumped &= ~1;
		]]

		-- handle hook
		--[[
		if(m_Input.m_Hook)
		{
			if(m_HookState == HOOK_IDLE)
			{
				m_HookState = HOOK_FLYING;
				m_HookPos = m_Pos+TargetDirection*PhysSize*1.5f;
				m_HookDir = TargetDirection;
				m_HookedPlayer = -1;
				m_HookTick = 0;
				m_TriggeredEvents |= COREEVENT_HOOK_LAUNCH;
			}
		}
		else
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_IDLE;
			m_HookPos = m_Pos;
		}]]
	end

	-- add the speed modification according to players wanted direction
	if char.direction < 0 then
		char.vel_x = SaturatedAdd(-maxspeed, maxspeed, char.vel_x, -accel)
	elseif char.direction > 0 then
		char.vel_x = SaturatedAdd(-maxspeed, maxspeed, char.vel_x, accel)
	elseif char.direction == 0 then
		char.vel_x = char.vel_x * friction
	end

	-- handle jumping
	-- 1 bit = to keep track if a jump has been made on this input
	-- 2 bit = to keep track if a air-jump has been made
	if grounded then
		-- char.jumped &= ~2
		if char.jumped > 1 then
			char.jumped = char.jumped - 2
		end
	end

	--[[
	-- do hook
	if(m_HookState == HOOK_IDLE)
	{
		m_HookedPlayer = -1;
		m_HookState = HOOK_IDLE;
		m_HookPos = m_Pos;
	}
	else if(m_HookState >= HOOK_RETRACT_START && m_HookState < HOOK_RETRACT_END)
	{
		m_HookState++;
	}
	else if(m_HookState == HOOK_RETRACT_END)
	{
		m_HookState = HOOK_RETRACTED;
		m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		m_HookState = HOOK_RETRACTED;
	}
	else if(m_HookState == HOOK_FLYING)
	{
		vec2 NewPos = m_HookPos+m_HookDir*m_pWorld->m_Tuning.m_HookFireSpeed;
		if(distance(m_Pos, NewPos) > m_pWorld->m_Tuning.m_HookLength)
		{
			m_HookState = HOOK_RETRACT_START;
			NewPos = m_Pos + normalize(NewPos-m_Pos) * m_pWorld->m_Tuning.m_HookLength;
		}

		// make sure that the hook doesn't go though the ground
		bool GoingToHitGround = false;
		bool GoingToRetract = false;
		int Hit = m_pCollision->IntersectLine(m_HookPos, NewPos, &NewPos, 0);
		if(Hit)
		{
			if(Hit&CCollision::COLFLAG_NOHOOK)
				GoingToRetract = true;
			else
				GoingToHitGround = true;
		}

		// Check against other players first
		if(m_pWorld && m_pWorld->m_Tuning.m_PlayerHooking)
		{
			float Distance = 0.0f;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
				if(!pCharCore || pCharCore == this)
					continue;

				vec2 ClosestPoint = closest_point_on_line(m_HookPos, NewPos, pCharCore->m_Pos);
				if(distance(pCharCore->m_Pos, ClosestPoint) < PhysSize+2.0f)
				{
					if (m_HookedPlayer == -1 || distance(m_HookPos, pCharCore->m_Pos) < Distance)
					{
						m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
						m_HookState = HOOK_GRABBED;
						m_HookedPlayer = i;
						Distance = distance(m_HookPos, pCharCore->m_Pos);
					}
				}
			}
		}

		if(m_HookState == HOOK_FLYING)
		{
			// check against ground
			if(GoingToHitGround)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_GROUND;
				m_HookState = HOOK_GRABBED;
			}
			else if(GoingToRetract)
			{
				m_TriggeredEvents |= COREEVENT_HOOK_HIT_NOHOOK;
				m_HookState = HOOK_RETRACT_START;
			}

			m_HookPos = NewPos;
		}
	}

	if(m_HookState == HOOK_GRABBED)
	{
		if(m_HookedPlayer != -1)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[m_HookedPlayer];
			if(pCharCore)
				m_HookPos = pCharCore->m_Pos;
			else
			{
				// release hook
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}

			// keep players hooked for a max of 1.5sec
			//if(Server()->Tick() > hook_tick+(Server()->TickSpeed()*3)/2)
				//release_hooked();
		}

		// don't do this hook rutine when we are hook to a player
		if(m_HookedPlayer == -1 && distance(m_HookPos, m_Pos) > 46.0f)
		{
			vec2 HookVel = normalize(m_HookPos-m_Pos)*m_pWorld->m_Tuning.m_HookDragAccel;
			// the hook as more power to drag you up then down.
			// this makes it easier to get on top of an platform
			if(HookVel.y > 0)
				HookVel.y *= 0.3f;

			// the hook will boost it's power if the player wants to move
			// in that direction. otherwise it will dampen everything abit
			if((HookVel.x < 0 && m_Direction < 0) || (HookVel.x > 0 && m_Direction > 0))
				HookVel.x *= 0.95f;
			else
				HookVel.x *= 0.75f;

			vec2 NewVel = m_Vel+HookVel;

			// check if we are under the legal limit for the hook
			if(length(NewVel) < m_pWorld->m_Tuning.m_HookDragSpeed || length(NewVel) < length(m_Vel))
				m_Vel = NewVel; // no problem. apply

		}

		// release hook (max hook time is 1.25
		m_HookTick++;
		if(m_HookedPlayer != -1 && (m_HookTick > SERVER_TICK_SPEED+SERVER_TICK_SPEED/5 || !m_pWorld->m_apCharacters[m_HookedPlayer]))
		{
			m_HookedPlayer = -1;
			m_HookState = HOOK_RETRACTED;
			m_HookPos = m_Pos;
		}
	}

	if(m_pWorld)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			CCharacterCore *pCharCore = m_pWorld->m_apCharacters[i];
			if(!pCharCore)
				continue;

			//player *p = (player*)ent;
			if(pCharCore == this) // || !(p->flags&FLAG_ALIVE)
				continue; // make sure that we don't nudge our self

			// handle player <-> player collision
			float Distance = distance(m_Pos, pCharCore->m_Pos);
			vec2 Dir = normalize(m_Pos - pCharCore->m_Pos);
			if(m_pWorld->m_Tuning.m_PlayerCollision && Distance < PhysSize*1.25f && Distance > 0.0f)
			{
				float a = (PhysSize*1.45f - Distance);
				float Velocity = 0.5f;

				// make sure that we don't add excess force by checking the
				// direction against the current velocity. if not zero.
				if (length(m_Vel) > 0.0001)
					Velocity = 1-(dot(normalize(m_Vel), Dir)+1)/2;

				m_Vel += Dir*a*(Velocity*0.75f);
				m_Vel *= 0.85f;
			}

			// handle hook influence
			if(m_HookedPlayer == i && m_pWorld->m_Tuning.m_PlayerHooking)
			{
				if(Distance > PhysSize*1.50f) // TODO: fix tweakable variable
				{
					float Accel = m_pWorld->m_Tuning.m_HookDragAccel * (Distance/m_pWorld->m_Tuning.m_HookLength);
					float DragSpeed = m_pWorld->m_Tuning.m_HookDragSpeed;

					// add force to the hooked player
					pCharCore->m_Vel.x = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.x, Accel*Dir.x*1.5f);
					pCharCore->m_Vel.y = SaturatedAdd(-DragSpeed, DragSpeed, pCharCore->m_Vel.y, Accel*Dir.y*1.5f);

					// add a little bit force to the guy who has the grip
					m_Vel.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel*Dir.x*0.25f);
					m_Vel.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel*Dir.y*0.25f);
				}
			}
		}
	}
	]]

	-- clamp the velocity to something sane
	if vLength(char.vel_x, char.vel_y) > 6000 then
		char.vel_x, char.vel_y = vNormalize(char.vel_x, char.vel_y) * 6000;
	end
end
--[[

-- TODO: rendering
DrawQuadTL(t,x,y,w,h,r)
?DrawFreeform
]]