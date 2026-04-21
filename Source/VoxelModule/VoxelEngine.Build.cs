using UnrealBuildTool;

public class VoxelEngine: ModuleRules
{
    public VoxelEngine(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {

            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "CoreData"
            }
        );
    }
}
