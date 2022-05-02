// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "FightGameState.generated.h"

// DECLARE_DELEGATE(FOnPreRoundDelegate);
// DECLARE_DELEGATE(FOnBeginRoundDelegate);
// DECLARE_DELEGATE(FOnEndRoundDelegate);
// DECLARE_DELEGATE(FOnEndFightDelegate);

// It should be possible to bind to these events from blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBeginRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndRoundDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndFightDelegate);

UCLASS()
class MENU_API AFightGameState : public AGameState
{
  GENERATED_BODY()

private:
  bool p0Ready = false;
  bool p1Ready = false;

public:
  UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
  FOnPreRoundDelegate OnPreRound;
  UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
  FOnBeginRoundDelegate OnBeginRound;
  UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
  FOnEndRoundDelegate OnEndRound;
  UPROPERTY (BlueprintAssignable, Category="Fight Sequence")
  FOnEndFightDelegate OnEndFight;

  // LogicPlayerController will call this function when it begins
  // ticking. When both players are ticking and call this function,
  // FightGameState will call ClientPlayersReady() on all clients.
  void ServerPlayerReady(int playerNumber);

  // This function will call doPreRound().
  UFUNCTION (NetMulticast, Reliable)
  void ClientPlayersReady();

  // These functions trigger the corresponding events. They should
  // only be called on the client.
  void doPreRound();
  void doBeginRound();
  void doEndRound();
  void doEndFight();
};

static inline AFightGameState* GetFightGameState(UWorld* world) {
  return world->GetGameState<AFightGameState>();
}
