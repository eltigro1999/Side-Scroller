// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterController.h"
#include "MainCharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


// Sets default values
AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = true;

	//GetCharacterMovement()->DefaultLandMovementMode(MOVE_Flying);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetRelativeRotation(FRotator(0, -90,0));
	//SpringArmComponent->bAbsoluteRotation = true;// Character rotation doesn't affect rotation of spring arm component // PRIVATE MEMBER. So it doesn't work
	SpringArmComponent->bUsePawnControlRotation = true; //If you look at the same thing in the Third Person template project,
														//you'll see that you can never use the right mouse movement to rotate
														//the camera around the pawn if Use Pawn Control Rotation is enabled.
	SpringArmComponent->SetUsingAbsoluteRotation(true);


	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->AirControl = 0.6f;
	GetCharacterMovement()->GroundFriction = 0.5f;

	jumping = false;

	crouching = false;

	jumpCount = 0;

	StartJumpVelocity_Y = 0.0f;
	BasicJumpZVelocity = GetCharacterMovement()->JumpZVelocity;
	StandKicking = false; 
	StandPunching = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();	
	GetMainCharacterMovementComponent()->MaxFlySpeed = 300;
	GetMainCharacterMovementComponent()->BrakingDecelerationFlying = 2000;	//Stop flying immediately after releas up/down key

}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (jumping) {
		StartJumpVelocity_Y = GetVelocity().Y;
		Jump();
	};

	if (crouching) GetCharacterMovement()->Crouch();
	else GetCharacterMovement()->UnCrouch();

	//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation()+ GetActorRotation().Vector()*(-75), FColor::Black, false, 2.0f, 0, 20);


}

void AMainCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	MainCharacterMovementComponent = Cast<UMainCharacterMovementComponent>(GetMovementComponent());
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Climb", this, &AMainCharacter::Climb);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AMainCharacter::CheckJump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AMainCharacter::CheckJump);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AMainCharacter::CheckCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &AMainCharacter::CheckCrouch);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AMainCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AMainCharacter::FinishSprint);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, this, &AMainCharacter::StartWalk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, this, &AMainCharacter::FinishWalk);
	PlayerInputComponent->BindAction("Punch", EInputEvent::IE_Pressed, this, &AMainCharacter::Punch);
	PlayerInputComponent->BindAction("Kick", EInputEvent::IE_Pressed, this,  &AMainCharacter::Kick);
	PlayerInputComponent->BindAction("Climbing", EInputEvent::IE_Pressed, this, &AMainCharacter::StartClimbing);
}

void AMainCharacter::Landed(const FHitResult& Hit) {
	Super::Landed(Hit);
	GEngine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString("Landed"));
	GEngine->AddOnScreenDebugMessage(1, 2, FColor::Green, FString::SanitizeFloat(GetVelocity().Y));
	if (FMath::IsNearlyZero(StartJumpVelocity_Y) && jumpCount < 2) {
		GetCharacterMovement()->MaxWalkSpeed = StandSpeed;
		GetCharacterMovement()->JumpZVelocity = 0.0f;
		GetWorld()->GetTimerManager().SetTimer(IdleJumpLandingTimer,this, &AMainCharacter::OnIdleJumpLandingStart, 0.82f, false );
	}
	jumpCount = 0;
}

void AMainCharacter::MoveForward(float Amount) {
	AddMovementInput(FVector(0.0f, HorizontalSpeed, 0.0f), Amount);

}

void AMainCharacter::CheckJump() {
	if (jumping) {
		jumping = false;
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Green, FString("Character Finishes Jumping"));
	}
	else {
		if (StandKicking || StandPunching) return;
		jumping = true;
		jumpCount++;
		if (jumpCount == 2) {
			GEngine->AddOnScreenDebugMessage(0, 10, FColor::Green, FString("JumpCount") + FString::SanitizeFloat(jumpCount));
			LaunchCharacter(FVector(0,0,700), false, true);
		}
	}
}

void AMainCharacter::CheckCrouch() {
	crouching = !crouching;
	GetCharacterMovement()->bWantsToCrouch = crouching;
}

void AMainCharacter::StartSprint() {
	if (!StandKicking && !StandPunching &&
		!(jumping && FMath::IsNearlyZero(StartJumpVelocity_Y)))
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AMainCharacter::FinishSprint() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AMainCharacter::StartWalk() {
	if (!StandKicking && !StandPunching &&
		!(jumping && FMath::IsNearlyZero(StartJumpVelocity_Y)))
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMainCharacter::FinishWalk() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

}

UMainCharacterMovementComponent* AMainCharacter::GetMainCharacterMovementComponent() const {
	return MainCharacterMovementComponent;
}

void AMainCharacter::Punch() {
	PunchKick(PunchAnimMontage);
}

void AMainCharacter::Kick() {
	PunchKick(KickAnimMontage);
}

void AMainCharacter::PunchKick(UAnimMontage* AnimMontage) {
	if (AnimMontage && !crouching && !GetCharacterMovement()->IsFalling()
		&& FMath::IsNearlyZero(GetVelocity().Y) && GetWorld()) {
		StandPunching= StandKicking = true;
		GetCharacterMovement()->MaxWalkSpeed = StandSpeed;
		PlayAnimMontage(AnimMontage, 1, NAME_None);
		GetWorld()->GetTimerManager().SetTimer(PunchingTimer, this, &AMainCharacter::OnPunchingTimerEnd, AnimMontage->CalculateSequenceLength(), false);
	}
}

void AMainCharacter::OnPunchingTimerEnd() {
	GetWorld()->GetTimerManager().ClearTimer(PunchingTimer);
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	StandPunching = StandKicking = false;
}

void AMainCharacter::OnIdleJumpLandingStart() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->JumpZVelocity = BasicJumpZVelocity;
	GetWorld()->GetTimerManager().ClearTimer(IdleJumpLandingTimer);
}

void AMainCharacter::StartClimbing() {
	if (!Climbing) {
		FVector Start = GetActorLocation() + FVector(0, 0, 100);
		FRotator Rotation = GetActorRotation();
		FHitResult Hit;
		//GetController()->GetPlayerViewPoint(Start, Rotation);
		FVector End = Start + (Rotation.Vector() * 40.0f);
		FCollisionQueryParams TraceParams;
		GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);
		//DrawDebugLine(GetWorld(), Start, End, FColor::Black, false, 2.0f, 0, 10);
		Climbing = Hit.bBlockingHit;
		if (Climbing) {
			RotationWhenClimbing = GetActorRotation();
			HorizontalSpeed = 0.0f;
			GetMainCharacterMovementComponent()->SetMovementMode(EMovementMode::MOVE_Flying);
		}
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::Red, FString(Climbing ? "Climbing" : "Not climbing"));
	} else {
		Climbing = false;
		GetMainCharacterMovementComponent()->SetMovementMode(EMovementMode::MOVE_Walking);
		HorizontalSpeed = 1.0f;
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::Red, FString(Climbing ? "Climbing" : "Not climbing"));
	}
}

void AMainCharacter::Climb(float Amount) {
	if (Climbing) {
		AddMovementInput(FVector(0.0f, 0.0f, ClimbSlowly), Amount);
		GEngine->AddOnScreenDebugMessage(1, 3, FColor::Green, FString("It's happening"));
		if (Amount) {
			SetActorRotation(RotationWhenClimbing);
		}
	}
}