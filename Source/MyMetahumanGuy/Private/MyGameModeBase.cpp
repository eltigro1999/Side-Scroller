// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "MainCharacter.h"
#include "MainCharacterController.h"

AMyGameModeBase::AMyGameModeBase() {
	DefaultPawnClass = AMainCharacter::StaticClass();
	PlayerControllerClass = AMainCharacterController::StaticClass();
}