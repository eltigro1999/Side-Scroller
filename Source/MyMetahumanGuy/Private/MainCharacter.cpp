// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterController.h"
#include "MainCharacterMovementComponent.h"
#include "Animation/AnimMontage.h"


// Sets default values
AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetRelativeRotation(FRotator(0, -90,0));
	//SpringArmComponent->bAbsoluteRotation = true;// Character rotation doesn't affect rotation of spring arm component // PRIVATE MEMBER. So it doesn't work
	SpringArmComponent->bUsePawnControlRotation = true; //If you look at the same thing in the Third Person template project,
														//you'll see that you can never use the right mouse movement to rotate
														//the camera around the pawn if Use Pawn Control Rotation is enabled.

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);


	jumping = false;

	crouching = false;

	jumpCount = 0;

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (jumping) Jump();

	if (crouching) GetCharacterMovement()->Crouch();
	else GetCharacterMovement()->UnCrouch();
	
	

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AMainCharacter::CheckJump);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &AMainCharacter::CheckJump);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &AMainCharacter::CheckCrouch);
	PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Released, this, &AMainCharacter::CheckCrouch);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AMainCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AMainCharacter::FinishSprint);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Pressed, this, &AMainCharacter::StartWalk);
	PlayerInputComponent->BindAction("Walk", EInputEvent::IE_Released, this, &AMainCharacter::FinishWalk);
	PlayerInputComponent->BindAction("Punch", EInputEvent::IE_Pressed, this, &AMainCharacter::Punch);
}

void AMainCharacter::Landed(const FHitResult& Hit) {
	Super::Landed(Hit);

	jumpCount = 0;
}

void AMainCharacter::MoveForward(float Amount) {
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Amount);

}

void AMainCharacter::CheckJump() {
	if (jumping) {
		jumping = false;
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Green, FString("Character Finishes Jumping"));
	}
	else {
		jumping = true;
		jumpCount++;
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Green, FString("JumpCount=")+FString::SanitizeFloat(jumpCount));
		if (jumpCount == 2) {
			LaunchCharacter(FVector(0,0,700), false, true);
		}
	}
}

void AMainCharacter::CheckCrouch() {
	crouching = !crouching;
	GetCharacterMovement()->bWantsToCrouch = crouching;
}

void AMainCharacter::StartSprint() {
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	GEngine->AddOnScreenDebugMessage(0, 3, FColor::Green, FString("Sprint"));
}

void AMainCharacter::FinishSprint() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void AMainCharacter::StartWalk() {
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMainCharacter::FinishWalk() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

}

UMainCharacterMovementComponent* AMainCharacter::GetMainCharacterMovementComponent() const {
	return MainCharacterMovementComponent;
}

void AMainCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();

	MainCharacterMovementComponent = Cast<UMainCharacterMovementComponent>(GetMovementComponent());
}

void AMainCharacter::Punch() {
	if (PunchAnimMontage && !crouching && !jumping) {
		GEngine->AddOnScreenDebugMessage(0, 3, FColor::Green, FString("Montage started"));
		PlayAnimMontage(PunchAnimMontage,1, NAME_None);
		GEngine->AddOnScreenDebugMessage(0, 3, FColor::Green, FString("Montage done"));
	}
}