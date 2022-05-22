// Fill out your copyright notice in the Description page of Project Settings.


#include "FightCameraActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include <algorithm>
#include <cmath>

AFightCameraActor::AFightCameraActor() {
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void AFightCameraActor::BeginPlay() {
  Super::BeginPlay();

  RegisterAllActorTickFunctions(true, false);

  // set the camera for the local player to this camera
  APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
  if (pc) {
    pc->SetViewTarget(this);
  }

  scale = 1.2;
  min = 750.0;
  height = 200.0;
  boundAdjust = 300.0;
}

void AFightCameraActor::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  FVector pos1 = fighter1->GetActorLocation();
  FVector pos2 = fighter2->GetActorLocation();

  FVector pos = (pos1+pos2) * 0.5;

  pos.Y = std::max(pos.Y, stageBoundLeft + boundAdjust);
  pos.Y = std::min(pos.Y, stageBoundRight - boundAdjust);

  pos.X = std::min(-1 * scale * std::abs(pos1.Y - pos2.Y),
                   -1 * min);

  // Adjust camera to mid level of fighters
  pos.Z += 100.0;
  pos.Z = std::max(pos.Z, height);

  SetActorLocation(pos, false, nullptr, ETeleportType::None);
}
