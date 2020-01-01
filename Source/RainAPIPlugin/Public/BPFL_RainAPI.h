// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VaRestRequestJSON.h"
#include "BPFL_RainAPI.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType) struct FApiData {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite) FString API_URL;
	UPROPERTY(BlueprintReadWrite) FString API_Key;
	UPROPERTY(BlueprintReadWrite) int32 GameID;
};

USTRUCT(BlueprintType) struct FCloudSaveData {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite) int32 GameID;
	UPROPERTY(BlueprintReadWrite) FString Name;
	UPROPERTY(BlueprintReadWrite) FString Format;
	UPROPERTY(BlueprintReadWrite) int32 Version;
	UPROPERTY(BlueprintReadWrite) FString Meta;
	UPROPERTY(BlueprintReadWrite) FString Tags;
};

UENUM(BlueprintType) 
enum class EServerState : uint8 {
	Unavailable,
	Initializing,
	WaitingMatchEnd,
	Terminating,
	Error,
	WaitingForPlayers,
	WaitingMatchStart,
	InProgress
};

USTRUCT(BlueprintType) struct FServerState {
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 ServerID;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 Port;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 CurrentPlayers;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 MaxPlayers;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) EServerState State;
};

UCLASS()
class RAINAPIPLUGIN_API UBPFL_RainAPI : public UBlueprintFunctionLibrary {
	GENERATED_BODY()


public:
	//static FApiData API_DATA;

	UFUNCTION(BlueprintCallable)
	static void SetAPI(FApiData Api);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void API_Request(UObject* WorldContextObject, FString api, FString endpoint, FString api_key, TArray<uint8> data);

	///////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_Publish(UObject* WorldContextObject, FCloudSaveData Save, TArray<uint8> Data, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_Retrieve(UObject* WorldContextObject, int32 ID, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_List(UObject* WorldContextObject, FString& URL);

	UFUNCTION(BlueprintCallable)
	static FString Rain_GameSession_GetFromLauncher();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_Begin(UObject* WorldContextObject, FString Username, FString Password, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_End(UObject* WorldContextObject, FString SessionKey, FString& URL);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_Matchmaking_Request_Create(UObject* WorldContextObject, FString SessionKey, uint8 Region, int32 PlaylistID, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_Matchmaking_Request_Check(UObject* WorldContextObject, FString SessionKey, int32 RequestID, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_Verify(UObject* WorldContextObject, FString SessionKey, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameServer_Begin(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameServer_End(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameServer_Heartbeat(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL);

	UFUNCTION(BlueprintCallable)
	static FString Rain_GetParameterFromOptions(FString Parameter, TArray<FString> Options);

	UFUNCTION(BlueprintCallable)
	static FString Rain_GameSecretKey_GetFromLauncher();

	UFUNCTION(BlueprintCallable)
	static int32 Rain_GameServerID_GetFromLauncher();

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void Rain_Matchmaking_JoinServerWithSession(UObject* WorldContextObject, FString Server, FString Options, FString Session);
	///////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_StringToBytes(FString Data);

	UFUNCTION(BlueprintCallable)
	static FString Rain_BytesToString(TArray<uint8> Data);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_AesEncryptBytes(UPARAM(ref) TArray<uint8> Input, UPARAM(ref) TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_AesDecryptBytes(UPARAM(ref) TArray<uint8> Input, UPARAM(ref) TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_ZipAndEncryptBytes(TArray<uint8> Input, TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_UnzipAndDecryptBytes(TArray<uint8> Input, TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static FString Rain_HashBytes(TArray<uint8> Data);

	UFUNCTION(BlueprintCallable)
	static void Rain_PrintHexBytes(TArray<uint8> Data);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static int32 Rain_GetCurrentServerPort(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	static FString Rain_GetPlayerAddress(APlayerController* PlayerController, bool bWithPort);

};
