using UnrealBuildTool;

public class CharacterMovement: ModuleRules
{
    public CharacterMovement(ReadOnlyTargetRules Target) : base(Target)
    {
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
                    "MessageHandler",
                    "UtilityFeatures",
                }
            );
    }
}
