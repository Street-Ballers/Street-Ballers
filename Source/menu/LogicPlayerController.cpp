// Fill out your copyright notice in the Description page of Project Settings.


#include "LogicPlayerController.h"

#include "LogicPlayerCameraManager.h"
#include "Logic.h"
#include "Input.h"

ALogicPlayerController::ALogicPlayerController()
{
  PlayerCameraManagerClass = ALogicPlayerCameraManager::StaticClass();
}

void ALogicPlayerController::PostLogin(int playerNumber_) {
  playerNumber = playerNumber_;

  ALogic *l = FindLogic(GetWorld());
  switch (playerNumber) {
  case 1: input = &(l->p1Input); break;
  case 2: input = &(l->p2Input); break;
  }

  l->AddTickPrerequisiteActor(this);
}

void ALogicPlayerController::Tick(float deltaSeconds) {
  // for now, just simulate player 1 walking forward, and player 2
  // standing still
  if (playerNumber == 1)
    input->buttonsPressed({Button::FORWARD});
  else
    input->buttonsPressed({});
}
