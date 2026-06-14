//! Client<->server transport for the admin mission browser. Works on dedicated
//! servers (no chat dependency): the admin's client RPCs the server, the server
//! validates the player is a listed admin and drives TBD_FrameworkManager.
//!
//! Added as methods on the player controller (which is replicated and owned by
//! one client), so server->owner replies route to the requesting admin only.
modded class SCR_PlayerController
{
	//! Client-side cache of the last received mission list (display lines).
	protected ref array<string> m_TBD_MissionLines;

	//------------------------------------------------------------------------------------------------
	// CLIENT (owner) -> SERVER: ask for the current mission list.
	void TBD_RequestMissionList()
	{
		Rpc(TBD_RpcAsk_MissionList);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void TBD_RpcAsk_MissionList()
	{
		string payload = TBD_MissionBrowserService.BuildListPayload();
		Rpc(TBD_RpcDo_ReceiveMissionList, payload);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TBD_RpcDo_ReceiveMissionList(string payload)
	{
		m_TBD_MissionLines = TBD_MissionBrowserService.ParseListPayload(payload);
		foreach (string line : m_TBD_MissionLines)
			Print("[TBD][browser] " + line);
	}

	//------------------------------------------------------------------------------------------------
	// CLIENT (owner) -> SERVER: select mission by 1-based number.
	void TBD_RequestSelectMission(int number)
	{
		Rpc(TBD_RpcAsk_SelectMission, number);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void TBD_RpcAsk_SelectMission(int number)
	{
		int playerId = GetPlayerId();

		SCR_PlayerListedAdminManagerComponent admins = SCR_PlayerListedAdminManagerComponent.GetInstance();
		if (!admins || !admins.IsPlayerOnAdminList(playerId))
		{
			Print(string.Format("[TBD][browser] non-admin player %1 tried to select mission %2 — denied.", playerId, number), LogLevel.WARNING);
			return;
		}

		TBD_FrameworkManager fm = TBD_FrameworkManager.GetInstance();
		if (!fm)
			return;

		string status = fm.SelectMissionByNumber(number);
		Print(string.Format("[TBD][browser] admin %1 -> %2", playerId, status));
	}

	//------------------------------------------------------------------------------------------------
	//! Client-side accessor for the cached list (for a future menu/HUD).
	array<string> TBD_GetMissionLines()
	{
		return m_TBD_MissionLines;
	}
}

//! Serializes the server mission list to a newline-delimited payload and parses
//! it back on the client. Keeps the RPC signature to a single string.
class TBD_MissionBrowserService
{
	//------------------------------------------------------------------------------------------------
	//! Server: build "n) name [terrain] N slots" lines from the cached list.
	static string BuildListPayload()
	{
		array<ref TBD_MissionListEntry> entries = TBD_MissionListLoader.GetEntries();
		if (!entries || entries.IsEmpty())
			return "No missions loaded yet.";

		string out;
		for (int i = 0; i < entries.Count(); i++)
		{
			TBD_MissionListEntry e = entries[i];
			if (i > 0)
				out = out + "\n";
			out = out + string.Format("%1) %2 [%3] %4 slots", i + 1, e.name, e.terrain, e.slotCount);
		}
		return out;
	}

	//------------------------------------------------------------------------------------------------
	//! Client: split the payload back into display lines.
	static array<string> ParseListPayload(string payload)
	{
		array<string> lines = new array<string>();
		payload.Split("\n", lines, false);
		return lines;
	}
}
