

#pragma once

//#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DECharacterMovementComponent.generated.h"
/**
 * 
 */

CHARACTERS_API DECLARE_LOG_CATEGORY_EXTERN(LogMovement, Log, All);

class CHARACTERS_API FDECharacterNetworkMoveData : public FCharacterNetworkMoveData
{
	
	typedef FCharacterNetworkMoveData Super;

public:
	// Additional data for character movement added here
	uint8 bWantsToSprint : 1;
	uint8 bWantsToRun : 1;

	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;
	virtual bool Serialize(UCharacterMovementComponent& Movement, FArchive& Ar, UPackageMap* PackageMap, 
		ENetworkMoveType MoveType) override;

};

class CHARACTERS_API FDECharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	TStaticArray<FDECharacterNetworkMoveData, 3> MoveData;	// NewMoveData, PendingMoveData, OldMoveData
	FDECharacterNetworkMoveDataContainer();
};

class CHARACTERS_API FDESavedMove : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;
public:
	uint8 bWantsToSprint : 1;
	uint8 bWantsToRun : 1;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, const FVector& NewAccel, 
		FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;
	virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC,
		const FVector& OldStartLocation) override;
};

class CHARACTERS_API FDENetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;
public:
	FDENetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);
	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS(ClassGroup = Movement, BlueprintType)
class CHARACTERS_API UDECharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend FDESavedMove;

public:
	UPROPERTY(Category = "Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToSprint : 1;
	UPROPERTY(Category = "Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToRun : 1;
	
	UDECharacterMovementComponent();
	~UDECharacterMovementComponent();

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysNavWalking(float DeltaTime, int32 Iterations) override;
	virtual void PhysWalking(float DeltaTime, int32 Iterations) override;

	virtual void MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, 
		FStepDownResult* OutSetDownResult = 0) override;
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;
	
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual float GetMaxSpeed() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BreakingDeceleration) override;
	virtual void ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations) override;
	
	virtual bool CanSprintInCurrentState();
	virtual bool CanRunInCurrentState();

	float GetWeightSpeedMultiplier() const;

	bool IsSprinting();
	bool IsRunning();

	void Sprint(bool bClientSimulation = false);
	void StopSprint(bool bClientSimulation = false);
	void Run(bool bClientSimulation = false);
	void StopRun(bool bClientSimulation = false);
protected:
	FDECharacterNetworkMoveDataContainer MoveDataContainer;
	
	UPROPERTY(Transient, DuplicateTransient)
	TObjectPtr<class ACharacterBase> CustomCharacterOwner;
	virtual void BeginPlay() override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void InitializeComponent() override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 300.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float SprintSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float CrouchSpeed = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float ExhaustedSpeed = 80.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement", meta = (AllowedPrivateAccess = "true"))
	float JumpVelocity = 200.f;
	/* Momentum & Inertia */
	// How fast character reaches max speed from standstill (per movement state)
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float WalkAcceleration = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float RunAcceleration = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float SprintAcceleration = 5.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float CrouchAcceleration = 12.f;
	// How fast character stops
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float WalkDeceleration = 35.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float RunDeceleration = 25.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float SprintDeceleration = 15.f; // Sprint bleeds off slower
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float CrouchDeceleration = 40.f;

	// Extra friction applied when sharply changing direction 
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float TurnFriction = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Momentum", meta = (AllowedPrivateAccess = "true"))
	float LandingSpeedBleedAmount = 0.25f;

	// Encumberance speed modifier
	UPROPERTY(EditDefaultsOnly, Category = "Character Movement|Weight", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "1.0"))
	float MinWeightSpeedMultiplier = 0.3;
};
