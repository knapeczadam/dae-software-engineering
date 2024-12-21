#pragma once

#include <sol/sol.hpp>

inline auto GetLua() -> sol::state &
{
    // TODO: better to create a singleton, finer control
    static sol::state m_Lua;
    return m_Lua;
}

template<typename... Args>
inline void CallLuaFunction(char const *functionName, Args &&... args)
{
    auto luaFunction = GetLua()[functionName];
    if (luaFunction.valid())
    {
        auto const result = luaFunction(std::forward<Args>(args)...);
        if (!result.valid())
        {
            const sol::error err = result;
            std::cerr << "[ERROR] Lua function '" << functionName << "' failed: " << err.what() << std::endl;
            std::cin.get();
        }
    }
    else
    {
        std::cerr << "[ERROR] Lua function '" << functionName << "' not found." << std::endl;
        std::cin.get();
    }
}

inline void HookLuaDebug(lua_State *L)
{
    lua_sethook(L, [](lua_State *L, lua_Debug *ar)
    {
        lua_getinfo(L, "nSl", ar); // Get detailed info about the current execution point

        // Print file name and line number
        std::cout << "Hook: " << ar->short_src << ":" << ar->currentline << std::endl;

        // Print the function name (if available)
        if (ar->name)
        {
            std::cout << "Function name: " << ar->name << std::endl;
        }
        else
        {
            std::cout << "Function name: (unknown)" << std::endl;
        }

        // Print the Lua stack
        std::cout << "Stack contents:" << std::endl;
        int stackSize = lua_gettop(L); // Get the number of elements in the stack
        for (int i = 1; i <= stackSize; ++i)
        {
            int t = lua_type(L, i);
            switch (t)
            {
                case LUA_TSTRING: // String
                    std::cout << i << ": " << lua_tostring(L, i) << " (string)" << std::endl;
                    break;
                case LUA_TBOOLEAN: // Boolean
                    std::cout << i << ": " << (lua_toboolean(L, i) ? "true" : "false") << " (boolean)" << std::endl;
                    break;
                case LUA_TNUMBER: // Number
                    std::cout << i << ": " << lua_tonumber(L, i) << " (number)" << std::endl;
                    break;
                default: // Other types
                    std::cout << i << ": " << lua_typename(L, t) << std::endl;
                    break;
            }
        }

        std::cout << std::endl;
    }, LUA_MASKLINE, 0);
}