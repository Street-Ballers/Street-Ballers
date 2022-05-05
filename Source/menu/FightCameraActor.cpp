// Fill out your copyright notice in the Description page of Project Settings.


#include "FightCameraActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include <algorithm>

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
}

void AFightCameraActor::SetFighters(AFighter* fighter1_, AFighter* fighter2_) {
  fighter1 = fighter1_;
  fighter2 = fighter2_;
}

void AFightCameraActor::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  FVector pos1 = fighter1->GetActorLocation();
  FVector pos2 = fighter2->GetActorLocation();

  FVector pos = (pos1+pos2) * 0.5;

  // 1.732 = cot(30 degrees). 30 deg is half our FOV.
  pos.X = std::min(-1.732 * FVector::Distance(pos1, pos2),
                   // don't move closer than this
                   -500.0);

  // Adjust camera to mid level of fighters
  pos.Z += 100.0;

  SetActorLocation(pos, false, nullptr, ETeleportType::None);
}
