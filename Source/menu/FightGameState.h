// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FightGameState.generated.h"

UCLASS()
class MENU_API AFightGameState : public AGameState
{
  GENERATED_BODY()

private:
  bool p0Ready = false;
  bool p1Ready = false;

public:
  // LogicPlayerController will call this function when it begins
  // ticking. When both players are ticking and call this function,
  // FightGameState will call ClientPlayersReady() on all clients.
  void ServerPlayerReady(int playerNumber);

  // This function will call ALogic::preRound().
  UFUNCTION (NetMulticast, Reliable)
  void ClientPlayersReady();
};

static inline AFightGameState* GetFightGameState(UWorld* world) {
  return world->GetGameState<AFightGameState>();
}
