// Fill out your copyright notice in the Description page of Project Settings.


#include "DEEventBus.h"

UDEEventBus* UDEEventBus::Get()
{
	if (!GEngine)
	{
		return nullptr;
	}
	
	return GEngine->GetEngineSubsystem<UDEEventBus>();
}
