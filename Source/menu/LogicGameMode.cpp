// Fill out your copyright notice in the Description page of Project Settings.


#include "LogicGameMode.h"


ALogicGameMode::ALogicGameMode() {
  // use our custom PlayerController class
  PlayerControllerClass = ALogicPlayerController::StaticClass();
  DefaultPawnClass = nullptr;
  SpectatorClass = nullptr;
  PlayerStateClass = nullptr;
}

void ALogicGameMode::PreLogin(const FString& Options,
                                  const FString& Address,
                                  const FUniqueNetIdRepl& UniqueId,
                                  FString& ErrorMessage) {
  if (GetNumPlayers() > 1) {
    ErrorMessage = "Server is full";
  }
}

void ALogicGameMode::PostLogin(APlayerController* NewPlayer) {
  ALogicPlayerController* c = Cast<ALogicPlayerController>(NewPlayer);
  c->PostLogin(GetNumPlayers());
}
