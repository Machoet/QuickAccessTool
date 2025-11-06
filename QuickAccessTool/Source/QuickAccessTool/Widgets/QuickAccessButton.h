#pragma once

#if ENGINE_MAJOR_VERSION == 4
	#include "EngineCode/QuickAccessButtonH_4.txt"
	#include "EngineCode/QuickAccessButtonCPP_4.txt"

#elif ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION <= 3
	#include "EngineCode/QuickAccessButtonH_5_3.txt"
	#include "EngineCode/QuickAccessButtonCPP_5_3.txt"

#elif ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION <= 6 && ENGINE_MINOR_VERSION > 3
	#include "EngineCode/QuickAccessButtonH_5_6.txt"
	#include "EngineCode/QuickAccessButtonCPP_5_6.txt"
#endif