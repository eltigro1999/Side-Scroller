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

	StartJumpVelocity_Y = 0.0f;
	BasicJumpZVelocity = GetCharacterMovement()->JumpZVelocity;

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

	if (jumping) {
		StartJumpVelocity_Y = GetVelocity().Y;
		Jump();
	};

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
	GEngine->AddOnScreenDebugMessage(0, 2, FColor::Green, FString("Landed"));
	GEngine->AddOnScreenDebugMessage(1, 2, FColor::Green, FString::SanitizeFloat(GetVelocity().Y));
	if (FMath::IsNearlyZero(StartJumpVelocity_Y) && jumpCount < 2) {
		GetCharacterMovement()->MaxWalkSpeed = IdleJunpLandingSpeed;
		GetCharacterMovement()->JumpZVelocity = 0.0f;
		GetWorld()->GetTimerManager().SetTimer(IdleJumpLandingTimer,this, &AMainCharacter::OnIdleJumpLandingStart, 0.82f, false );
	}
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
		GEngine->AddOnScreenDebugMessage(0, 10, FColor::Green, FString("Character starts jumping")+FString::SanitizeFloat(jumpCount));
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
	if (PunchAnimMontage && !crouching && !GetCharacterMovement()->IsFalling() && FMath::Abs(GetVelocity().Y)<10.0f) {
		GetCharacterMovement()->MaxWalkSpeed = StandingPunchSpeed;
		PlayAnimMontage(PunchAnimMontage,1, NAME_None);
		GetWorld()->GetTimerManager().SetTimer(PunchingTimer, this, &AMainCharacter::OnPunchingTimerEnd, PunchAnimMontage->CalculateSequenceLength(), false);
	}
}

void AMainCharacter::OnPunchingTimerEnd() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetWorld()->GetTimerManager().ClearTimer(PunchingTimer);
}

void AMainCharacter::OnIdleJumpLandingStart() {
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	GetCharacterMovement()->JumpZVelocity = BasicJumpZVelocity;
	GetWorld()->GetTimerManager().ClearTimer(IdleJumpLandingTimer);
}