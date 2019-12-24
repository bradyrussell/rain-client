// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VaRestRequestJSON.h"
#include "BPFL_RainAPI.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType) struct FApiData{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite)	FString API_URL;
	UPROPERTY(BlueprintReadWrite)	FString API_Key;
};

USTRUCT(BlueprintType) struct FCloudSaveData{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(BlueprintReadWrite)	int32 GameID;
	UPROPERTY(BlueprintReadWrite)	FString Name;
	UPROPERTY(BlueprintReadWrite)	FString Format;
	UPROPERTY(BlueprintReadWrite)	int32 Version;
	UPROPERTY(BlueprintReadWrite)	FString Meta;
	UPROPERTY(BlueprintReadWrite)	FString Tags;
};

UCLASS()
class RAINAPIPLUGIN_API UBPFL_RainAPI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static void API_Request(UObject* WorldContextObject, FString api, FString endpoint, FString api_key, TArray<uint8> data);

	///////////////////////////////////////////////////
	
	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_Publish(UObject* WorldContextObject, FApiData Api, FCloudSaveData Save, TArray<uint8> Data, FString& URL);

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_Retrieve(UObject* WorldContextObject, FApiData Api, int32 ID, FString& URL);

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_CloudSave_List(UObject* WorldContextObject, FApiData Api, FString& URL);
	
	UFUNCTION(BlueprintCallable)
	static FString Rain_GameSession_GetFromLauncher();

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_Begin(UObject* WorldContextObject, FApiData Api, int32 GameID, FString Username, FString Password, FString& URL);
	
	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_End(UObject* WorldContextObject, FApiData Api, int32 GameID, FString SessionKey, FString& URL);
	
	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static UVaRestRequestJSON* Rain_GameSession_Verify(UObject* WorldContextObject, FApiData Api, int32 GameID, FString SessionKey, FString& URL);

	UFUNCTION(BlueprintCallable)
	static FString Rain_GameSession_GetFromOptions(TArray<FString> Options);

	UFUNCTION(BlueprintCallable,  meta = (WorldContext = "WorldContextObject"))
	static void Rain_Matchmaking_JoinServerWithSession(UObject* WorldContextObject, FString Server, FString Session);
	///////////////////////////////////////////////////
	
	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_StringToBytes(FString Data);

	UFUNCTION(BlueprintCallable)
	static FString Rain_BytesToString(TArray<uint8> Data);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_AesEncryptBytes( UPARAM(ref) TArray<uint8> Input,  UPARAM(ref) TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_AesDecryptBytes( UPARAM(ref) TArray<uint8> Input,  UPARAM(ref) TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_ZipAndEncryptBytes(TArray<uint8> Input, TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static TArray<uint8> Rain_UnzipAndDecryptBytes(TArray<uint8> Input, TArray<uint8> Key);

	UFUNCTION(BlueprintCallable)
	static FString Rain_HashBytes(TArray<uint8> Data);

	UFUNCTION(BlueprintCallable)
	static void Rain_PrintHexBytes(TArray<uint8> Data);
	
};
