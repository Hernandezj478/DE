#pragma once

#include "CoreMinimal.h"

UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCharacters, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCoreData, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogCoreSystems, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogUtilityFeatures, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogEnvironment, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxelEngine, Log, All);
UTILITYFEATURES_API DECLARE_LOG_CATEGORY_EXTERN(LogSaveGameSubsystem, Log, All);

#define LOGPREFACE L"LogFile"

#define LOG_MSG(Level, Format, ...) \
	Logger::GetInstance()->AddMessage( \
		FString::Printf(TEXT("%s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__), \
		Level)

#define LOG_UE_MSG(Category, Level, UELevel, Format, ...) \
	do{ \
		LOG_MSG(Level, Format, ##__VA_ARGS__);\
		UE_LOG(Category, UELevel, TEXT("%s: " Format), TEXT(__FUNCTION__), ##__VA_ARGS__);\
	} while(0)

#define LOG_DEBUG(Category, Format, ...)\
	LOG_UE_MSG(Category, DEBUG, Type::Log, Format, ##__VA_ARGS__)

#define LOG_WARNING(Category, Format, ...)\
	LOG_UE_MSG(Category, WARNING, Type::Warning, Format, ##__VA_ARGS__)

#define LOG_ERROR(Category, Format, ...)\
	LOG_UE_MSG(Category, ERROR, Type::Error, Format, ##__VA_ARGS__)

#define LOG_CRITICAL(Category, Format, ...)\
	LOG_UE_MSG(Category, CRITICAL, Type::Fatal, Format, ##__VA_ARGS__)


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
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	static Logger* GetInstance()
	{
		static Logger Instance;
		return &Instance;
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

		FFileHelper::SaveStringToFile(LogLine, *LogFile, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
		
#if UE_EDITOR
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Key, 2.f, ErrorLevelAsColor(Level), Message);
		}
		if (Level == CRITICAL)
		{
			check(false);
		}
#endif
	}
private:
	FString LogFile;


	Logger()
	{
		LogFile = Path();
	}
	~Logger() {}

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
			//FLinearColor color = FLinearColor::Gray;
			return FLinearColor::Gray.ToFColor(true);
			//return color.ToFColor(true);
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
