// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetShootGame : ModuleRules
{
	public NetShootGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"HeadMountedDisplay",
				
				// Use push model to speed up rep
				"NetCore",
				"AIModule"
			}
		);

		// PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemUtils" });
	}
}
