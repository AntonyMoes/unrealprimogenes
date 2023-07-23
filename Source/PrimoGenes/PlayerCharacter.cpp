// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const auto* PlayerController = Cast<APlayerController>(Controller))
	{
		if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const auto MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
		AddMovementInput(GetActorForwardVector(), MovementVector.X);

	auto* MeshComponent = FindComponentByClass<UStaticMeshComponent>();
	const double Rotation = MovementVector.X > 0 ? 0 : 180;
	MeshComponent->SetRelativeRotation(FRotator::MakeFromEuler(UE::Math::TVector<double>(0, 0, Rotation)));
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	// TODO
}

bool APlayerCharacter::CanJumpInternal_Implementation() const {
	return Super::CanJumpInternal_Implementation() || bInWallSlide;
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleWallSlide();
}

void APlayerCharacter::HandleWallSlide()
{
	auto SetWallSlide = [this](auto inWallSlide)
	{
		if (bInWallSlide && !inWallSlide)
			GLog->Log("Not in Wall slide");
		else if (!bInWallSlide && inWallSlide)
			GLog->Log("In Wall slide");
		bInWallSlide = inWallSlide;
	};

	if (!GetMovementComponent()->IsFalling())
	{
		SetWallSlide(false);
		return;
	}

	const auto Location = GetActorLocation();
	auto CollisionShape = GetCapsuleComponent()->GetCollisionShape();
	CollisionShape.SetCapsule(CollisionShape.Capsule.Radius + 1, CollisionShape.Capsule.HalfHeight);
	FCollisionQueryParams IgnoreSelf(FName(TEXT("TraceParam")), false, this);
	FHitResult HitResult;
	auto hit = GetWorld()->SweepSingleByChannel(HitResult, Location, Location, FQuat::Identity, ECC_Visibility, CollisionShape, IgnoreSelf);
	if (hit && (!bInWallSlide || GetVelocity().Z <= 0))
	{
		auto Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.ImpactNormal, GetLastMovementInputVector())));
		auto InputIntoWallSlide = Angle > 90;

		if (InputIntoWallSlide)
			SetWallSlide(true);
	}
	else
	{
		SetWallSlide(false);
	}

	if (bInWallSlide)
	{
		auto CurrentVelocity = GetMovementComponent()->Velocity;
		CurrentVelocity.Z = 0.2f;
		GetMovementComponent()->Velocity = CurrentVelocity;
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (auto* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Jump);
		Input->BindAction(LookAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping);
	}
}

