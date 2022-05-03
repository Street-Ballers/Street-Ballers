// Fill out your copyright notice in the Description page of Project Settings.

#include "FightGameState.h"
#include "Logic.h"

void AFightGameState::ServerPlayerReady(int playerNumber) {
  switch (playerNumber) {
  case 0: p0Ready = true; break;
  case 1: p1Ready = true; break;
  }

  if (p0Ready && p1Ready) {
    ClientPlayersReady();
  }
}

void AFightGameState::ClientPlayersReady_Implementation() {
  ALogic* l = FindLogic(GetWorld());
  l->preRound();
}
