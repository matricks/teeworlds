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

data.texture_tee = engine.Resource_Get("skins/default.png")

data.tee = {
	body = {0, 0, 3/8, 3/4},
	body_shadow = {3/8, 0, 3/8+3/8, 3/4},
	eye = {2/8, 3/4, 2/8 + 1/8, 1},
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

function SetUV(uvset)
	engine.Graphics_SetUV(uvset[1], uvset[2], uvset[3], uvset[4])

end

function RenderTee(x, y) --, CAnimState *pAnim, CTeeRenderInfo *pInfo, int Emote, vec2 Dir, vec2 Pos)
	-- first pass we draw the outline
	-- second pass we draw the filling
	local base_size = 64
	local dir_x = 1.0
	local dir_y = 0

	for i = 0, 1 do
		local outline = i == 0
		
		if outline then
			SetUV(data.tee.body_shadow)
			engine.Graphics_DrawQuad(data.texture_tee, x, y, base_size, base_size, r)
		else
			SetUV(data.tee.body)
			engine.Graphics_DrawQuad(data.texture_tee, x, y, base_size, base_size, r)

			local eye_scale = base_size * 0.4
			SetUV(data.tee.eye)

			local eye_separation = (0.075 - 0.010*math.abs(dir_x)) * base_size
			local ox = dir_x*0.125 * base_size
			local oy = (-0.05 + dir_y * 0.10) * base_size
			engine.Graphics_DrawQuad(data.texture_tee, x-eye_separation+ox, y+oy, eye_scale, eye_scale, 0)
			engine.Graphics_DrawQuad(data.texture_tee, x+eye_separation+ox, y+oy, -eye_scale, eye_scale, 0)
		end
	end

	engine.Graphics_ResetUV()
--[[
	vec2 Direction = Dir;
	vec2 Position = Pos;

	//Graphics()->TextureSet(data->images[IMAGE_CHAR_DEFAULT].id);
	Graphics()->TextureSet(pInfo->m_pTexture);

	// TODO: FIX ME
	Graphics()->QuadsBegin();
	//Graphics()->QuadsDraw(pos.x, pos.y-128, 128, 128);

	// first pass we draw the outline
	// second pass we draw the filling
	for(int p = 0; p < 2; p++)
	{
		int OutLine = p==0 ? 1 : 0;

		for(int f = 0; f < 2; f++)
		{
			float AnimScale = pInfo->m_Size * 1.0f/64.0f;
			float BaseSize = pInfo->m_Size;
			if(f == 1)
			{
				Graphics()->QuadsSetRotation(pAnim->GetBody()->m_Angle*pi*2);

				// draw body
				Graphics()->SetColor(pInfo->m_ColorBody.r, pInfo->m_ColorBody.g, pInfo->m_ColorBody.b, pInfo->m_ColorBody.a);
				vec2 BodyPos = Position + vec2(pAnim->GetBody()->m_X, pAnim->GetBody()->m_Y)*AnimScale;
				SelectSprite(OutLine?SPRITE_TEE_BODY_OUTLINE:SPRITE_TEE_BODY, 0, 0, 0);
				IGraphics::CQuadItem QuadItem(BodyPos.x, BodyPos.y, BaseSize, BaseSize);
				Graphics()->QuadsDraw(&QuadItem, 1);

				// draw eyes
				if(p == 1)
				{
					switch (Emote)
					{
						case EMOTE_PAIN:
							SelectSprite(SPRITE_TEE_EYE_PAIN, 0, 0, 0);
							break;
						case EMOTE_HAPPY:
							SelectSprite(SPRITE_TEE_EYE_HAPPY, 0, 0, 0);
							break;
						case EMOTE_SURPRISE:
							SelectSprite(SPRITE_TEE_EYE_SURPRISE, 0, 0, 0);
							break;
						case EMOTE_ANGRY:
							SelectSprite(SPRITE_TEE_EYE_ANGRY, 0, 0, 0);
							break;
						default:
							SelectSprite(SPRITE_TEE_EYE_NORMAL, 0, 0, 0);
							break;
					}

					float EyeScale = BaseSize*0.40f;
					float h = Emote == EMOTE_BLINK ? BaseSize*0.15f : EyeScale;
					float EyeSeparation = (0.075f - 0.010f*absolute(Direction.x))*BaseSize;
					vec2 Offset = vec2(Direction.x*0.125f, -0.05f+Direction.y*0.10f)*BaseSize;
					IGraphics::CQuadItem Array[2] = {
						IGraphics::CQuadItem(BodyPos.x-EyeSeparation+Offset.x, BodyPos.y+Offset.y, EyeScale, h),
						IGraphics::CQuadItem(BodyPos.x+EyeSeparation+Offset.x, BodyPos.y+Offset.y, -EyeScale, h)};
					Graphics()->QuadsDraw(Array, 2);
				}
			}

			// draw feet
			CAnimKeyframe *pFoot = f ? pAnim->GetFrontFoot() : pAnim->GetBackFoot();

			float w = BaseSize;
			float h = BaseSize/2;

			Graphics()->QuadsSetRotation(pFoot->m_Angle*pi*2);

			bool Indicate = !pInfo->m_GotAirJump && g_Config.m_ClAirjumpindicator;
			float cs = 1.0f; // color scale

			if(OutLine)
				SelectSprite(SPRITE_TEE_FOOT_OUTLINE, 0, 0, 0);
			else
			{
				SelectSprite(SPRITE_TEE_FOOT, 0, 0, 0);
				if(Indicate)
					cs = 0.5f;
			}

			Graphics()->SetColor(pInfo->m_ColorFeet.r*cs, pInfo->m_ColorFeet.g*cs, pInfo->m_ColorFeet.b*cs, pInfo->m_ColorFeet.a);
			IGraphics::CQuadItem QuadItem(Position.x+pFoot->m_X*AnimScale, Position.y+pFoot->m_Y*AnimScale, w, h);
			Graphics()->QuadsDraw(&QuadItem, 1);
		}
	}

	Graphics()->QuadsEnd();
	]]--
end

function RenderPlayer(prev_char, char, prev_playerinfo, playerinfo)
	--[[
	CNetObj_Character Prev;
	CNetObj_Character Player;
	Prev = *pPrevChar;
	Player = *pPlayerChar;

	CNetObj_PlayerInfo pInfo = *pPlayerInfo;
	CTeeRenderInfo RenderInfo = m_pClient->m_aClients[pInfo.m_ClientID].m_RenderInfo;

	// check for teamplay modes
	bool IsTeamplay = false;
	bool NewTick = m_pClient->m_NewTick;
	if(m_pClient->m_Snap.m_pGameInfoObj)
		IsTeamplay = (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS) != 0;

	// check for ninja
	if (Player.m_Weapon == WEAPON_NINJA)
	{
		// change the skin for the player to the ninja
		int Skin = m_pClient->m_pSkins->Find("x_ninja");
		if(Skin != -1)
		{
			if(IsTeamplay)
				RenderInfo.m_pTexture = m_pClient->m_pSkins->Get(Skin)->m_pColorTexture;
			else
			{
				RenderInfo.m_pTexture = m_pClient->m_pSkins->Get(Skin)->m_pOrgTexture;
				RenderInfo.m_ColorBody = vec4(1,1,1,1);
				RenderInfo.m_ColorFeet = vec4(1,1,1,1);
			}
		}
	}

	// set size
	RenderInfo.m_Size = 64.0f;

	float IntraTick = Client()->IntraGameTick();

	float Angle = mix((float)Prev.m_Angle, (float)Player.m_Angle, IntraTick)/256.0f;

	//float angle = 0;

	if(pInfo.m_Local && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		// just use the direct input if it's local player we are rendering
		Angle = GetAngle(m_pClient->m_pControls->m_MousePos);
	}
	else
	{
		/*
		float mixspeed = Client()->FrameTime()*2.5f;
		if(player.attacktick != prev.attacktick) // shooting boosts the mixing speed
			mixspeed *= 15.0f;

		// move the delta on a constant speed on a x^2 curve
		float current = g_GameClient.m_aClients[info.cid].angle;
		float target = player.angle/256.0f;
		float delta = angular_distance(current, target);
		float sign = delta < 0 ? -1 : 1;
		float new_delta = delta - 2*mixspeed*sqrt(delta*sign)*sign + mixspeed*mixspeed;

		// make sure that it doesn't vibrate when it's still
		if(fabs(delta) < 2/256.0f)
			angle = target;
		else
			angle = angular_approach(current, target, fabs(delta-new_delta));

		g_GameClient.m_aClients[info.cid].angle = angle;*/
	}

	// use preditect players if needed
	if(pInfo.m_Local && g_Config.m_ClPredict && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(!m_pClient->m_Snap.m_pLocalCharacter || (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
		{
		}
		else
		{
			// apply predicted results
			m_pClient->m_PredictedChar.Write(&Player);
			m_pClient->m_PredictedPrevChar.Write(&Prev);
			IntraTick = Client()->PredIntraGameTick();
			NewTick = m_pClient->m_NewPredictedTick;
		}
	}

	vec2 Direction = GetDirection((int)(Angle*256.0f));
	vec2 Position = mix(vec2(Prev.m_X, Prev.m_Y), vec2(Player.m_X, Player.m_Y), IntraTick);
	vec2 Vel = mix(vec2(Prev.m_VelX/256.0f, Prev.m_VelY/256.0f), vec2(Player.m_VelX/256.0f, Player.m_VelY/256.0f), IntraTick);

	m_pClient->m_pFlow->Add(Position, Vel*100.0f, 10.0f);

	RenderInfo.m_GotAirJump = Player.m_Jumped&2?0:1;


	// detect events
	if(NewTick)
	{
		// detect air jump
		if(!RenderInfo.m_GotAirJump && !(Prev.m_Jumped&2))
			m_pClient->m_pEffects->AirJump(Position);
	}

	bool Stationary = Player.m_VelX <= 1 && Player.m_VelX >= -1;
	bool InAir = !Collision()->CheckPoint(Player.m_X, Player.m_Y+16);
	bool WantOtherDir = (Player.m_Direction == -1 && Vel.x > 0) || (Player.m_Direction == 1 && Vel.x < 0);

	// evaluate animation
	float WalkTime = fmod(absolute(Position.x), 100.0f)/100.0f;
	CAnimState State;
	State.Set(&g_pData->m_aAnimations[ANIM_BASE], 0);

	if(InAir)
		State.Add(&g_pData->m_aAnimations[ANIM_INAIR], 0, 1.0f); // TODO: some sort of time here
	else if(Stationary)
		State.Add(&g_pData->m_aAnimations[ANIM_IDLE], 0, 1.0f); // TODO: some sort of time here
	else if(!WantOtherDir)
		State.Add(&g_pData->m_aAnimations[ANIM_WALK], WalkTime, 1.0f);

	if (Player.m_Weapon == WEAPON_HAMMER)
	{
		float ct = (Client()->PrevGameTick()-Player.m_AttackTick)/(float)SERVER_TICK_SPEED + Client()->GameTickTime();
		State.Add(&g_pData->m_aAnimations[ANIM_HAMMER_SWING], clamp(ct*5.0f,0.0f,1.0f), 1.0f);
	}
	if (Player.m_Weapon == WEAPON_NINJA)
	{
		float ct = (Client()->PrevGameTick()-Player.m_AttackTick)/(float)SERVER_TICK_SPEED + Client()->GameTickTime();
		State.Add(&g_pData->m_aAnimations[ANIM_NINJA_SWING], clamp(ct*2.0f,0.0f,1.0f), 1.0f);
	}

	// do skidding
	if(!InAir && WantOtherDir && length(Vel*50) > 500.0f)
	{
		static int64 SkidSoundTime = 0;
		if(time_get()-SkidSoundTime > time_freq()/10)
		{LF_Graphics_ResetUV
			m_pClient->m_pSounds->Play(CSounds::CHN_WORLD, SOUND_PLAYER_SKID, 0.25f, Position);
			SkidSoundTime = time_get();
		}

		m_pClient->m_pEffects->SkidTrail(
			Position+vec2(-Player.m_Direction*6,12),
			vec2(-Player.m_Direction*100*length(Vel),-50)
		);
	}

	// draw gun
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_pResource);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(State.GetAttach()->m_Angle*pi*2+Angle);

		// normal weapons
		int iw = clamp(Player.m_Weapon, 0, NUM_WEAPONS-1);
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody, Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);

		vec2 Dir = Direction;
		float Recoil = 0.0f;
		vec2 p;
		if (Player.m_Weapon == WEAPON_HAMMER)
		{
			// Static position for hammer
			p = Position + vec2(State.GetAttach()->m_X, State.GetAttach()->m_Y);
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
			// if attack is under way, bash stuffs
			if(Direction.x < 0)
			{
				Graphics()->QuadsSetRotation(-pi/2-State.GetAttach()->m_Angle*pi*2);
				p.x -= g_pData->m_Weapons.m_aId[iw].m_Offsetx;
			}
			else
			{
				Graphics()->QuadsSetRotation(-pi/2+State.GetAttach()->m_Angle*pi*2);
			}
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}
		else if (Player.m_Weapon == WEAPON_NINJA)
		{
			p = Position;
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;

			if(Direction.x < 0)
			{
				Graphics()->QuadsSetRotation(-pi/2-State.GetAttach()->m_Angle*pi*2);
				p.x -= g_pData->m_Weapons.m_aId[iw].m_Offsetx;
				m_pClient->m_pEffects->PowerupShine(p+vec2(32,0), vec2(32,12));
			}
			else
			{
				Graphics()->QuadsSetRotation(-pi/2+State.GetAttach()->m_Angle*pi*2);
				m_pClient->m_pEffects->PowerupShine(p-vec2(32,0), vec2(32,12));
			}
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);

			// HADOKEN
			if ((Client()->GameTick()-Player.m_AttackTick) <= (SERVER_TICK_SPEED / 6) && g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)
			{
				int IteX = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
				if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
				{
					static int s_LastIteX = IteX;
					const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
					if(pInfo->m_Paused)
						IteX = s_LastIteX;
					else
						s_LastIteX = IteX;
				}
				if(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX])
				{
					vec2 Dir = vec2(pPlayerChar->m_X,pPlayerChar->m_Y) - vec2(pPrevChar->m_X, pPrevChar->m_Y);
					Dir = normalize(Dir);
					float HadOkenAngle = GetAngle(Dir);
					Graphics()->QuadsSetRotation(HadOkenAngle );
					//float offsety = -data->weapons[iw].muzzleoffsety;
					RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], 0);
					vec2 DirY(-Dir.y,Dir.x);
					p = Position;
					float OffsetX = g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsetx;
					p -= Dir * OffsetX;
					RenderTools()->DrawSprite(p.x, p.y, 160.0f);
				}
			}
		}
		else
		{
			// TODO: should be an animation
			Recoil = 0;
			float a = (Client()->GameTick()-Player.m_AttackTick+IntraTick)/5.0f;
			if(a < 1)
				Recoil = sinf(a*pi);
			p = Position + Dir * g_pData->m_Weapons.m_aId[iw].m_Offsetx - Dir*Recoil*10.0f;
			p.y += g_pData->m_Weapons.m_aId[iw].m_Offsety;
			RenderTools()->DrawSprite(p.x, p.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
		}

		if (Player.m_Weapon == WEAPON_GUN || Player.m_Weapon == WEAPON_SHOTGUN)
		{
			// check if we're firing stuff
			if(g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles)//prev.attackticks)
			{
				float Alpha = 0.0f;
				int Phase1Tick = (Client()->GameTick() - Player.m_AttackTick);
				if (Phase1Tick < (g_pData->m_Weapons.m_aId[iw].m_Muzzleduration + 3))
				{
					float t = ((((float)Phase1Tick) + IntraTick)/(float)g_pData->m_Weapons.m_aId[iw].m_Muzzleduration);
					Alpha = mix(2.0f, 0.0f, min(1.0f,max(0.0f,t)));
				}

				int IteX = rand() % g_pData->m_Weapons.m_aId[iw].m_NumSpriteMuzzles;
				if (Alpha > 0.0f && g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX])
				{
					float OffsetY = -g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsety;
					RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_aSpriteMuzzles[IteX], Direction.x < 0 ? SPRITE_FLAG_FLIP_Y : 0);
					if(Direction.x < 0)
						OffsetY = -OffsetY;

					vec2 DirY(-Dir.y,Dir.x);
					vec2 MuzzlePos = p + Dir * g_pData->m_Weapons.m_aId[iw].m_Muzzleoffsetx + DirY * OffsetY;

					RenderTools()->DrawSprite(MuzzlePos.x, MuzzlePos.y, g_pData->m_Weapons.m_aId[iw].m_VisualSize);
				}
			}
		}
		Graphics()->QuadsEnd();

		switch (Player.m_Weapon)
		{
			case WEAPON_GUN: RenderHand(&RenderInfo, p, Direction, -3*pi/4, vec2(-15, 4)); break;
			case WEAPON_SHOTGUN: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-5, 4)); break;
			case WEAPON_GRENADE: RenderHand(&RenderInfo, p, Direction, -pi/2, vec2(-4, 7)); break;
		}

	}

	// render the "shadow" tee
	if(pInfo.m_Local && g_Config.m_Debug)
	{
		vec2 GhostPosition = mix(vec2(pPrevChar->m_X, pPrevChar->m_Y), vec2(pPlayerChar->m_X, pPlayerChar->m_Y), Client()->IntraGameTick());
		CTeeRenderInfo Ghost = RenderInfo;
		Ghost.m_ColorBody.a = 0.5f;
		Ghost.m_ColorFeet.a = 0.5f;
		RenderTools()->RenderTee(&State, &Ghost, Player.m_Emote, Direction, GhostPosition); // render ghost
	}

	RenderInfo.m_Size = 64.0f; // force some settings
	RenderInfo.m_ColorBody.a = 1.0f;
	RenderInfo.m_ColorFeet.a = 1.0f;
	RenderTools()->RenderTee(&State, &RenderInfo, Player.m_Emote, Direction, Position);

	if(Player.m_PlayerFlags&PLAYERFLAG_CHATTING)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_pResource);
		Graphics()->QuadsBegin();
		RenderTools()->SelectSprite(SPRITE_DOTDOT);
		IGraphics::CQuadItem QuadItem(Position.x + 24, Position.y - 40, 64,64);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	if (m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart != -1 && m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() > Client()->GameTick())
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_EMOTICONS].m_pResource);
		Graphics()->QuadsBegin();

		int SinceStart = Client()->GameTick() - m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart;
		int FromEnd = m_pClient->m_aClients[pInfo.m_ClientID].m_EmoticonStart + 2 * Client()->GameTickSpeed() - Client()->GameTick();

		float a = 1;

		if (FromEnd < Client()->GameTickSpeed() / 5)
			a = FromEnd / (Client()->GameTickSpeed() / 5.0);

		float h = 1;
		if (SinceStart < Client()->GameTickSpeed() / 10)
			h = SinceStart / (Client()->GameTickSpeed() / 10.0);

		float Wiggle = 0;
		if (SinceStart < Client()->GameTickSpeed() / 5)
			Wiggle = SinceStart / (Client()->GameTickSpeed() / 5.0);

		float WiggleAngle = sinf(5*Wiggle);

		Graphics()->QuadsSetRotation(pi/6*WiggleAngle);

		Graphics()->SetColor(1.0f,1.0f,1.0f,a);
		// client_datas::emoticon is an offset from the first emoticon
		RenderTools()->SelectSprite(SPRITE_OOP + m_pClient->m_aClients[pInfo.m_ClientID].m_Emoticon);
		IGraphics::CQuadItem QuadItem(Position.x, Position.y - 23 - 32*h, 64, 64*h);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
	]]--
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

	RenderTee(500, 300)
end

function vLength(x,y)
	return math.sqrt(x*x+y*y)
end

function vNormalize(x,y)
	local l = vLength(x, y)
	return x/l, y/l
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

function X(player, input, use_input)
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

	--[[ handle input ]]--
	if use_input then
		player.direction = input.direction;

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

		player.angle = a*256.0

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