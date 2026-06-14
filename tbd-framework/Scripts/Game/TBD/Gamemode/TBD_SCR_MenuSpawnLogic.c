//! Slot-based deploy: overrides vanilla menu spawn to use mission slots[] position + kit.
modded class SCR_MenuSpawnLogic
{
	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess_S(int playerId)
	{
		TBD_SpawnManager sm = TBD_SpawnManager.GetInstance();
		if (sm)
			sm.AssignSlotForPlayer(playerId);

		super.OnPlayerAuditSuccess_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void DoSpawn_S(int playerId)
	{
		TBD_SpawnManager sm = TBD_SpawnManager.GetInstance();
		if (!sm)
		{
			super.DoSpawn_S(playerId);
			return;
		}

		TBD_MissionSlotStruct slot = sm.GetAssignedSlot(playerId);
		if (!slot)
		{
			Print("[TBD] SpawnLogic: no slot assigned for player " + playerId, LogLevel.ERROR);
			return;
		}

		SCR_SpawnPoint sp = sm.GetSpawnPointForSlot(slot.id);
		if (!sp)
		{
			Print("[TBD] SpawnLogic: no spawn point for slot " + slot.id, LogLevel.ERROR);
			return;
		}

		bool kitOk;
		ResourceName prefab = TBD_Registry.Resolve(slot.kit, kitOk);
		if (!kitOk || prefab.IsEmpty())
		{
			Print("[TBD] SpawnLogic: kit resolve failed: " + slot.kit, LogLevel.ERROR);
			return;
		}

		SCR_PlayerFactionAffiliationComponent factionComp = GetPlayerFactionComponent_S(playerId);
		if (factionComp)
		{
			string engineKey = sm.EngineFactionKey(slot.faction);
			factionComp.SetAffiliatedFactionByKey(engineKey);
		}

		RplComponent rpl = RplComponent.Cast(sp.FindComponent(RplComponent));
		if (!rpl)
		{
			Print("[TBD] SpawnLogic: spawn point missing RplComponent for " + slot.id, LogLevel.ERROR);
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(prefab, rpl.Id());
		SCR_RespawnComponent respawn = GetPlayerRespawnComponent_S(playerId);
		if (!respawn || !respawn.RequestSpawn(data))
		{
			Print("[TBD] SpawnLogic: RequestSpawn failed for slot " + slot.id, LogLevel.ERROR);
			return;
		}

		Print(string.Format("[TBD] SpawnLogic: spawn requested player %1 slot %2 kit %3", playerId, slot.id, slot.kit));
	}
}
