// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::HandleWallSlide()
{
	// TODO
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

