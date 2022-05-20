// Fill out your copyright notice in the Description page of Project Settings.

#include "LogicPlayerController.h"

#include "Logic.h"
#include "FightInput.h"
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
  buttonsPressed = 0;
  buttonsReleased = 0;
}

void ALogicPlayerController::SetupInputComponent() {
  Super::SetupInputComponent();
  MYLOG(Display, "SetupInputComponent");

  InputComponent->BindAction("Right", IE_Pressed, this, &ALogicPlayerController::ButtonRightPressed);
  InputComponent->BindAction("Left", IE_Pressed, this, &ALogicPlayerController::ButtonLeftPressed);
  InputComponent->BindAction("Up", IE_Pressed, this, &ALogicPlayerController::ButtonUpPressed);
  InputComponent->BindAction("Down", IE_Pressed, this, &ALogicPlayerController::ButtonDownPressed);
  InputComponent->BindAction("Right", IE_Released, this, &ALogicPlayerController::ButtonRightReleased);
  InputComponent->BindAction("Left", IE_Released, this, &ALogicPlayerController::ButtonLeftReleased);
  InputComponent->BindAction("Up", IE_Released, this, &ALogicPlayerController::ButtonUpReleased);
  InputComponent->BindAction("Down", IE_Released, this, &ALogicPlayerController::ButtonDownReleased);
  InputComponent->BindAction("LP", IE_Pressed, this, &ALogicPlayerController::ButtonLP);
  InputComponent->BindAction("HP", IE_Pressed, this, &ALogicPlayerController::ButtonHP);
  InputComponent->BindAction("HK", IE_Pressed, this, &ALogicPlayerController::ButtonHK);
  InputComponent->BindAction("LK", IE_Pressed, this, &ALogicPlayerController::ButtonLK);

  EnableInput(this);
}

void ALogicPlayerController::ButtonRightPressed() {
  MYLOG(Warning, "ButtonRightPressed");
  buttonsPressed = AFightInput::encodeButton(Button::RIGHT, buttonsPressed);
  buttonsReleased = AFightInput::unsetButton(Button::RIGHT, buttonsReleased);
}

void ALogicPlayerController::ButtonLeftPressed() {
  MYLOG(Warning, "ButtonLeftPressed");
  buttonsPressed = AFightInput::encodeButton(Button::LEFT, buttonsPressed);
  buttonsReleased = AFightInput::unsetButton(Button::LEFT, buttonsReleased);
}

void ALogicPlayerController::ButtonUpPressed() {
  MYLOG(Warning, "ButtonUpPressed");
  buttonsPressed = AFightInput::encodeButton(Button::UP, buttonsPressed);
  buttonsReleased = AFightInput::unsetButton(Button::UP, buttonsReleased);
}

void ALogicPlayerController::ButtonDownPressed() {
  MYLOG(Warning, "ButtonDownPressed");
  buttonsPressed = AFightInput::encodeButton(Button::DOWN, buttonsPressed);
  buttonsReleased = AFightInput::unsetButton(Button::DOWN, buttonsReleased);
}

void ALogicPlayerController::ButtonRightReleased() {
  MYLOG(Warning, "ButtonRightReleased");
  buttonsReleased = AFightInput::encodeButton(Button::RIGHT, buttonsReleased);
  buttonsPressed = AFightInput::unsetButton(Button::RIGHT, buttonsPressed);
}

void ALogicPlayerController::ButtonLeftReleased() {
  MYLOG(Warning, "ButtonLeftReleased");
  buttonsReleased = AFightInput::encodeButton(Button::LEFT, buttonsReleased);
  buttonsPressed = AFightInput::unsetButton(Button::LEFT, buttonsPressed);
}

void ALogicPlayerController::ButtonUpReleased() {
  MYLOG(Warning, "ButtonUpReleased");
  buttonsReleased = AFightInput::encodeButton(Button::UP, buttonsReleased);
  buttonsPressed = AFightInput::unsetButton(Button::UP, buttonsPressed);
}

void ALogicPlayerController::ButtonDownReleased() {
  MYLOG(Warning, "ButtonDownReleased");
  buttonsReleased = AFightInput::encodeButton(Button::DOWN, buttonsReleased);
  buttonsPressed = AFightInput::unsetButton(Button::DOWN, buttonsPressed);
}

void ALogicPlayerController::ButtonLP() {
  MYLOG(Warning, "ButtonLP");
  buttonsPressed = AFightInput::encodeButton(Button::LP, buttonsPressed);
}

void ALogicPlayerController::ButtonLK() {
  MYLOG(Warning, "ButtonLK");
  buttonsPressed = AFightInput::encodeButton(Button::LK, buttonsPressed);
}

void ALogicPlayerController::ButtonHP() {
  MYLOG(Warning, "ButtonHP");
  buttonsPressed = AFightInput::encodeButton(Button::HP, buttonsPressed);
}

void ALogicPlayerController::ButtonHK() {
  MYLOG(Warning, "ButtonHK");
  buttonsPressed = AFightInput::encodeButton(Button::HK, buttonsPressed);
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

  default:
      MYLOG(Warning, "Something went wrong");
      opponentInput = l->p1Input;
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

  // MYLOG(Display, "Tick");

  if (!readiedUp) {
    ServerReadyUp();
    readiedUp = true;
  }

  int targetFrame = input->getCurrentFrame() + 1;
  input->buttons(buttonsPressed, buttonsReleased, targetFrame);
  ServerButtons(buttonsPressed, buttonsReleased, targetFrame);
  buttonsPressed = 0;
  buttonsReleased = 0;
}

void ALogicPlayerController::ServerButtons_Implementation(int8 _buttonsPressed, int8 _buttonsReleased, int targetFrame) {
  if (GetWorld()->IsNetMode(NM_ListenServer)) {
    MYLOG(Display, "ServerButtons");
    if (!input) {
      MYLOG(Warning, "ServerButtons: input is NULL");
    }
    else {
      input->ClientButtons(_buttonsPressed, _buttonsReleased, targetFrame);
    }
  }
}

int ALogicPlayerController::getPlayerNumber() {
  return playerNumber;
}
