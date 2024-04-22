//
// Created by Dottik on 14/11/2023.
//
#include "Environment.hpp"
#include <iostream>
#include "Closures.hpp"
#include "Dependencies/HttpStatus.hpp"
#include "Dependencies/Termcolor.hpp"
#include "Dependencies/cpr/include/cpr/cpr.h"
#include "Execution.hpp"
#include "Utilities.hpp"
#include "WebsocketLibrary.hpp"

#include "ClosureLibrary.hpp"
#include "DebugLibrary.hpp"
#include "Dependencies/Luau/VM/include/lua.h"
#include "Dependencies/Luau/VM/src/lgc.h"
#include "Dependencies/Luau/VM/src/lmem.h"
#include "Dependencies/Luau/VM/src/lvm.h"
#include "Hook.hpp"
#include "Scheduler.hpp"
#include "Security.hpp"
#include "ltable.h"

Environment *Environment::sm_pSingleton = nullptr;

Environment *Environment::get_singleton() {
    if (sm_pSingleton == nullptr)
        sm_pSingleton = new Environment();
    return sm_pSingleton;
}

int getreg(lua_State *L) {
    auto scheduler{Scheduler::get_singleton()};

    L->top->tt = LUA_TTABLE;
    L->top->value = scheduler->get_global_roblox_state()->global->registry.value;
    L->top++;
    return 1;
}

int getgenv(lua_State *L) {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}

int getrenv(lua_State *L) {
    auto scheduler{Scheduler::get_singleton()};

    lua_State *gL = scheduler->get_global_roblox_state();
    lua_pushvalue(gL, LUA_GLOBALSINDEX);
    lua_xmove(gL, L, 1);
    return 1;
}

int consoleprint(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);
    std::wstringstream strStream;
    strStream << "[INFO] ";
    for (int i = 0; i <= argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->to_wstring(lStr).c_str() << " ";
    }
    std::wcout << termcolor::green << strStream.str() << termcolor::reset << std::endl;

    return 0;
}

int consolewarn(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);
    std::wstringstream strStream;
    strStream << "[WARN] ";
    for (int i = 0; i <= argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->to_wstring(lStr).c_str() << " ";
    }
    std::wcout << termcolor::yellow << strStream.str() << termcolor::reset << std::endl;

    return 0;
}

int consoleerror(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);

    std::wstringstream strStream;
    strStream << "[ERROR] ";
    for (int i = 0; i < argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->to_wstring(lStr).c_str() << " ";
    }
    std::wcerr << termcolor::red << strStream.str() << termcolor::reset << std::endl;

    luaG_runerror(L, utilities->to_string(strStream.str()).c_str());
}

int getgc(lua_State *L) {
    const bool addTables = luaL_optboolean(L, 1, false);

    lua_createtable(L, 0, 0); // getgc table, prealloc sum space, because yes.

    typedef struct {
        lua_State *pLua;
        bool accessTables;
        int itemsFound;
    } GCOContext;

    auto gcCtx = GCOContext{L, addTables, 0};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
    const auto ullOldThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;
    // Never return true. We aren't deleting shit.
    luaM_visitgco(L, &gcCtx, [](void *ctx, lua_Page *pPage, GCObject *pGcObj) -> bool {
        const auto pCtx = static_cast<GCOContext *>(ctx);
        const auto ctxL = pCtx->pLua;

        if (iswhite(pGcObj))
            return false; // The object is being collected/checked. Skip it.

        if (const auto gcObjType = pGcObj->gch.tt;
            gcObjType == LUA_TFUNCTION || gcObjType == LUA_TUSERDATA || gcObjType == LUA_TTABLE && pCtx->accessTables) {
            // Push copy to top of stack.
            ctxL->top->value.gc = pGcObj;
            ctxL->top->tt = gcObjType;
            ctxL->top++;

            // Store onto GC table.
            const auto tIndx = pCtx->itemsFound++;
            lua_rawseti(ctxL, -2, tIndx + 1);
        }
        return false;
    });
    L->global->GCthreshold = ullOldThreshold;
#pragma clang diagnostic pop

    return 1;
}

int httppost(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TSTRING);
    luaL_checktype(L, 4, LUA_TSTRING);

    const std::string targetUrl = lua_tostring(L, 2);

    if (targetUrl.find("http://") == std::string::npos && targetUrl.find("https://") == std::string::npos) {
        luaG_runerror(L, "Invalid protocol (expected 'http://' or 'https://')");
    }

    const auto *postData = lua_tostring(L, 3);
    const auto *contentType = lua_tostring(L, 4);

    const auto response = cpr::Post(cpr::Url{targetUrl}, cpr::Body{postData},
                                    cpr::Header{{"Content-Type", contentType}, {"User-Agent", "Roblox/WinInet"}});

    if (HttpStatus::IsError(response.status_code)) {
        const std::string Output =
                std::format("HttpPost Failed.\r\nResponse: %s - %s\n", std::to_string(response.status_code),
                            HttpStatus::ReasonPhrase(response.status_code));

        lua_pushstring(L, Output.c_str());
        return 1;
    }


    lua_pushlstring(L, response.text.c_str(), response.text.size());
    return 1;
}

int httpget(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    const std::string targetUrl = lua_tostring(L, 1);

    if (targetUrl.find("http://") == std::string::npos && targetUrl.find("https://") == std::string::npos) {
        luaG_runerror(L, "Invalid protocol (expected 'http://' or 'https://')");
    }
    const auto response = cpr::Get(cpr::Url{targetUrl}, cpr::Header{{"User-Agent", "Roblox/WinInet"}});

    if (HttpStatus::IsError(response.status_code)) {
        const std::string Output =
                std::format("HttpGet Failed.\r\nResponse: %s - %s\n", std::to_string(response.status_code),
                            HttpStatus::ReasonPhrase(response.status_code));

        lua_pushstring(L, Output.c_str());
        return 1;
    }

    lua_pushstring(L, response.text.c_str());
    return 1;
}

int checkcaller(lua_State *L) {
    const auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    // We must include a better checkcaller, at least for the future, this gayass check is killing me so bad.
    lua_pushboolean(L, extraSpace != nullptr && ((extraSpace->identity >= 4 && extraSpace->identity <= 9)));
    // Check identity, the main thread of our original thread HAS to match up as well, else it is not one of our states
    // at ALL! That is also another way we can check with the identity!
    return 1;
}

int getidentity(lua_State *L) {
    const auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    // wprintf(oxorany(L"Current State ExtraSpace:\r\n"));
    // wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    // wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);
    lua_pushnumber(L, extraSpace->identity);
    return 1;
}

int setidentity(lua_State *L) {
    auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    const auto newIdentity = luaL_optinteger(L, -1, 8);

    if (newIdentity > 9 || newIdentity < 0) {
        luaG_runerrorL(L, ("You may not set your identity below 0 or above 9."));
    }

    // The identity seems to be consulted on the L->mainthread. Due to this, we may need to keep two lua states running.
    // One originating from an elevated one, and one originated from a non elevated one. With this we can bypass
    // identity, while still being able to use functions like require, which are literally a REQUIREment. Heh.

    // wprintf(oxorany(L"Old State ExtraSpace:\r\n"));
    // wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    // wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);

    extraSpace->identity = newIdentity; // Identity bypass.
    extraSpace->capabilities = 0x3FFFF00 | RBX::Security::to_obfuscated_identity(newIdentity);

    // wprintf(oxorany(L"New State ExtraSpace:\r\n"));
    // wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    // wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);

    return 0;
}

int getrawmetatable(lua_State *L) {
    luaL_checkany(L, 1);
    if (!lua_getmetatable(L, 1))
        lua_pushnil(L);

    return 1;
}

int setrawmetatable(lua_State *L) {
    luaL_argexpected(L, lua_istable(L, 1) || lua_islightuserdata(L, 1) || lua_isuserdata(L, 1), 2,
                     "table or userdata or lightuserdata");

    luaL_checktype(L, 2, LUA_TTABLE);
    lua_setmetatable(L, 1);
    return 0;
}

int setreadonly(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    lua_setreadonly(L, 1, lua_toboolean(L, 2));
    return 1;
}

int isreadonly(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushboolean(L, lua_getreadonly(L, 1));
    return 1;
}

int make_writeable(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushboolean(L, false);
    return setreadonly(L);
}

int make_readonly(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_pushboolean(L, true);
    return setreadonly(L);
}

int getnamecallmethod(lua_State *L) {
    const char *namecall_method = lua_namecallatom(L, nullptr);
    if (namecall_method == nullptr)
        lua_pushnil(L);
    else {
        lua_pushlstring(L, namecall_method, strlen(namecall_method));
    }

    return 1;
}

int setnamecallmethod(lua_State *L) {
    luaL_checkstring(L, 1);
    if (L->namecall != nullptr)
        L->namecall = &L->top->value.gc->ts;
    return 0;
}


int identifyexecutor(lua_State *L) {
    lua_pushstring(L, "RbxStu");
    lua_pushstring(L, "1.0.0");
    return 2;
}

int hookmetamethod(lua_State *L) {
    // Crashes on usage due to table related issues.
    if (lua_type(L, 1) != LUA_TTABLE && lua_type(L, 1) != LUA_TUSERDATA) {
        luaL_typeerrorL(L, 1, "table or userdata");
    }

    luaL_checktype(L, 2, LUA_TSTRING);
    luaL_checktype(L, 3, LUA_TFUNCTION);

    const auto *obj = static_cast<const lua_TValue *>(lua_topointer(L, 1));
    Table *mt;

    if (lua_type(L, 1) == LUA_TTABLE)
        mt = hvalue(obj)->metatable;
    else
        mt = uvalue(obj)->metatable;

    if (!mt)
        luaL_typeerrorL(L, 1, "table or userdata with a metatable.");

    const auto metamethodName = lua_tostring(L, 2);

    L->top->tt = LUA_TTABLE;
    L->top->value.p = static_cast<void *>(mt);
    L->top++;
    lua_getfield(L, -1, metamethodName);
    const auto *metamethodCl = static_cast<const lua_TValue *>(lua_topointer(L, -1));

    if (metamethodCl->tt == LUA_TNIL)
        luaL_error(L, "table or userdata has no metamethod with name %s", metamethodName);

    L->top->tt = LUA_TFUNCTION;
    L->top->value.p = static_cast<void *>(const_cast<lua_TValue *>(metamethodCl));
    L->top++; // toHook
    lua_pushvalue(L, 3); // hookWith

    lua_getglobal(L, "hookfunction");
    lua_pcall(L, 2, 1, 0);

    return 1;
}

int gettenv(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTHREAD);
    const auto *otherL = static_cast<const lua_State *>(lua_topointer(L, 1));

    auto nT = luaH_clone(L, otherL->gt);
    L->top->tt = LUA_TTABLE;
    L->top->value.p = static_cast<void *>(nT);
    L->top++;
    return 1;
}

int gethui(lua_State *L) {
    lua_getglobal(L, "game");
    lua_getfield(L, -1, "CoreGui");
    return 1;
}

int isrbxactive(lua_State *L) {
    char buf[0xff];
    if (GetWindowTextA(GetForegroundWindow(), buf, sizeof(buf))) {
        lua_pushboolean(L, strstr(buf, "Roblox Studio") != nullptr);
    } else {
        lua_pushboolean(L, false);
    }

    return 1;
}

int isluau(lua_State *L) {
    lua_pushboolean(L, true);
    return 1;
}

int Environment::register_env(lua_State *L, bool useInitScript) {
    static const luaL_Reg reg[] = {
            {"isluau", isluau},
            {"isrbxactive", isrbxactive},

            {"getreg", getreg},
            {"getgc", getgc},

            {"gettenv", gettenv},
            {"getgenv", getgenv},
            {"getrenv", getrenv},

            {"checkcaller", checkcaller},

            {"setidentity", setidentity},
            {"setthreadidentity", setidentity},
            {"setthreadcontext", setidentity},

            {"getidentity", getidentity},
            {"getthreadidentity", getidentity},
            {"getthreadcontext", getidentity},

            {"getrawmetatable", getrawmetatable},
            {"setrawmetatable", setrawmetatable},

            {"setreadonly", setreadonly},
            {"isreadonly", isreadonly},
            {"make_writeable", make_writeable},
            {"make_readonly", make_readonly},

            {"getnamecallmethod", getnamecallmethod},
            {"setnamecallmethod", setnamecallmethod},

            {"identifyexecutor", identifyexecutor},
            {"getexecutorname", identifyexecutor},


            {"consoleprint", consoleprint},
            {"rconsoleprint", consoleprint},
            {"consolewarn", consolewarn},
            {"rconsolewarn", consolewarn},
            {"consoleerror", consoleerror},
            {"rconsoleerror", consoleerror},
            {"isrbxactive", isrbxactive},
            {"isgameactive", isrbxactive},

            //("hookmetamethod",    hookmetamethod},
            {"HttpGet", httpget},
            {"HttpPost", httppost},

            {"__SCHEDULER_STEPPED__HOOK", (static_cast<lua_CFunction>([](lua_State *L) -> int {
                 if (auto *scheduler{Scheduler::get_singleton()}; scheduler->is_initialized()) {
                     scheduler->scheduler_step(scheduler->get_global_executor_state());
                 }
                 return (0);
             }))},

            {"gethui", gethui},

            // {oxorany_pchar(L"reinit"),            reinit},

            {nullptr, nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_setfield(L, -2, "_G");

    auto closuresLibrary = ClosureLibrary{};
    auto websocketsLibrary = WebsocketLibrary{};
    auto debugLibrary = DebugLibrary{};
    printf("[Envionment::register_env] Registering available libraries...\r\n");
    closuresLibrary.register_environment(L);
    debugLibrary.register_environment(L);
    websocketsLibrary.register_environment(L);

    if (useInitScript) {
        printf("[Envionment::register_env] Pushing initialization script to scheduler for execution...\r\n");


        // Initialize execution,
        const std::string initscript = (R"(
local clonefunction_c = clonefunction(clonefunction)
local checkcaller_c = clonefunction(checkcaller)
local game_getservice = clonefunction_c(game.GetService)
local insertservice_LoadLocalAsset = clonefunction_c(game_getservice(game, "InsertService").LoadLocalAsset)
local string_match = clonefunction_c(string.match)
local string_lower = clonefunction_c(string.lower)
local getgenv_c = clonefunction_c(getgenv)
local getIdentity_c = clonefunction_c(getidentity)
local setIdentity_c = clonefunction_c(setidentity)
local rRequire = clonefunction_c(require)
local hookfunction_c = clonefunction_c(hookfunction)
local newcclosure_c = clonefunction_c(newcclosure)
local getrawmetatable_c = clonefunction_c(getrawmetatable)
local error_c = clonefunction_c(error)
local getnamecallmethod_c = clonefunction_c(getnamecallmethod)
local HttpPost_c = clonefunction_c(HttpPost)
local HttpGet_c = clonefunction_c(HttpGet)
local select_c = clonefunction_c(select)
local pairs_c = clonefunction_c(pairs)
local typeof_c = clonefunction_c(typeof)

print("setting genv")
getgenv_c().GetObjects = newcclosure_c(function(assetId)
	local oldId = getIdentity_c()
	setIdentity_c(8)
	local obj = insertservice_LoadLocalAsset(game_getservice(game, "InsertService"), assetId)
	setIdentity_c(oldId)
	return obj
end)
local GetObjects_c = clonefunction_c(getgenv_c().GetObjects)

getgenv_c().hookmetamethod = newcclosure_c(function(t, metamethod, fun)
	local mt = getrawmetatable_c(t)
	if not mt[metamethod] then
		error_c("hookmetamethod: No metamethod found with name " .. metamethod .. " in metatable.")
	end
	return hookfunction_c(mt[metamethod], fun)
end)
local hookmetamethod_c = clonefunction_c(getgenv_c().hookmetamethod)

getgenv_c().require = newcclosure_c(function(moduleScript)
	local old = getIdentity_c()
	setIdentity_c(2)
	local r = rRequire(moduleScript)
	setIdentity_c(old)
	return r
end)


getgenv_c().getnilinstances = newcclosure_c(function()
	local Instances = {}

	for _, Object in getreg() do
		if typeof_c(Object) == "Instance" and Object.Parent == nil then
			table.insert(Instances, Object)
		end
	end

	return Instances
end)

getgenv_c().getinstances = newcclosure_c(function()
	local Instances = {}

	for _, obj in getreg() do
		if obj and typeof_c(obj) == "Instance" then
			table.insert(Instances, obj)
		end
	end

	return Instances
end)

getgenv_c().getsenv = newcclosure_c(function(scr)
    if typeof(scr) ~= "Instance" then
        error("Expected script. Got ", typeof_c(script), " Instead.")
    end

    local Instances = {}

	for _, obj in getreg() do
		if obj and typeof_c(obj) == "function" and table.find(getfenv(obj), scr)  then
            return getfenv(obj)
		end
	end

	return Instances
end)

getgenv_c().getrunningscripts = newcclosure_c(function()
    local scripts = {}

	for _, obj in getreg() do
		if obj and typeof_c(obj) == "Instance" and obj:IsA("LocalScript") then
            table.insert(scripts, obj)
		end
	end

	return scripts
end)

getgenv_c().vsc_websocket = newcclosure_c(function()
	if not game:IsLoaded() then
		game.Loaded:Wait()
	end

	while task.wait(1) do
		local success, client = pcall(WebSocket.connect, "ws://localhost:33882/")
		if success then
			client.OnMessage:Connect(function(payload)
				local callback, exception = loadstring(payload)
				if exception then
					error(exception, 2)
				end

				task.spawn(callback)
			end)

			client.OnClose:Wait()
		end
	end
end)


local illegal = {
	"OpenVideosFolder",
	"OpenScreenshotsFolder",
	"GetRobuxBalance",
	"PerformPurchase",
	"PromptBundlePurchase",
	"PromptNativePurchase",
	"PromptProductPurchase",
	"PromptPurchase",
	"PromptThirdPartyPurchase",
	"Publish",
	"GetMessageId",
	"OpenBrowserWindow",
	"RequestInternal",
	"ExecuteJavaScript",
}

local oldNamecall
oldNamecall = hookmetamethod_c(
	game,
	"__namecall",
	newcclosure(function(...)
		if typeof_c(select_c(1, ...)) ~= "Instance" or not checkcaller_c() then
			return oldNamecall(...)
		end

		local namecallName = (getnamecallmethod_c())

		-- If we did a simple table find, as simple as a \0 at the end of the string would bypass our security.
		-- Unacceptable.
		for _, str in pairs_c(illegal) do
			if string_match(string_lower(namecallName), string_lower(str)) then
				return error_c("This function has been disabled for security reasons.")
			end
		end

		if namecallName == "HttpGetAsync" or namecallName == "HttpGet" then
			return HttpGet_c(select_c(2, ...)) -- 1 self, 2 arg (url)
		end

		if namecallName == "HttpPostAsync" or namecallName == "HttpPost" then
			return HttpPost_c(select_c(2, ...)) -- 1 self, 2 arg (url)
		end

		if namecallName == "GetObjects" then
			local a = select_c(2, ...)
			if typeof_c(a) ~= "table" and typeof_c(a) ~= "string" then
				return {}
			end
			return GetObjects_c(a) -- 1 self, 2 arg (table/string)
		end

		return oldNamecall(...)
	end)
)

local oldIndex
oldIndex = hookmetamethod_c(
	game,
	"__index",
	newcclosure(function(...)
		if not checkcaller_c() then
			return oldIndex(...)
		end
		if typeof_c(select_c(1, ...)) ~= "Instance" or typeof_c(select_c(2, ...)) ~= "string" then
			return oldIndex(...)
		end

		local self = select_c(1, ...)
		local idx = select_c(2, ...)

		-- If we did a simple table find, as simple as a \0 at the end of the string would bypass our security.
		-- Unacceptable.
		for _, str in pairs(illegal) do
			if string_match(idx, str) then
				return error_c("This function has been disabled for security reasons.")
			end
		end

		if idx == "HttpGetAsync" or idx == "HttpGet" then
			return clonefunction_c(HttpGet_c)
		end

		if idx == "HttpPostAsync" or idx == "HttpPost" then
			return clonefunction_c(HttpPost_c)
		end

		if idx == "GetObjects" then
			return clonefunction_c(GetObjects_c)
		end

		return oldIndex(...)
	end)
)
        )");
        // execution->lua_loadstring(L, str, utilities->RandomString(32),
        //                           static_cast<RBX::Identity>(RBX::Security::to_obfuscated_identity(
        //                                   static_cast<RBX::Lua::ExtraSpace *>(L->userdata)->identity)));
        // lua_pcall(L, 0, 0, 0);
        // RBX::Studio::Functions::rTask_defer(L);

        printf("[Environment::register_env] Attempting to initialize custom scheduler...\r\n");
        const std::string schedulerHook = (R"(
            game:GetService("RunService").Stepped:Connect(function()
                __SCHEDULER_STEPPED__HOOK()
            end)
        )");
        Scheduler::get_singleton()->schedule_job(schedulerHook);
        Scheduler::get_singleton()->scheduler_step(Scheduler::get_singleton()->get_global_executor_state());
        Scheduler::get_singleton()->schedule_job(initscript);
        Sleep(200);
    }

    return 0;
}
