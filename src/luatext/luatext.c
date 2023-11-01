#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define FILE_READ_BUFFER_SIZE 65536
#define INPUT_READ_BUFFER_SIZE 256

/*----------------------------------
	Prints error to the console.
-----------------------------------*/
const char* lt_GetError(lua_State* L)
{
	const char* error = lua_tostring(L, -1);
	printf("\n%s\n", error);
	system("pause");
	return error;
}

/*----------------------------------
	Tries to run a file and
	checks for errors.
-----------------------------------*/
void lt_RunFile(lua_State* L, const char* path)
{
	if (luaL_dofile(L, path) != LUA_OK)
	{
		lt_GetError(L);
	}
}

/*----------------------------------
	Makes the program sleep
	for a given amount of time.
-----------------------------------*/
#ifdef _WIN32
static int LT_Sleep(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TNUMBER)
	{
		luaL_error(L, "%s", "expected number");
		return 1;
	}
	DWORD time = (DWORD)lua_tonumber(L, 1);
	Sleep(time);

	return 1;
}
#endif

/*----------------------------------
	Opens and runs a Lua file.
-----------------------------------*/
static int LT_OpenFile(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TSTRING)
	{
		luaL_error(L, "%s", "expected string");
		return 1;
	}
	const char* path = lua_tostring(L, 1);

	lt_RunFile(L, path);

	return 1;
}

/*----------------------------------
	Changes colors in console.
-----------------------------------*/
#ifdef _WIN32
static int LT_Color(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TNUMBER)
	{
		luaL_error(L, "%s", "expected number");
		return 1;
	}
	WORD number = (WORD)lua_tonumber(L, 1);
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, number);

	return 1;
}
#endif

/*----------------------------------
	Runs system()
-----------------------------------*/
static int LT_System(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TSTRING)
	{
		luaL_error(L, "%s", "expected string");
		return 1;
	}
	const char* cmd = lua_tostring(L, 1);
	system(cmd);

	return 1;
}

/*----------------------------------
	Changes cursor position.
-----------------------------------*/
#ifdef _WIN32
static int LT_Cursor(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TNUMBER || lua_type(L, 2) != LUA_TNUMBER)
	{
		luaL_error(L, "%s", "expected number");
		return 2;
	}

	COORD coordinates;
	coordinates.X = (SHORT)lua_tonumber(L, 1);
	coordinates.Y = (SHORT)lua_tonumber(L, 2);

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(console, coordinates);

	return 2;
}
#endif

/*----------------------------------
	Reads contents of a file.
-----------------------------------*/
static int LT_ReadFile(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TSTRING)
	{
		luaL_error(L, "%s", "expected string");
		return 1;
	}

	const char* path = lua_tostring(L, 1);

	FILE* handle = NULL;
	fopen_s(&handle, path, "r");
	if (!handle)
	{
		luaL_error(L, "%s", "Unable to open file");
		return 2;
	}

	char* buffer = (char*)malloc(sizeof(char) * FILE_READ_BUFFER_SIZE);

	char ch = 0;
	uint64_t i = 0;

	for (i = 0; ch != EOF && i < FILE_READ_BUFFER_SIZE - 1; i++)
	{
		ch = fgetc(handle);
		buffer[i] = ch;
	}

	buffer[i] = '\0';

	lua_pushstring(L, buffer);

	fclose(handle);

	free(buffer);

	return 1;
}

/*----------------------------------
	Saves string to a file.
-----------------------------------*/
static int LT_SaveFile(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TSTRING || lua_type(L, 2) != LUA_TSTRING)
	{
		luaL_error(L, "%s", "expected string");
		return 2;
	}

	const char* path = lua_tostring(L, 1);
	const char* content = lua_tostring(L, 2);

	FILE* handle = NULL;
	fopen_s(&handle, path, "w+");
	if (!handle)
	{
		luaL_error(L, "%s", "Unable to open file");
		return 2;
	}

	fwrite(content, sizeof(char), strlen(content), handle);

	fclose(handle);

	return 2;
}

/*----------------------------------
	Retrieves a string from user.
-----------------------------------*/
static int LT_ReadString(lua_State* L)
{
	const char* msg = 0;
	if (lua_type(L, 1) == LUA_TSTRING)
	{
		msg = lua_tostring(L, 1);
		printf("%s", msg);
	}

	char input[INPUT_READ_BUFFER_SIZE] = { 0 };

	fgets(input, INPUT_READ_BUFFER_SIZE, stdin);
	input[strcspn(input, "\n")] = 0; // Removes the \n
	lua_pushstring(L, input);

	return 1;
}

/*----------------------------------
	Retrieves a number from user.
-----------------------------------*/
static int LT_ReadNumber(lua_State* L)
{
	const char* msg = 0;
	if (lua_type(L, 1) == LUA_TSTRING)
	{
		msg = lua_tostring(L, 1);
		printf("%s", msg);
	}

	char input[INPUT_READ_BUFFER_SIZE] = { 0 };
	fgets(input, INPUT_READ_BUFFER_SIZE, stdin);
	double number = atof(input);

	lua_pushnumber(L, number);

	return 1;
}

static const struct luaL_Reg library[] = {
#ifdef _WIN32
	{"Sleep", LT_Sleep},
	{"Color", LT_Color},
	{"SetCursorPos", LT_Cursor},
#endif
	{"OpenFile", LT_OpenFile},
	{"System", LT_System},
	{"ReadFile", LT_ReadFile},
	{"SaveFile", LT_SaveFile},
	{"ReadString", LT_ReadString},
	{"ReadNumber", LT_ReadNumber},
	{NULL, NULL}
};

int main()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_openlib(L, "lt", library, 0);

	lt_RunFile(L, "scripts/main.lua");
}