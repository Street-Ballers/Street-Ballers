// Fill out your copyright notice in the Description page of Project Settings.


#include "LogicPlayerController.h"

#include "Logic.h"
#include "Input.h"
#include "Fighter.h"
#include "FightCameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

ALogicPlayerController::ALogicPlayerController()
{

}

void ALogicPlayerController::BeginPlay()
{
  UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: BeginPlay"));
}

void ALogicPlayerController::PostLogin(int playerNumber_) {
  // this will probably work for networked multiplayer, but it does
  // not work for local multiplayer
  // playerNumber = playerNumber_;

  // this works for local multiplayer only
  ULocalPlayer* localPlayer = GetLocalPlayer();
  playerNumber = localPlayer ? localPlayer->GetLocalPlayerIndex() : playerNumber_;

  ALogic *l = FindLogic(GetWorld());
  switch (playerNumber) {
  case 0: input = &(l->p1Input); break;
  case 1: input = &(l->p2Input); break;
  }

  UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Logging in Player %i (%s)"), playerNumber, localPlayer ? TEXT("local player") : TEXT("networked player"));

  l->AddTickPrerequisiteActor(this);
}

void ALogicPlayerController::Tick(float deltaSeconds) {
  // for now, just simulate player 1 walking forward, and player 2
  // standing still
  if (input) {
    if (playerNumber == 0) {
      input->buttonsPressed({Button::RIGHT});
      UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: player 0 tick"));
    }
    else {
      input->buttonsPressed({});
      UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: player 1 tick"));
    }
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: INPUT IS NULL!"));
  }
}
