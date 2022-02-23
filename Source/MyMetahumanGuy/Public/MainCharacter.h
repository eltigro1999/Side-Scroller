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

	void CheckJump();
	void CheckCrouch();

	void StartSprint();
	void FinishSprint();

	void StartWalk();
	void FinishWalk();

	void Punch();
	void Kick();
	bool StandPunching;
	bool StandKicking;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float SprintSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category="Speed")
		float RunSpeed = 600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float WalkSpeed = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
		float StandSpeed = 0.0f;


	UPROPERTY()
		bool jumping;
	
	UPROPERTY()
		bool crouching;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
		FORCEINLINE UMainCharacterMovementComponent* GetMainCharacterMovementComponent() const;

	virtual void PostInitializeComponents() override;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category="Jump")
		int jumpCount;

private:
	void MoveForward(float Amount);
	UMainCharacterMovementComponent* MainCharacterMovementComponent;
	FTimerHandle PunchingTimer;
	FTimerHandle IdleJumpLandingTimer;
	void OnPunchingTimerEnd();
	float StartJumpVelocity_Y;
	void OnIdleJumpLandingStart();
	float BasicJumpZVelocity;

	void PunchKick(UAnimMontage*);
};
