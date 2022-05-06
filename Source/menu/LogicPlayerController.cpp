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
  buttonsPressed = 0;
  buttonsReleased = 0;
}

void ALogicPlayerController::SetupInputComponent() {
    Super::SetupInputComponent();

    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Input Setting up!!"));

    InputComponent->BindAxis("Right", this, &ALogicPlayerController::RightInput);
    InputComponent->BindAxis("Left", this, &ALogicPlayerController::LeftInput);

    InputComponent->BindAction("LP", IE_Pressed, this, &ALogicPlayerController::LP);
    InputComponent->BindAction("HP", IE_Pressed, this, &ALogicPlayerController::HP);
    InputComponent->BindAction("HK", IE_Pressed, this, &ALogicPlayerController::HK);
    InputComponent->BindAction("LK", IE_Pressed, this, &ALogicPlayerController::LK);

    EnableInput(this);

}

void ALogicPlayerController::RightInput(float value) {

    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::RIGHT);

    // MYLOG(Display, "Right Input: button %i", encodedButtons);

    if (value >= 0.25) {
        UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Right was pressed!!"));
        UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Value is %f!!"), value);
        buttonsPressed = AFightInput::encodeButton(Button::RIGHT, buttonsPressed);
        // input->buttons(encodedButtons, 0, targetFrame);
        // ServerButtons(encodedButtons, 0, targetFrame);
    }
    else{
        // input->buttons(0, encodedButtons, targetFrame);
        // ServerButtons(0, encodedButtons, targetFrame);
    }
        
}

void ALogicPlayerController::LeftInput(float value) {

    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::LEFT);

    if (value <= -0.25) {
        UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Left was pressed!!"));
        UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: Value is %f!!"), value);
        buttonsPressed = AFightInput::encodeButton(Button::LEFT, buttonsPressed);
        // input->buttons(encodedButtons, 0, targetFrame);
        // ServerButtons(encodedButtons, 0, targetFrame);
    }
    else {
        // input->buttons(0, encodedButtons, targetFrame);
        // ServerButtons(0, encodedButtons, targetFrame);
    }
}

void ALogicPlayerController::LP() {
    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::LP);

    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: LP was pressed"));

    buttonsPressed = AFightInput::encodeButton(Button::LP, buttonsPressed);
    // input->buttons(encodedButtons, 0, targetFrame);
    // ServerButtons(encodedButtons, 0, targetFrame);
}

void ALogicPlayerController::LK() {
    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::LK);

    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: LK was pressed"));

    buttonsPressed = AFightInput::encodeButton(Button::LK, buttonsPressed);
    // input->buttons(encodedButtons, 0, targetFrame);
    // ServerButtons(encodedButtons, 0, targetFrame);
}

void ALogicPlayerController::HP() {
    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::HP);

    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: HP was pressed"));

    buttonsPressed = AFightInput::encodeButton(Button::HP, buttonsPressed);
    // input->buttons(encodedButtons, 0, targetFrame);
    // ServerButtons(encodedButtons, 0, targetFrame);
}

void ALogicPlayerController::HK() {
    // int targetFrame = input->getCurrentFrame() + 1;
    // int8 encodedButtons = AFightInput::encodeButton(Button::HK);

    UE_LOG(LogTemp, Warning, TEXT("ALogicPlayerController: HK was pressed"));

    buttonsPressed = AFightInput::encodeButton(Button::HK, buttonsPressed);
    // input->buttons(encodedButtons, 0, targetFrame);
    // ServerButtons(encodedButtons, 0, targetFrame);
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

  MYLOG(Display, "Tick");

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
