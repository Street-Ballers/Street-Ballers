// Fill out your copyright notice in the Description page of Project Settings.


#include "Fighter.h"
#include "Logic.h"

// Sets default values
AFighter::AFighter()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFighter::BeginPlay()
{
  Super::BeginPlay();
  l = FindLogic(GetWorld());
  AddTickPrerequisiteActor(l);
}

// Called every frame
void AFighter::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  const Player& p = isPlayer1 ? l->getPlayer1() : l->getPlayer2();
  SetActorLocation(p.pos, false, nullptr, ETeleportType::None);
}
