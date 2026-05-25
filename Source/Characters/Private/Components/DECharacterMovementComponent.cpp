#include "Components/DECharacterMovementComponent.h"
#include "CharacterBase.h"
#include "MessagingSubsystem.h"
#include "Logger.h"

DEFINE_LOG_CATEGORY(LogMovement);
// =========================================================
// FDECharacterNetworkMoveData
// =========================================================
void FDECharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);
	const FDESavedMove& SavedMove = static_cast<const FDESavedMove&>(Move);
	// Custom move vars needed for transmit
	bWantsToSprint = SavedMove.bWantsToSprint;
	bWantsToRun = SavedMove.bWantsToRun;
}

bool FDECharacterNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Ar, UPackageMap* PackageMap,
	ENetworkMoveType MoveType)
{
	Super::Serialize(Movement, Ar, PackageMap, MoveType);

	bool bSprint = !!bWantsToSprint;
	bool bRun = !!bWantsToRun;

	Ar.SerializeBits(&bSprint, 1);
	Ar.SerializeBits(&bRun, 1);

	if(Ar.IsLoading())	
	{
		bWantsToSprint = bSprint;
		bWantsToRun = bRun;
	}
	
	return !Ar.IsError();
}
// =========================================================
// FDECharacterNetworkMovedataContainer
// =========================================================
FDECharacterNetworkMoveDataContainer::FDECharacterNetworkMoveDataContainer()
{
	NewMoveData		= &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData		= &MoveData[2];
}
// =========================================================
// FDESavedMove
// =========================================================
bool FDESavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FDESavedMove* NewDEMove = static_cast<const FDESavedMove*>(NewMove.Get());
	if (bWantsToSprint	!= NewDEMove->bWantsToSprint)	return false;
	if (bWantsToRun		!= NewDEMove->bWantsToRun)		return false;
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FDESavedMove::Clear()
{
	Super::Clear();
	bWantsToSprint = 0;
	bWantsToRun = 0;
}

uint8 FDESavedMove::GetCompressedFlags() const
{
	uint8 Flags = Super::GetCompressedFlags();
	if (bWantsToSprint) Flags |= FLAG_Custom_0;
	if (bWantsToRun) Flags |= FLAG_Custom_1;
	return Flags;

}

void FDESavedMove::SetMoveFor(ACharacter* C, float InDeltaTime, const FVector& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	if (UDECharacterMovementComponent* CMC = Cast<UDECharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bWantsToSprint	= CMC->bWantsToSprint;
		bWantsToRun		= CMC->bWantsToRun;
	}
}

void FDESavedMove::PrepMoveFor(ACharacter * C)
{
	Super::PrepMoveFor(C);
	if (UDECharacterMovementComponent* CMC = Cast<UDECharacterMovementComponent>(C->GetCharacterMovement()))
	{
		CMC->bWantsToSprint = bWantsToSprint;
		CMC->bWantsToRun	= bWantsToRun;
	}
}

void FDESavedMove::CombineWith(const FSavedMove_Character * OldMove, ACharacter * InCharacter, APlayerController * PC, const FVector & OldStartLocation)
{
	Super::CombineWith(OldMove, InCharacter, PC, OldStartLocation);
}
// =========================================================
// FDENetworkPreditionData_Client_Character
// =========================================================
FDENetworkPredictionData_Client_Character::FDENetworkPredictionData_Client_Character(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
	
}

FSavedMovePtr FDENetworkPredictionData_Client_Character::AllocateNewMove()
{
	return MakeShared<FDESavedMove>();
}
// =========================================================
// UDECharacterMovementComponent
// =========================================================
UDECharacterMovementComponent::UDECharacterMovementComponent()
{
	bOrientRotationToMovement = false;
	RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	JumpZVelocity = JumpVelocity;
	// No air control - movement should feel grounded
	AirControl = 0.f;
	BrakingDecelerationFalling = 0.f;
	FallingLateralFriction = 0.f;
	
	GetNavAgentPropertiesRef().bCanCrouch = true;
	MaxWalkSpeed = WalkSpeed;
	MaxWalkSpeedCrouched = CrouchSpeed;
	MinAnalogWalkSpeed = 20.0f;

	// BreadkingDecelerationWalking is overridden by GetMaxBreakingDeceleration - 
	// set to 0 so base class never fights custom CalcVelocity
	BrakingDecelerationWalking = 0.0f;
	SetNetworkMoveDataContainer(MoveDataContainer);
}

UDECharacterMovementComponent::~UDECharacterMovementComponent()
{
}

void UDECharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
}

void UDECharacterMovementComponent::PhysNavWalking(float DeltaTime, int32 Iterations)
{
	Super::PhysNavWalking(DeltaTime, Iterations);
}

void UDECharacterMovementComponent::PhysWalking(float DeltaTime, int32 Iterations)
{
	Super::PhysWalking(DeltaTime, Iterations);
}

void UDECharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds, FStepDownResult* OutSetDownResult)
{
	Super::MoveSmooth(InVelocity, DeltaSeconds, OutSetDownResult);
}

void UDECharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	if (FDECharacterNetworkMoveData* MoveData = static_cast<FDECharacterNetworkMoveData*>(GetCurrentNetworkMoveData()))
	{
		bWantsToSprint	= MoveData->bWantsToSprint;
		bWantsToRun		= MoveData->bWantsToRun;
	}
	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

float UDECharacterMovementComponent::GetMaxAcceleration() const
{
	if (!IsMovingOnGround())
	{
		return Super::GetMaxAcceleration();
	}
	if (IsCrouching())	
	{
		return CrouchAcceleration * GetMaxSpeed();
	}
	if (bWantsToSprint) 
	{
		return SprintAcceleration * GetMaxSpeed();
	}
	if (bWantsToRun)	
	{
		return RunAcceleration * GetMaxSpeed();
	}

	return WalkAcceleration * GetMaxSpeed();
}

float UDECharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (!IsMovingOnGround())
	{
		return Super::GetMaxBrakingDeceleration();
	}
	if (IsCrouching())	return CrouchDeceleration	* GetMaxSpeed();
	if (bWantsToSprint) return SprintDeceleration	* GetMaxSpeed();
	if (bWantsToRun)	return RunDeceleration		* GetMaxSpeed();

	return WalkDeceleration * GetMaxSpeed();
}

float UDECharacterMovementComponent::GetMaxSpeed() const
{
	float WeightMultiplier = GetWeightSpeedMultiplier();
	if (bWantsToSprint)
	{
		return SprintSpeed * WeightMultiplier;
	}
	if (bWantsToRun)
	{
		return RunSpeed * WeightMultiplier;
	}
	// Default for walk, crouch, etc.
	return Super::GetMaxSpeed() * WeightMultiplier;
}

void UDECharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	// Proxies get replicated Sprint/Run State
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Check for change in sprint. Players toggle sprint by changing bWantsToSprint
		bool bIsSprinting = IsSprinting();
		if (bIsSprinting && (!bWantsToSprint || !CanSprintInCurrentState()))
		{
			StopSprint(false);
		}
		else if (!bIsSprinting && bWantsToSprint && CanSprintInCurrentState())
		{
			Sprint(false);
		}

		// Check for change in run. Players toggle run by changing bWantsToRun
		bool bIsRunning = IsRunning();
		if (bIsRunning && (!bWantsToRun || !CanRunInCurrentState()))
		{
			StopRun(false);
		}
		else if (!bIsRunning && bWantsToRun && CanRunInCurrentState())
		{
			Run(false);
		}
	}
}

void UDECharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	// Proxies get replicated
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Stop sprint if no longer allowed to sprint
		if (IsSprinting() && !CanSprintInCurrentState())
		{
			StopSprint(false);
		}
		// Stop run if no longer allowed to run
		if (IsRunning() && !CanRunInCurrentState())
		{
			StopRun(false);
		}
	}
}

void UDECharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BreakingDeceleration)
{
	if (!IsMovingOnGround())
	{
		Super::CalcVelocity(DeltaTime, Friction, bFluid, BreakingDeceleration);
		return;
	}
	const FVector InputVector = GetCurrentAcceleration().GetSafeNormal();
	const bool bHasInput = !InputVector.IsNearlyZero();

	// Turn friction
	if (bHasInput && !Velocity.IsNearlyZero())
	{
		const FVector CurrentDir = Velocity.GetSafeNormal();
		const float DirectionDot = FVector::DotProduct(CurrentDir, InputVector);

		if (DirectionDot < 0.f)
		{
			const float MappedFriction = FMath::GetMappedRangeValueClamped(
				FVector2D(-1.f, 0.f), 
				FVector2D(TurnFriction, 0.f),
				DirectionDot);
			Velocity *= FMath::Clamp(1.f - MappedFriction * DeltaTime, 0.f, 1.f);
		}
	}
	// Acceleration / Deceleration LERP
	const FVector TargetVelocity = bHasInput ? InputVector * GetMaxSpeed() : FVector::ZeroVector;
	const float AccelRate = bHasInput ? GetMaxAcceleration() / GetMaxSpeed() : GetMaxBrakingDeceleration() / GetMaxSpeed();
	const float LerpAlpha = FMath::Clamp(AccelRate * DeltaTime, 0.f, 1.f);
	FVector NewVelocity = FMath::Lerp(Velocity, TargetVelocity, LerpAlpha);
	NewVelocity.Z = Velocity.Z;
	Velocity = NewVelocity;
}

void UDECharacterMovementComponent::ProcessLanded(const FHitResult& Hit, float RemainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, RemainingTime, Iterations);
	// ImpactNormal.Z approaches 1.0 for flat ground, lower for steep surfaces
	// Steep landings bleed more speed
	const float ImpactZ = FMath::Abs(Hit.ImpactNormal.Z);
	const float SpeedRetention = FMath::GetMappedRangeValueClamped(
		FVector2D(0.7f, 1.f), 
		FVector2D(1.f - LandingSpeedBleedAmount, 1.f), 
		ImpactZ);
	Velocity.X *= SpeedRetention;
	Velocity.Y *= SpeedRetention;
}

bool UDECharacterMovementComponent::CanSprintInCurrentState()
{
	if (CustomCharacterOwner &&
		(UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics() && !GetCurrentAcceleration().IsNearlyZero()))
	{
		return CustomCharacterOwner->CanSprint();
	}
	return false;
}

bool UDECharacterMovementComponent::CanRunInCurrentState()
{
	if (CustomCharacterOwner && (UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics()))
	{
		return CustomCharacterOwner->CanRun();
	}
	return false;
}

float UDECharacterMovementComponent::GetWeightSpeedMultiplier() const
{
	if (!CustomCharacterOwner)
	{
		return 1.f;
	}
	const float WeightRatio = FMath::Clamp(CustomCharacterOwner->GetCarryWeightPercentile(), 0.f, 1.f);
	return FMath::Lerp(1.f, MinWeightSpeedMultiplier, WeightRatio);
}

bool UDECharacterMovementComponent::IsSprinting()
{
	return CustomCharacterOwner && CustomCharacterOwner->IsSprinting();
}

bool UDECharacterMovementComponent::IsRunning()
{
	return CustomCharacterOwner && CustomCharacterOwner->IsRunning();
}

void UDECharacterMovementComponent::Sprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}
	if (!bClientSimulation && !CanSprintInCurrentState())
	{
		return;
	}
	// See if we are already sprinting
	if (CustomCharacterOwner->IsSprinting() == IsSprinting())
	{
		if(!bClientSimulation)
		{
			CustomCharacterOwner->SetIsSprinting(true);
		}
		return;
	}
	// TODO: We might not need this, we might not want to set the sprint state to default on simulated proxy
	if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Restore sprint to default sprint
		ACharacterBase* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacterBase>();
		CustomCharacterOwner->SetIsSprinting(DefaultCharacter->IsSprinting());
	}
	if (!bClientSimulation)
	{
		CustomCharacterOwner->SetIsSprinting(true);
	}
}

void UDECharacterMovementComponent::StopSprint(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}
	if (CustomCharacterOwner)
	{
		CustomCharacterOwner->SetIsSprinting(false);
	}
}

void UDECharacterMovementComponent::Run(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}
	if (!bClientSimulation && !CanRunInCurrentState())
	{
		return;
	}
	// Check if we are already running
	if (CustomCharacterOwner->IsRunning() == IsRunning())
	{
		if(!bClientSimulation)
		{
			CustomCharacterOwner->SetIsRunning(true);
		}
		return;
	}
	
}

void UDECharacterMovementComponent::StopRun(bool bClientSimulation)
{
	if (!HasValidData())
	{
		return;
	}
	if (ACharacterBase* Owner = Cast<ACharacterBase>(CharacterOwner))
	{
		Owner->SetIsRunning(false);
	}
}

void UDECharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasBegunPlay())
	{
		return;
	}
	CustomCharacterOwner = Cast<ACharacterBase>(CharacterOwner);
#if !UE_BUILD_SHIPPING
	if (!CustomCharacterOwner)
	{
		LOG_CRITICAL(LogCharacters, "CustomCharacterOwner missing!");
	}
#endif
}

void UDECharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSprint	= (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToRun		= (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void UDECharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

FNetworkPredictionData_Client* UDECharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UDECharacterMovementComponent* MutableThis = const_cast<UDECharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FDENetworkPredictionData_Client_Character(*this);
	}
	return ClientPredictionData;
}
