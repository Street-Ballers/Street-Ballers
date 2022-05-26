// Fill out your copyright notice in the Description page of Project Settings.

#include "FightGameState.h"
#include "Logic.h"
#include "StreetBrallersGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("FightGameState (%s) " message), (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

void AFightGameState::PostInitializeComponents()
{
  Super::PostInitializeComponents();

  check(UGameplayStatics::GetGameInstance(GetWorld()) != nullptr);
  UStreetBrallersGameInstance* gi = Cast<UStreetBrallersGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
  check(gi != nullptr);
  if ((!gi->isOnline) || GetWorld()->IsNetMode(NM_ListenServer)) {
    p1Char = gi->p1Char;
    p2Char = gi->p2Char;
  }
}

void AFightGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AFightGameState, p1Char);
  DOREPLIFETIME(AFightGameState, p2Char);
}

void AFightGameState::ServerPlayerReady(int playerNumber) {
  MYLOG(Warning, "ServerPlayerReady");
  switch (playerNumber) {
  case 0: p0Ready = true; break;
  case 1: p1Ready = true; break;
  }

  if (p0Ready && p1Ready) {
    ALogic* l = FindLogic(GetWorld());
    if ((GetWorld()->IsNetMode(NM_ListenServer)))
      l->ClientPlayersReady();
    l->preRound();
  }
}
