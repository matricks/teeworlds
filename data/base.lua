data = {}
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

-- these must match the current tuning
WEAPON_GUN_SPEED = 2200
WEAPON_GUN_CURVATURE = 1.25

WEAPON_SHOTGUN_SPEED = 2750
WEAPON_SHOTGUN_CURVATURE = 1.25

WEAPON_GRENADE_SPEED = 1000
WEAPON_GRENADE_CURVATURE = 7

tuning = {}
tuning.groundcontrolspeed = 10.0
tuning.groundcontrolaccel = 100.0
tuning.groundfriction = 0.5
tuning.groundjumpimpulse = 13.2
tuning.airjumpimpulse = 12.0
tuning.aircontrolspeed = 250.0
tuning.aircontrolaccel = 1.5
tuning.airfriction = 0.95
tuning.hooklength = 380.0
tuning.hookfirespeed = 80.0
tuning.hookdragaccel = 3.0
tuning.hookdragspeed = 15.0
tuning.gravity = 0.5

SNAPITEM_PLAYERINPUT = engine.Snap_RegisterItemType({"direction", "target_x", "target_y", "jump", "fire", "hook", "playerflags", "wantedweapon", "nextweapon", "prevweapon"})
SNAPITEM_PROJECTILE = engine.Snap_RegisterItemType({"x", "y", "vel_x", "vel_y", "type", "starttick"})
SNAPITEM_LASER = engine.Snap_RegisterItemType({"x", "y", "from_x", "from_y", "starttick"})
SNAPITEM_PICKUP = engine.Snap_RegisterItemType({"x", "y", "type"})
SNAPITEM_FLAG = engine.Snap_RegisterItemType({"x", "y", "team"})
SNAPITEM_GAMEINFO = engine.Snap_RegisterItemType({"gameflags", "gamestateflags", "roundstarttick", "warmuptimer", "scorelimit", "timelimit", "roundnum", "roundcurrent"})
SNAPITEM_CHARACTERCORE = engine.Snap_RegisterItemType({"tick", "x", "y", "vel_x", "vel_y", "angle", "direction", "jumped", "hookedplayer", "hookedstate", "hooktick", "hook_x", "hook_y", "hook_dx", "hook_dy"})
SNAPITEM_CHARACTER = engine.Snap_RegisterItemType({"tick", "x", "y", "vel_x", "vel_y", "angle", "direction", "jumped", "hookedplayer", "hookedstate", "hooktick", "hook_x", "hook_y", "hook_dx", "hook_dy", 
													"playerflags", "health", "armor", "ammocount", "weapon", "emote", "attacktick"})
SNAPITEM_PLAYERINFO = engine.Snap_RegisterItemType({"local", "clientid", "team", "score", "latency"})
SNAPITEM_CLIENTINFO = engine.Snap_RegisterItemType({"name0", "name1", "name2", "name3", "clan0", "clan1", "clan2", "country", "skin0", "skin1", "skin2", "skin3", "skin4", "skin5", "usecustomcolor", "colorbody", "colorfeet"})
SNAPITEM_SPECTATORINFO = engine.Snap_RegisterItemType({"spectatorid", "x", "y"})

SNAPITEM_EVENT_COMMON = engine.Snap_RegisterItemType({"x", "y"})
SNAPITEM_EVENT_EXPLOSION = engine.Snap_RegisterItemType({"x", "y"})
SNAPITEM_EVENT_SPAWN = engine.Snap_RegisterItemType({"x", "y"})
SNAPITEM_EVENT_HAMMERHIT = engine.Snap_RegisterItemType({"x", "y"})
SNAPITEM_EVENT_DEATH = engine.Snap_RegisterItemType({"x", "y", "clientid"})
SNAPITEM_EVENT_SOUNDWORLD = engine.Snap_RegisterItemType({"x", "y", "soundid"})
SNAPITEM_EVENT_DAMAGEIND = engine.Snap_RegisterItemType({"x", "y", "angle"})

-- TODO: messages

---------------------------- END SECTION ----------------------------------------
function CalculateCurvedPosition(x, y, vx, vy, curvature, speed, time)
	time = time * speed;
	local nx = x + vx*time;
	local ny = y + vy*time + curvature/10000*(time*time);
	return nx, ny;
end


function AngleFromVector(x, y)
	if x == 0 and y == 0 then
		return 0
	end

	local a = math.atan(y/x);
	if x < 0 then
		a = a + math.pi;
	end
	return a;
end

function RenderProjectile(item)

	local ct = (engine.time_prevgametick - item.starttick) / engine.time_servertickspeed + engine.time_gameticktime;
	if ct < 0 then return end

	-- fetch curvature and speed
	local texture = 0
	local curvature = 1
	local speed = 1

	if item.type == WEAPON_GUN then
		curvature = WEAPON_GUN_CURVATURE
		speed = WEAPON_GUN_SPEED
		texture = data.texture_bullet_gun
	elseif item.type == WEAPON_SHOTGUN then
		curvature = WEAPON_SHOTGUN_CURVATURE
		speed = WEAPON_SHOTGUN_SPEED
		texture = data.texture_bullet_shotgun
	elseif item.type == WEAPON_GRENADE then
		curvature = WEAPON_GRENADE_CURVATURE
		speed = WEAPON_GRENADE_SPEED
		texture = data.texture_bullet_grenade
	end

	local vx = item.vel_x / 100
	local vy = item.vel_y / 100
	local x,y  = CalculateCurvedPosition(item.x, item.y, vx, vy, curvature, speed, ct)
	local r = 0

	if item.type == WEAPON_GUN then
		local px,py  = CalculateCurvedPosition(item.x, item.y, vx, vy, curvature, speed, ct - 0.001)
		r = AngleFromVector(x-px, y-py)
	elseif item.type == WEAPON_GRENADE then
		r = engine.time_localtime*math.pi*4
	end

	engine.Graphics_DrawQuad(texture, x, y, 32, 32, r)

	-- TODO: add particles
end

function RenderPickup(item)
	local pickupinfo = data.pickups[item.type+1]
	if not pickupinfo then return end

	local Offset = item.y/32.0 + item.x/32.0;
	local x = item.x + math.cos(engine.time_localtime*2.0+Offset) * 2.5
	local y = item.y + math.sin(engine.time_localtime*2.0+Offset) * 2.5

	engine.Graphics_DrawQuad(pickupinfo.tex, x, y, pickupinfo.w, pickupinfo.h, 0)
end

function OnRender()
	local i = 0
	local num = engine.Snap_NumItems()
	for i = 0, num-1 do
		local item = engine.Snap_GetItem(i)
		if item then
			if item._type == SNAPITEM_PROJECTILE then RenderProjectile(item) end
			if item._type == SNAPITEM_PICKUP then RenderPickup(item) end
		end
	end
end

function vLength(x,y)
	return math.sqrt(x*x+y*y)
end

function vNormalize(x,y)
	local l = vLength(x, y)
	return x/l,y/l
end


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

function X(input, player)
	local PHYS_SIZE = 28.0
	--m_TriggeredEvents = 0;

	-- get ground state
	local grounded = false
	if engine.Physics_CheckPoint(player.pos_x+PHYS_SIZE/2, player.pos_y+PHYS_SIZE/2+5) then
		grounded = true
	elseif engine.Physics_CheckPoint(player.pos_x-PHYS_SIZE/2, player.pos_y+PHYS_SIZE/2+5) then
		grounded = true
	end
	

	local target_dir_x, target_dir_y = vNormalize(input.target_x, input.target_y)

	player.vel_y = player.vel_y + tuning.gravity

	local maxspeed = tuning.aircontrolspeed
	local accel = tuning.aircontrolaccel
	local friction = tuning.airfriction

	if grounded then
		maxspeed = tuning.groundcontrolspeed
		accel = tuning.groundcontrolaccel
		friction = tuning.m_groundfriction
	end

	--[[ handle input
	if(UseInput)
	{
		m_Direction = m_Input.m_Direction;

		// setup angle
		float a = 0;
		if(m_Input.m_TargetX == 0)
			a = atanf((float)m_Input.m_TargetY);
		else
			a = atanf((float)m_Input.m_TargetY/(float)m_Input.m_TargetX);

		if(m_Input.m_TargetX < 0)
			a = a+pi;

		m_Angle = (int)(a*256.0f);

		// handle jump
		if(m_Input.m_Jump)
		{
			if(!(m_Jumped&1))
			{
				if(Grounded)
				{
					m_TriggeredEvents |= COREEVENT_GROUND_JUMP;
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

		// handle hook
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
		}
	}]]

	-- add the speed modification according to players wanted direction
	if player.direction < 0 then
		player.vel_x = SaturatedAdd(-maxspeed, maxspeed, player.vel_x, -accel)
	elseif player.direction > 0 then
		player.vel_x = SaturatedAdd(-maxspeed, maxspeed, player.vel_x, accel)
	elseif player.direction == 0 then
		player.vel_x = player.vel_x * friction
	end

	-- handle jumping
	-- 1 bit = to keep track if a jump has been made on this input
	-- 2 bit = to keep track if a air-jump has been made
	if grounded then
		-- player.jumped &= ~2
		if player.jumped > 1 then
			player.jumped = player.jumped - 2
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
	if vLength(player.vel_x, player.vel_y) > 6000 then
		player.vel_x, player.vel_y = vNormalize(player.vel_x, player.vel_y) * 6000;
	end
end
--[[

-- Physics_CheckPoint

-- TODO: rendering
DrawQuadTL(t,x,y,w,h,r)
?DrawFreeform
]]