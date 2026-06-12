// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DedicatedX : ModuleRules
{
	public DedicatedX(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
            
            //UI
            "UMG"
        });

        PublicIncludePaths.AddRange(new string[]
        {
        });
    }
}
