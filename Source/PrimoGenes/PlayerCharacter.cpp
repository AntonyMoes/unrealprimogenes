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

void APlayerCharacter::TryJump()
{
	if (!bInWallSlide)
	{
		Jump();
		return;
	}

	constexpr auto JumpSpeed = 700;

	const auto TargetComponent = FindComponentByClass<UPrimitiveComponent>();
	FVector JumpVector(-1 * WallSlideDirection, 0, 1);
	JumpVector.Normalize();
	JumpVector *= JumpSpeed;

	GetMovementComponent()->Velocity = JumpVector;
	SetWallSlideState(false);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleWallSlide();
}

void APlayerCharacter::HandleWallSlide()
{
	if (!GetMovementComponent()->IsFalling())
	{
		SetWallSlideState(false);
		return;
	}

	const auto Location = GetActorLocation();
	auto CollisionShape = GetCapsuleComponent()->GetCollisionShape();
	CollisionShape.SetCapsule(CollisionShape.Capsule.Radius + 1, CollisionShape.Capsule.HalfHeight);
	FCollisionQueryParams IgnoreSelf(FName(TEXT("TraceParam")), false, this);
	FHitResult HitResult;
	auto bHit = GetWorld()->SweepSingleByChannel(HitResult, Location, Location, FQuat::Identity,
		ECC_Visibility, CollisionShape, IgnoreSelf);
	if (bHit && (!bInWallSlide || GetVelocity().Z <= 0))
	{
		auto LastMovementInput = GetLastMovementInputVector();
		auto HorizontalInput = LastMovementInput.X;
		auto Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(HitResult.ImpactNormal, LastMovementInput)));

		auto InputIntoWallSlide = Angle > 90 && HorizontalInput != 0;
		if (InputIntoWallSlide)
			SetWallSlideState(true, HorizontalInput > 0 ? 1 : -1);
	}
	else
	{
		SetWallSlideState(false);
	}

	if (bInWallSlide)
	{
		auto CurrentVelocity = GetMovementComponent()->Velocity;
		CurrentVelocity.Z = 0.2f;
		GetMovementComponent()->Velocity = CurrentVelocity;
	}
}

void APlayerCharacter::SetWallSlideState(bool InWallSlide, int Direction)
{
	if (bInWallSlide && !InWallSlide)
		GLog->Log("Not in Wall slide");
	else if (!bInWallSlide && InWallSlide)
		GLog->Log("In Wall slide");
	bInWallSlide = InWallSlide;
	WallSlideDirection = Direction;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (const auto Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		Input->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::TryJump);
		Input->BindAction(LookAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopJumping);
	}
}

