// Fill out your copyright notice in the Description page of Project Settings.


#include "LogicGameMode.h"
#include "Logic.h"
#include "Action.h"

ALogicGameMode::ALogicGameMode() {
  // use our custom PlayerController class
  PlayerControllerClass = ALogicPlayerController::StaticClass();
  DefaultPawnClass = nullptr;
  SpectatorClass = nullptr;
  // setting this to null just causes it to fall back to APlayerState
  // PlayerStateClass = nullptr;
}

void ALogicGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
  Super::InitGame(MapName, Options, ErrorMessage);
  HAction::init();
  HCharacter::init();
  check(HAction(IActionIdle).character() == HChar1);
  check(HCharacter(IChar1).idle() == HActionIdle);
}

void ALogicGameMode::PreLogin(const FString& Options,
                              const FString& Address,
                              const FUniqueNetIdRepl& UniqueId,
                              FString& ErrorMessage) {
  if (GetNumPlayers() > 1) {
    ErrorMessage = "Server is full";
    UE_LOG(LogTemp, Warning, TEXT("ALogicGameMode: SERVER IS FULL"));
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("ALogicGameMode: PRELOGIN SUCEEDED!"));
  }
}

void ALogicGameMode::PostLogin(APlayerController* NewPlayer) {
  UE_LOG(LogTemp, Warning, TEXT("ALogicGameMode: POSTLOGIN"));
  ALogicPlayerController* c = Cast<ALogicPlayerController>(NewPlayer);
  c->PostLogin(1);
  ALogic* l = FindLogic(GetWorld());
  l->beginFight();
}
