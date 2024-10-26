// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#pragma pack(push, 8)

#define EOS_LOBBYDETAILS_INFO_API_001 1

EOS_STRUCT(EOS_LobbyDetails_Info001, (
	/** API Version: Set this to EOS_LOBBYDETAILS_INFO_API_LATEST. */
	int32_t ApiVersion;
	/** Lobby ID */
	EOS_LobbyId LobbyId;
	/** The Product User ID of the current owner of the lobby */
	EOS_ProductUserId LobbyOwnerUserId;
	/** Permission level of the lobby */
	EOS_ELobbyPermissionLevel PermissionLevel;
	/** Current available space */
	uint32_t AvailableSlots;
	/** Max allowed members in the lobby */
	uint32_t MaxMembers;
	/** If true, users can invite others to this lobby */
	EOS_Bool bAllowInvites;
));

/**
 * Function prototype definition for callbacks passed to EOS_Lobby_JoinLobby
 * @param Data A EOS_Lobby_JoinLobby CallbackInfo containing the output information and result
 */
EOS_DECLARE_CALLBACK(EOS_Lobby_OnJoinLobbyCallback, const EOS_Lobby_JoinLobbyCallbackInfo* Data);

#pragma pack(pop)
