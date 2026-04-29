using UnrealBuildTool;

public class Environment : ModuleRules
{
    public Environment(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "UtilityFeatures",
                "MessageHandler",
                "DE"
            }
        );
    }
}