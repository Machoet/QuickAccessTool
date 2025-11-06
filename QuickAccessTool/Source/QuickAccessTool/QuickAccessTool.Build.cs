// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class QuickAccessTool : ModuleRules
{
	public QuickAccessTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"AssetRegistry",
				"UnrealEd",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"ApplicationCore",
				"AssetManagerEditor",
				"EditorStyle",
				"Json",
				"JsonUtilities",
				"AppFramework",
				"Blutility",
				"DeveloperSettings"
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}