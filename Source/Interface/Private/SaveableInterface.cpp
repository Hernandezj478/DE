#include "SaveableInterface.h"
#include "Logger.h"

// Add default functionality here for any ISaveableInterface functions that are not pure virtual.

FName ISaveableInterface::GetSaveID_Implementation() const
{
    return FName();
}

bool ISaveableInterface::CollectSaveData_Implementation(FEntitySaveRecord& OutRecord) const
{
    return false;
}

void ISaveableInterface::ApplySaveData_Implementation(const FEntitySaveRecord& Record)
{

}

void ISaveableInterface::OnPreSave_Implementation()
{

}

void ISaveableInterface::OnPostLoad_Implementation()
{

}
