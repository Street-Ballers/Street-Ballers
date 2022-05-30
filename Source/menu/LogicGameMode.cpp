// Fill out your copyright notice in the Description page of Project Settings.

#include "LogicGameMode.h"
#include "Logic.h"
#include "Action.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DefaultValueHelper.h"

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("ALogicGameMode (%s) " message), (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

ALogicGameMode::ALogicGameMode() {
  // use our custom PlayerController class
  PlayerControllerClass = ALogicPlayerController::StaticClass();
  DefaultPawnClass = nullptr;
  SpectatorClass = nullptr;
  GameStateClass = AFightGameState::StaticClass();
  // setting this to null just causes it to fall back to APlayerState
  // PlayerStateClass = nullptr;
}

void ALogicGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
  Super::InitGame(MapName, Options, ErrorMessage);
  playerCount = 0;
}

void ALogicGameMode::PreLogin(const FString& Options,
                              const FString& Address,
                              const FUniqueNetIdRepl& UniqueId,
                              FString& ErrorMessage) {
  if (playerCount > 1) {
    ErrorMessage = "Server is full";
    MYLOG(Warning, "SERVER IS FULL");
  }
  else {
    MYLOG(Display, "PRELOGIN SUCEEDED!");
    FString character = UGameplayStatics::ParseOption(Options, FString("char"));
    if (!character.IsEmpty()) {
      int32 c = 0;
      FDefaultValueHelper::ParseInt(character, c);
      AFightGameState* gs = Cast<AFightGameState>(UGameplayStatics::GetGameState(GetWorld()));
      gs->p2Char = c;
    }
  }
}

void ALogicGameMode::PostLogin(APlayerController* NewPlayer) {
  int playerNumber = playerCount++;
  MYLOG(Display, "PostLogin: player %i", playerNumber);
  if (GetWorld()->IsNetMode(NM_ListenServer)) {
    MYLOG(Display, "PostLogin: is on server");
  }
  else {
    MYLOG(Warning, "PostLogin: is on client");
  }
  ALogicPlayerController* c = Cast<ALogicPlayerController>(NewPlayer);
  c->ServerPostLogin(playerNumber);
  c->ClientPostLogin(playerNumber);
}
