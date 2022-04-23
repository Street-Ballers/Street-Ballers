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

}

void ALogicPlayerController::PostLogin(int playerNumber_) {
  playerNumber = playerNumber_;

  ALogic *l = FindLogic(GetWorld());
  switch (playerNumber) {
  case 0: input = &(l->p1Input); break;
  case 1: input = &(l->p2Input); break;
  }

  l->AddTickPrerequisiteActor(this);
}

void ALogicPlayerController::Tick(float deltaSeconds) {
  // for now, just simulate player 1 walking forward, and player 2
  // standing still
  if (input) {
    if (playerNumber == 0)
      input->buttonsPressed({Button::FORWARD});
    else
      input->buttonsPressed({});
  }
  else {
    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: INPUT IS NULL!"));
  }
}
