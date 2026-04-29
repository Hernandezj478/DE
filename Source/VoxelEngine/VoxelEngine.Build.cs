using UnrealBuildTool;

public class VoxelEngine: ModuleRules
{
    public VoxelEngine(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ProceduralMeshComponent"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "CoreUObject", 
                "Engine",
                "NavigationSystem",
                "UtilityFeatures",
                "MessageHandler",
            }
        );
    }
}
