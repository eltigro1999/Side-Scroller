// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UMainCharacterMovementComponent;
class UAnimMontage;
struct FTimerHandle;


UCLASS()
class MYMETAHUMANGUY_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
		UCameraComponent* CameraComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Spring Arm")
		USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montage")
		UAnimMontage* PunchAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Montage")
		UAnimMontage* KickAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
		bool Climbing=false;

	void CheckJump();
	void CheckCrouch();

	void StartSprint();
	void FinishSprint();

	void StartWalk();
	void FinishWalk();

	void Climb(float Amount);
	float HorizontalSpeed = 1.0f;

	void Punch();
	void Kick();
	bool StandPunching;
	bool StandKicking;
	void StartClimbing();

	float ClimbSlowly = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float SprintSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category="Speed")
		float RunSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float WalkSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float StandSpeed = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Climbing")
	FRotator RotationWhenClimbing;

	UPROPERTY()
		bool jumping;
	
	UPROPERTY()
		bool crouching;


	UFUNCTION(BlueprintCallable, Category = "Movement")
		FORCEINLINE class UMainCharacterMovementComponent* GetMainCharacterMovementComponent() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UMainCharacterMovementComponent* MainCharacterMovementComponent;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Jump")
		int jumpCount;

	virtual void PostInitializeComponents() override;

private:
	void MoveForward(float Amount);
	FTimerHandle PunchingTimer;
	FTimerHandle IdleJumpLandingTimer;
	void OnPunchingTimerEnd();
	float StartJumpVelocity_Y;
	void OnIdleJumpLandingStart();
	float BasicJumpZVelocity;

	void PunchKick(UAnimMontage*);

};
