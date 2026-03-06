#include "Logger.h"

DEFINE_LOG_CATEGORY(LogCharacters);
DEFINE_LOG_CATEGORY(LogCoreData);
DEFINE_LOG_CATEGORY(LogCoreSystems);
DEFINE_LOG_CATEGORY(LogUtilityFeatures);
DEFINE_LOG_CATEGORY(LogEnvironment);

Logger* Logger::pInstance = nullptr;