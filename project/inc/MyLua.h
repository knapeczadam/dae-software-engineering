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
