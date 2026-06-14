[ComponentEditorProps(category: "TBD/Framework", description: "Server-only: slot assignment + per-slot SCR_SpawnPoint entities from mission JSON.")]
class TBD_SpawnManagerClass : SCR_BaseGameModeComponentClass {}

//! Builds one SCR_SpawnPoint per mission slots[] entry at exact JSON coordinates.
//! Assigns each player a slot (roster identity → slotId, else round-robin).
class TBD_SpawnManager : SCR_BaseGameModeComponent
{
	protected const ResourceName SPAWN_POINT_PREFAB = "{E7F4D5562F48DDE4}Prefabs/MP/Spawning/SpawnPoint_Base.et";

	protected static TBD_SpawnManager s_Instance;

	protected ref map<int, ref TBD_MissionSlotStruct> m_mPlayerSlot;
	protected ref map<string, SCR_SpawnPoint> m_mSlotSpawnPoints;
	protected int m_iRoundRobin;
	protected bool m_bSlotSpawnPointsBuilt;

	//------------------------------------------------------------------------------------------------
	void TBD_SpawnManager(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		s_Instance = this;
		m_mPlayerSlot = new map<int, ref TBD_MissionSlotStruct>();
		m_mSlotSpawnPoints = new map<string, SCR_SpawnPoint>();
	}

	//------------------------------------------------------------------------------------------------
	static TBD_SpawnManager GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int playerId)
	{
		super.OnPlayerAuditSuccess(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Assign mission slot to player (roster or round-robin). Idempotent per player.
	void AssignSlotForPlayer(int playerId)
	{
		if (m_mPlayerSlot.Contains(playerId))
			return;

		array<ref TBD_MissionSlotStruct> slots = TBD_MissionLoader.GetSlots();
		if (!slots || slots.IsEmpty())
		{
			Print("[TBD] SpawnManager: no mission slots — cannot assign player " + playerId, LogLevel.ERROR);
			return;
		}

		string slotId = ResolveSlotIdForPlayer(playerId);
		TBD_MissionSlotStruct slot = TBD_MissionLoader.GetSlotById(slotId);
		if (!slot)
		{
			// Round-robin fallback when roster slot id unknown
			int idx = m_iRoundRobin % slots.Count();
			slot = slots[idx];
			m_iRoundRobin++;
		}

		m_mPlayerSlot.Insert(playerId, slot);
		Print(string.Format("[TBD] SpawnManager: assigned slot %1 to player %2 at (%3)", slot.id, playerId, slot.x.ToString() + "," + slot.z.ToString()));
	}

	//------------------------------------------------------------------------------------------------
	protected string ResolveSlotIdForPlayer(int playerId)
	{
		if (!TBD_RosterLoader.IsLoaded())
			return string.Empty;

		string identityId = string.Format("%1", SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));
		if (identityId.IsEmpty())
			return string.Empty;

		return TBD_RosterLoader.GetSlotForIdentity(identityId);
	}

	//------------------------------------------------------------------------------------------------
	TBD_MissionSlotStruct GetAssignedSlot(int playerId)
	{
		return m_mPlayerSlot.Get(playerId);
	}

	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPointForSlot(string slotId)
	{
		return m_mSlotSpawnPoints.Get(slotId);
	}

	//------------------------------------------------------------------------------------------------
	//! Engine faction key for mission faction key.
	string EngineFactionKey(string missionFactionKey)
	{
		switch (missionFactionKey)
		{
			case "blufor": return "US";
			case "opfor": return "USSR";
		}
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Authority-only: one SCR_SpawnPoint per mission slots[] at exact JSON coordinates.
	void BuildMissionSlotSpawnPoints()
	{
		if (m_bSlotSpawnPointsBuilt)
			return;

		array<ref TBD_MissionSlotStruct> slots = TBD_MissionLoader.GetSlots();
		if (!slots || slots.IsEmpty())
		{
			Print("[TBD] SpawnManager: no mission slots — cannot build spawn points.", LogLevel.ERROR);
			return;
		}

		Resource resource = Resource.Load(SPAWN_POINT_PREFAB);
		if (!resource || !resource.IsValid())
		{
			Print("[TBD] SpawnManager: spawn point prefab failed to load.", LogLevel.ERROR);
			return;
		}

		int built = 0;
		foreach (TBD_MissionSlotStruct slot : slots)
		{
			if (!slot)
				continue;

			string engineKey = EngineFactionKey(slot.faction);
			if (engineKey.IsEmpty())
				continue;

			float x = slot.x;
			float z = slot.z;
			vector pos = Vector(x, GetGame().GetWorld().GetSurfaceY(x, z), z);

			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			Math3D.MatrixIdentity4(params.Transform);
			params.Transform[3] = pos;

			// Apply heading from JSON (yaw around Y)
			float yawRad = slot.headingDeg * Math.DEG2RAD;
			params.Transform[0] = Vector(Math.Cos(yawRad), 0, Math.Sin(yawRad));
			params.Transform[2] = Vector(-Math.Sin(yawRad), 0, Math.Cos(yawRad));

			IEntity ent = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
			SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(ent);
			if (!sp)
			{
				Print("[TBD] SpawnManager: failed to spawn SCR_SpawnPoint for " + slot.id, LogLevel.ERROR);
				continue;
			}

			sp.SetFactionKey(engineKey);
			m_mSlotSpawnPoints.Insert(slot.id, sp);
			built++;
			Print(string.Format("[TBD] SpawnManager: built slot spawn %1 (%2) kit %3 at %4", slot.id, engineKey, slot.kit, pos.ToString()));
		}

		if (built > 0)
			m_bSlotSpawnPointsBuilt = true;
	}

	//------------------------------------------------------------------------------------------------
	void OnStageChanged(TBD_EGameStage stage)
	{
	}
}
