using UnrealBuildTool;

public class CharacterStatline: ModuleRules
{
    public CharacterStatline(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Core", 
            "CoreUObject", 
            "Engine"
        });
    }
}
