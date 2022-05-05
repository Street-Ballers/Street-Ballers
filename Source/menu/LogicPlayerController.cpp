// Fill out your copyright notice in the Description page of Project Settings.

#include "LogicPlayerController.h"

#include "Logic.h"
#include "FightInput.h"
#include "Fighter.h"
#include "FightCameraActor.h"
#include "FightGameState.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

#define MYLOG(category, message, ...) UE_LOG(LogTemp, category, TEXT("ALogicPlayerController (%i %s) " message), playerNumber, (GetWorld()->IsNetMode(NM_ListenServer)) ? TEXT("server") : TEXT("client"), ##__VA_ARGS__)

ALogicPlayerController::ALogicPlayerController()
{

}

void ALogicPlayerController::BeginPlay()
{
  Super::BeginPlay();
  MYLOG(Warning, "BeginPlay");
}

void ALogicPlayerController::ServerPostLogin_Implementation(int playerNumber_) {
  // NOTE: we run this even when running on the client because, when
  // starting a listenserver as PIE, it seems to create and connect
  // the first player before it actually breaks off a listen server.

  // Get FightInputs, figure out which one is ours and which is
  // opponent, and set ownership of ours
  // ULocalPlayer* localPlayer = GetLocalPlayer();
  // playerNumber = localPlayer ? localPlayer->GetLocalPlayerIndex() : playerNumber_;
  playerNumber = playerNumber_;
  MYLOG(Warning, "ServerPostLogin");
  //UE_LOG(LogTemp, Warning, "ALogicPlayerController ServerPostLogin: Logging in Player %i (%s)", playerNumber, GetLocalPlayer() ? "local player" : "networked player");

  ALogic *l = FindLogic(GetWorld());
  AFightInput* opponentInput;
  switch (playerNumber) {
  case 0:
    input = l->p1Input;
    opponentInput = l->p2Input;
    break;
  case 1:
    input = l->p2Input;
    opponentInput = l->p1Input;
    break;
  }

  opponentInput->SetOwner(this);
}

void ALogicPlayerController::ClientPostLogin_Implementation(int playerNumber_) {
  // Also get FightInputs, and figure out which one is ours. Also set
  // tick dependency for Logic.
  // ULocalPlayer* localPlayer = GetLocalPlayer();
  // playerNumber = localPlayer ? localPlayer->GetLocalPlayerIndex() : playerNumber_;
  playerNumber = playerNumber_;
  MYLOG(Display, "ClientPostLogin");

  ALogic *l = FindLogic(GetWorld());
  switch (playerNumber) {
  case 0:
    input = l->p1Input;
    break;
  case 1:
    input = l->p2Input;
    break;
  }

  l->AddTickPrerequisiteActor(this);
  readiedUp = false;
}

void ALogicPlayerController::ServerReadyUp_Implementation() {
  if (!GetFightGameState(GetWorld())) {
    MYLOG(Warning, "ServerReadyUp: FightGameState is NULL");
  }
  else {
    GetFightGameState(GetWorld())->ServerPlayerReady(playerNumber);
  }
}

// TODO: change this to PlayerTick when we add the Input component
void ALogicPlayerController::Tick(float deltaSeconds) {
  if (!IsLocalController())
    return;

  if (GetWorld()->IsPaused())
    return;

  MYLOG(Display, "Tick");

  if (!readiedUp) {
    ServerReadyUp();
    readiedUp = true;
  }

  // for now, just simulate player 1 pressing right repeatedly, and
  // player 2 pressing HP repeatedly
  if (input) {
    int targetFrame = input->getCurrentFrame()+1;
    if (playerNumber == 0) {
      int8 encodedButtons = AFightInput::encodeButton(Button::RIGHT);
      MYLOG(Display, "Tick: button %i", encodedButtons);
      input->buttons(encodedButtons, 0, targetFrame);
      ServerButtons(encodedButtons, 0, targetFrame);
    }
    else {
      int8 encodedButtons = AFightInput::encodeButton(Button::HP);
      MYLOG(Display, "Tick: button %i", encodedButtons);
      input->buttons(encodedButtons, 0, targetFrame);
      ServerButtons(encodedButtons, 0, targetFrame);
    }
  }
  else {
    MYLOG(Warning, "Tick: INPUT IS NULL!");
  }
}

void ALogicPlayerController::ServerButtons_Implementation(int8 buttonsPressed, int8 buttonsReleased, int targetFrame) {
  if (GetWorld()->IsNetMode(NM_ListenServer)) {
    MYLOG(Display, "ServerButtons");
    if (!input) {
      MYLOG(Warning, "ServerButtons: input is NULL");
    }
    else {
      input->ClientButtons(buttonsPressed, buttonsReleased, targetFrame);
    }
  }
}

int ALogicPlayerController::getPlayerNumber() {
  return playerNumber;
}
