// Fill out your copyright notice in the Description page of Project Settings.


#include "EventBus.h"

UEventBus* UEventBus::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}
	return GEngine->GetEngineSubsystem<UEventBus>();
}

