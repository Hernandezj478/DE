using UnrealBuildTool;

public class Characters : ModuleRules
{
    public Characters(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "EnhancedInput",
                "Interface"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "CoreData",
                "Inventory",
                "UtilityFeatures",
                "MessageHandler",
                "VoxelEngine",
                "SaveSystem"
            }
        );
    }
}