#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Con
{
	inline HANDLE g_hOutput = 0;
	inline CONSOLE_SCREEN_BUFFER_INFO g_BufferInfo;
	inline int g_Argc = 0;
	inline char** g_Argv = nullptr;

	__forceinline void SetColor(WORD p_Color)
	{
		SetConsoleTextAttribute(g_hOutput, p_Color);
	}

	void Uninitialize()
	{
		SetColor(g_BufferInfo.wAttributes);
	}

	void Initialize(const char* p_szTitle, int p_Argc, char** p_Argv)
	{
		g_hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(g_hOutput, &g_BufferInfo);
		SetConsoleTitleA(p_szTitle);

		g_Argc = p_Argc;
		g_Argv = p_Argv;

		atexit(Uninitialize);
	}

	template <typename... Args>
	__forceinline void Print(WORD p_Attribute, const char* p_Format, Args... p_Args)
	{
		SetColor(p_Attribute);

		printf(p_Format, p_Args...);

		SetColor(g_BufferInfo.wAttributes);
	}

	template <typename... Args>
	__forceinline void PrintError(const char* p_Format, Args... p_Args)
	{
		Print(12, "[ ERROR ] ");
		Print(12, p_Format, p_Args...);
	}

	template <typename... Args>
	__forceinline void PrintWarning(const char* p_Format, Args... p_Args)
	{
		Print(14, "[ WARNING ] ");
		Print(14, p_Format, p_Args...);
	}

	//================================================================
	// Arguments

	std::string GetArgParam(const char* p_Arg)
	{
		for (int i = 0; g_Argc > i; ++i)
		{
			if (_stricmp(p_Arg, g_Argv[i]) != 0) {
				continue;
			}

			int nIndex = (i + 1);
			if (nIndex >= g_Argc) {
				break;
			}

			return g_Argv[nIndex];
		}

		return "";
	}

	int GetArgParamInt(const char* p_Arg)
	{
		std::string sParam = GetArgParam(p_Arg);
		if (sParam.empty()) {
			return 0;
		}

		return atoi(&sParam[0]);
	}

	bool IsArgSet(const char* p_Arg)
	{
		for (int i = 0; g_Argc > i; ++i)
		{
			if (_stricmp(p_Arg, g_Argv[i]) == 0) {
				return true;
			}
		}

		return false;
	}
}