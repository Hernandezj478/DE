#pragma once

#include "CoreMinimal.h"

UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCharacters, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCoreData, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCoreSystems, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogUtilityFeatures, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogEnvironment, Log, All);

#define LOGPREFACE L"LogFile"

enum ErrorLevel
{
  	DEBUG	= 0,
  	WARNING = 1,
  	ERROR	= 2,
  	CRITICAL = 3
  };

class UTILITYFEATURES_API Logger
{
public:
	static Logger* GetInstance()
	{
		if (pInstance == nullptr)
		{
			pInstance = new Logger();
		}
		return pInstance;
	}
	
	void AddMessage(FString Message, ErrorLevel Level, int Key = -1)
	{
		FString LogLine;
		LogLine += "[";
		LogLine += TodaysDateAsString();
		LogLine += "_";
		LogLine += GetCurrentTimeStamp();
		LogLine += "]";
		LogLine += ErrorLevelToString(Level);
		LogLine += " ";
		LogLine += Message;
		LogLine += "\n";

		FFileHelper::SaveStringToFile(LogLine, *Path(), FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		
#if UE_EDITOR
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Key, 5.f, ErrorLevelAsColor(Level), Message);
		}
#endif
	}
private:
	static Logger* pInstance;
	
	FString TodaysDateAsString()
	{
		const FDateTime Time;
		FString Year = IntToString(Time.Now().GetYear());
		FString Month = IntToString(Time.Now().GetMonth());
		FString Day = IntToString(Time.Now().GetDay());
		return Year + "-" + Month + "-" + Day;
	}
	
	FString GetCurrentTimeStamp()
	{
		const FDateTime Time;
		FString Hour = IntToString(Time.Now().GetHour());
		FString Minute = IntToString(Time.Now().GetMinute());
		FString Second = IntToString(Time.Now().GetSecond());
		return Hour + ":" + Minute + ":" + Second;
	}
	
	FString IntToString(const int& i, bool padded = true)
	{
		FString Ret = FString::FromInt(i);
		if (padded && Ret.Len() == 1)
		{
			Ret = "0" + Ret;
		}
		return Ret;
	}
	
	FString ErrorLevelToString(ErrorLevel Level)
	{
		switch (Level)
		{
		case DEBUG:
			return FString("[DEBUG]");
		case WARNING:
			return FString("[WARNING]");
		case ERROR:
			return FString("[ERROR]");
		case CRITICAL:
			return FString("[CRITICAL]");
		default:
			return FString("[UNKNOWN]");
		}
	}
	
	FColor ErrorLevelAsColor(ErrorLevel Level)
	{
		switch (Level)
		{
		case DEBUG:
			FLinearColor color = FLinearColor::Gray;
			return color.ToFColor(true);
		case WARNING:
			return FColor::Yellow;
		case ERROR:
			return FColor::Orange;
		case CRITICAL:
			return FColor::Red;
		default:
			return FColor::White;
		}
	}
	
	FString Path()
	{
		FString ProjectDir = FPaths::ProjectDir();
		FString LogDir = FPaths::Combine(ProjectDir, TEXT("Saved/Logs/DE"));
		if (!FPaths::FileExists(LogDir))
		{
			IFileManager::Get().MakeDirectory(*LogDir);
		}
		FString LogFileName = FString::Printf(TEXT("%s_%s.log"), LOGPREFACE, *TodaysDateAsString());
		FString FullLogPath = FPaths::Combine(LogDir, LogFileName);
		return FullLogPath;
	}
};
