// Fill out your copyright notice in the Description page of Project Settings.

#include "BPFL_RainAPI.h"
#include "VaRestLibrary.h"
#include "VaRestRequestJSON.h"
#include "Containers/UnrealString.h"
#include "AES.h"
#include "BufferArchive.h"
#include "MemoryReader.h"
#include "ArchiveSaveCompressedProxy.h"
#include "ArchiveLoadCompressedProxy.h"
#include "hash.h"
#include "CommandLine.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetConnection.h"
//#include "BPFL_RainAPI.generated.h"

static FApiData API_DATA;

int32 TranslateState(EServerState State) {
	switch (State) {
		case EServerState::Unavailable: return 0;
		case EServerState::Initializing: return 1;
		case EServerState::WaitingMatchEnd: return 50;
		case EServerState::Terminating: return 99;
		case EServerState::Error: return 100;
		case EServerState::WaitingForPlayers: return 200;
		case EServerState::WaitingMatchStart: return 201;
		case EServerState::InProgress: return 250;
		default: return 100;
	}
}

void UBPFL_RainAPI::SetAPI(FApiData Api) {
	API_DATA = Api;
}

void UBPFL_RainAPI::API_Request(UObject* WorldContextObject, FString api, FString endpoint, FString api_key, TArray<uint8> data) {
	FLatentActionInfo latent;
	UVaRestJsonObject *result;
	
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::POST);
	//request->SetContentType(ERequestContentType::json);
	request->SetContentType(ERequestContentType::binary);
	
	request->SetBinaryContentType("application/octet-stream");
	request->SetBinaryRequestContent(data);
	
	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!api_key.IsEmpty()) request->SetHeader("Authorization","Bearer "+api_key);
	
	request->OnStaticRequestComplete.AddLambda([](UVaRestRequestJSON* request) {
		int32 a = request->GetResponseCode();
		UE_LOG(LogTemp, Warning, TEXT("Request has completed. %d"), a);
	});
	
	request->ApplyURL(api+endpoint, result, WorldContextObject, latent);
	
}

//////////////////////////////////////////////////////////////

UVaRestRequestJSON* UBPFL_RainAPI::Rain_CloudSave_Publish(UObject* WorldContextObject, FCloudSaveData Save, TArray<uint8> Data, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::POST);
	request->SetContentType(ERequestContentType::binary);
	
	request->SetBinaryContentType("application/octet-stream");
	request->SetBinaryRequestContent(Data);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	Save.Name = UVaRestLibrary::PercentEncode(Save.Name);
	Save.Format = UVaRestLibrary::PercentEncode(Save.Format);
	Save.Meta = UVaRestLibrary::PercentEncode(Save.Meta);
	Save.Tags = UVaRestLibrary::PercentEncode(Save.Tags);

	const FString Endpoint = "save";
	
	URL = FString::Printf(TEXT("%s%s?game_id=%d&name=%s&format=%s&version=%d&meta=%s&tags=%s"), *API_DATA.API_URL, *Endpoint, Save.GameID, *Save.Name, *Save.Format, Save.Version, *Save.Meta, *Save.Tags);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_CloudSave_Retrieve(UObject* WorldContextObject, int32 ID, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	const FString Endpoint = "save";
	
	URL = FString::Printf(TEXT("%s%s/%d"), *API_DATA.API_URL, *Endpoint, ID);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_CloudSave_List(UObject* WorldContextObject, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	const FString Endpoint = "save";

	URL = FString::Printf(TEXT("%s%s"), *API_DATA.API_URL, *Endpoint);
	request->SetURL(URL);
	return request;
}


UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameSession_Verify(UObject* WorldContextObject, FString SessionKey, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");

	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);
	
	const FString Endpoint = "gamesession/verify";

	URL = FString::Printf(TEXT("%s%s/%d/%s"), *API_DATA.API_URL, *Endpoint, API_DATA.GameID, *SessionKey);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameServer_Begin(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL) {
		auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	const FString Endpoint = "matchmaking/server/up";

	FString args = FString::Printf(TEXT("?state=%d&port=%d&current_players=%d&max_players=%d"), TranslateState(State.State),State.Port,State.CurrentPlayers, State.MaxPlayers);

	URL = FString::Printf(TEXT("%s%s/%d/%s%s"), *API_DATA.API_URL, *Endpoint, State.ServerID, *GameSecretKey, *args);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameServer_End(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL) {
			auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	const FString Endpoint = "matchmaking/server/down";

	FString args = FString::Printf(TEXT("?state=%d&port=%d&current_players=%d&max_players=%d"), TranslateState(State.State),State.Port,State.CurrentPlayers, State.MaxPlayers);

	URL = FString::Printf(TEXT("%s%s/%d/%s%s"), *API_DATA.API_URL, *Endpoint, State.ServerID, *GameSecretKey, *args);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameServer_Heartbeat(UObject* WorldContextObject, FString GameSecretKey, FServerState State, FString& URL) {
			auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);
	
	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");
	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);

	const FString Endpoint = "matchmaking/server/tick";

	FString args = FString::Printf(TEXT("?state=%d&port=%d&current_players=%d&max_players=%d"), TranslateState(State.State),State.Port,State.CurrentPlayers, State.MaxPlayers);

	URL = FString::Printf(TEXT("%s%s/%d/%s%s"), *API_DATA.API_URL, *Endpoint, State.ServerID, *GameSecretKey, *args);
	request->SetURL(URL);
	return request;
}


UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameSession_Begin(UObject* WorldContextObject, FString Username, FString Password, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");

	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);
	
	const FString Endpoint = "gamesession/begin";

	URL = FString::Printf(TEXT("%s%s/%d/%s/%s"), *API_DATA.API_URL, *Endpoint, API_DATA.GameID, *Username, *Password);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_GameSession_End(UObject* WorldContextObject, FString SessionKey, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");

	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);
	
	const FString Endpoint = "gamesession/end";

	URL = FString::Printf(TEXT("%s%s/%d/%s"), *API_DATA.API_URL, *Endpoint, API_DATA.GameID, *SessionKey);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_Matchmaking_Request_Create(UObject* WorldContextObject, FString SessionKey, uint8 Region, int32 PlaylistID, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");

	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);
	
	const FString Endpoint = "matchmaking/request/new";

	URL = FString::Printf(TEXT("%s%s/%d/%d/%s"), *API_DATA.API_URL, *Endpoint, Region, PlaylistID,  *SessionKey);
	request->SetURL(URL);
	return request;
}

UVaRestRequestJSON* UBPFL_RainAPI::Rain_Matchmaking_Request_Check(UObject* WorldContextObject, FString SessionKey, int32 RequestID, FString& URL) {
	auto request = UVaRestRequestJSON::ConstructRequest(WorldContextObject);

	request->SetVerb(ERequestVerb::GET);
	request->SetContentType(ERequestContentType::x_www_form_urlencoded_url);

	request->SetHeader("Accept","application/json");
	request->SetHeader("Cache-Control","no-cache");

	if(!API_DATA.API_Key.IsEmpty()) request->SetHeader("Authorization","Bearer "+API_DATA.API_Key);
	
	const FString Endpoint = "matchmaking/request";

	URL = FString::Printf(TEXT("%s%s/%d/%s"), *API_DATA.API_URL, *Endpoint, RequestID,  *SessionKey);
	request->SetURL(URL);
	return request;
}

////////////////////////////////////////////////////////

TArray<uint8> UBPFL_RainAPI::Rain_StringToBytes(FString Data) {
	TArray<uint8> arr;

	for(int i = 0; i < Data.Len(); i++) {
		arr.Add(Data[i]);
	}
	
	//StringToBytes(Data, arr.GetData(), arr.Num()); BROKEN

	return arr;
}

FString UBPFL_RainAPI::Rain_BytesToString(TArray<uint8> Data) {
	//return BytesToString(Data.GetData(), Data.Num()); BROKEN
	FString out;
	
	for(int i = 0; i < Data.Num(); i++) {
		out.AppendChar(Data[i]);
	}
	return out;
}

TArray<uint8> UBPFL_RainAPI::Rain_AesEncryptBytes(TArray<uint8> Input, TArray<uint8> Key) {
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before padding: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	//pad
	FBufferArchive padded = FBufferArchive();
	padded << Input;

	padded.AddZeroed(AES_BLOCK_SIZE-(padded.Num() % AES_BLOCK_SIZE));

	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before encryption: %s"), *BytesToHex(padded.GetData(), padded.Num()));

	if(Key.Num() < 32) Key.AddZeroed(32-Key.Num());
	
	FAES::EncryptData(padded.GetData(), padded.Num(), Key.GetData(), Key.Num());
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After encryption: %s"), *BytesToHex(padded.GetData(), padded.Num()));
	return padded;
}

TArray<uint8> UBPFL_RainAPI::Rain_AesDecryptBytes(TArray<uint8> Input, TArray<uint8> Key) {
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before decryption: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	if(Key.Num() < 32) Key.AddZeroed(32-Key.Num());
	FAES::DecryptData(Input.GetData(), Input.Num(), Key.GetData(), Key.Num());
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After decryption: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	
	FMemoryReader padded = FMemoryReader(Input);
	TArray<uint8> raw;
	padded << raw;
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After unpadding: %s"), *BytesToHex(raw.GetData(), raw.Num()));
	return raw;
}

TArray<uint8> UBPFL_RainAPI::Rain_ZipAndEncryptBytes(TArray<uint8> Input, TArray<uint8> Key) {
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before zipping: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	//pad
	TArray<uint8> crypted;
	FArchiveSaveCompressedProxy padded(crypted,NAME_Zlib);
	
	padded << Input;

	padded.Flush();
	padded.Close();

	crypted.AddZeroed(AES_BLOCK_SIZE-(crypted.Num() % AES_BLOCK_SIZE));

	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before encryption: %s"), *BytesToHex(crypted.GetData(), crypted.Num()));
	if(Key.Num() < 32) Key.AddZeroed(32-Key.Num());
	FAES::EncryptData(crypted.GetData(), crypted.Num(), Key.GetData(), Key.Num());
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After encryption: %s"), *BytesToHex(crypted.GetData(), crypted.Num()));
	return crypted;
}

TArray<uint8> UBPFL_RainAPI::Rain_UnzipAndDecryptBytes(TArray<uint8> Input, TArray<uint8> Key) {
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] Before decryption: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	if(Key.Num() < 32) Key.AddZeroed(32-Key.Num());
	FAES::DecryptData(Input.GetData(), Input.Num(), Key.GetData(), Key.Num());
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After decryption: %s"), *BytesToHex(Input.GetData(), Input.Num()));
	
	FArchiveLoadCompressedProxy padded = FArchiveLoadCompressedProxy(Input,NAME_Zlib);
	TArray<uint8> raw;
	padded << raw;

	padded.Flush();
	padded.Close();
	UE_LOG(LogTemp, Warning, TEXT("[RainEncryption] After unzipping: %s"), *BytesToHex(raw.GetData(), raw.Num()));
	return raw;
}

FString UBPFL_RainAPI::Rain_HashBytes(TArray<uint8> Data) {
	UE_LOG(LogTemp, Warning, TEXT("[RainHash] Hashing %d bytes %s."), Data.Num(),*BytesToHex(Data.GetData(), Data.Num()));
	return FString(sha512(std::string((char*)Data.GetData(), Data.Num())).c_str());
}

void UBPFL_RainAPI::Rain_PrintHexBytes(TArray<uint8> Data) {
	UE_LOG(LogTemp, Warning, TEXT("[PrintHexBytes] %s"), *BytesToHex(Data.GetData(), Data.Num()));
}

int32 UBPFL_RainAPI::Rain_GetCurrentServerPort(UObject* WorldContextObject) {
	if(WorldContextObject) {
		if(WorldContextObject->GetWorld()) {
			return WorldContextObject->GetWorld()->URL.Port;
		}
	}
	return -1;
}

FString UBPFL_RainAPI::Rain_GetPlayerAddress(APlayerController* PlayerController, bool bWithPort) {
	if(PlayerController) {
		auto connection = PlayerController->GetNetConnection();
		if(connection) return connection->LowLevelGetRemoteAddress(bWithPort);
	}
	return FString();
}

FString UBPFL_RainAPI::Rain_GetParameterFromOptions(FString Parameter, TArray<FString> Options) {
	for(auto&elem:Options) {
		FString left,right;
		if(elem.Split("=",&left,&right)) {
			if(left.Equals(Parameter, ESearchCase::IgnoreCase)) {
				return right;
			}
		}
	}
	return FString();
}

FString UBPFL_RainAPI::Rain_GameSecretKey_GetFromLauncher() {
	TArray<FString> Tokens, Switches;
	FCommandLine::Parse(FCommandLine::Get(),Tokens, Switches);
	return Rain_GetParameterFromOptions("RAIN_SECRET", Tokens);
}

int32 UBPFL_RainAPI::Rain_GameServerID_GetFromLauncher() {
	TArray<FString> Tokens, Switches;
	FCommandLine::Parse(FCommandLine::Get(),Tokens, Switches);
	return FCString::Atoi(*Rain_GetParameterFromOptions("RAIN_SERVER", Tokens));
}

void UBPFL_RainAPI::Rain_Matchmaking_JoinServerWithSession(UObject* WorldContextObject, FString Server, FString Options, FString Session) {
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*Server), true, FString::Printf(TEXT("?RAIN_SESSION=%s?%s"), *Session, *Options));
}


FString UBPFL_RainAPI::Rain_GameSession_GetFromLauncher() {
	TArray<FString> Tokens, Switches;
	FCommandLine::Parse(FCommandLine::Get(),Tokens, Switches);
	return Rain_GetParameterFromOptions("RAIN_SESSION", Tokens);
}


