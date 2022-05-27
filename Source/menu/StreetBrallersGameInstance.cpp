// Fill out your copyright notice in the Description page of Project Settings.


#include "StreetBrallersGameInstance.h"

void UStreetBrallersGameInstance::OnStart() {
  p1Char = 0;
  p2Char = 1;
  isOnline = false;
}

void UStreetBrallersGameInstance::LoadComplete(const float LoadTime, const FString& MapName) {
  Super::LoadComplete(LoadTime, MapName);
  CreateMessagesWidget();
}
