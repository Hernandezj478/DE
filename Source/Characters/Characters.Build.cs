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
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "CoreData",
                "Statline",
                "Inventory",
                "UtilityFeatures",
                "MessageHandler"
            }
        );
    }
}