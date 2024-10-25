#pragma once

namespace Con
{
	int gArgc;
	char** gArgv;

	void Initialize(int argc, char** argv)
	{
		gArgc = argc;
		gArgv = argv;
	}

	int GetArgIndex(const char* arg)
	{
		for (int i = 0; gArgc > i; ++i)
		{
			if (UFG::qStringCompareInsensitive(gArgv[i], arg)) {
				continue;
			}

			return i;
		}

		return -1;
	}

	bool HasArg(const char* arg)
	{
		return (GetArgIndex(arg) != -1);
	}

	UFG::qString GetArg(const char* arg)
	{
		int argIndex = GetArgIndex(arg) + 1;
		if (argIndex == 0 || argIndex >= gArgc) {
			return {};
		}

		return gArgv[argIndex];
	}
}