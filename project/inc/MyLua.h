#pragma once

#include <sol/sol.hpp>

inline auto GetLua() -> sol::state&
{
    static sol::state m_Lua;
    return m_Lua;
}

template <typename... Args>
inline void CallLuaFunction(char const *functionName, Args&&... args)
{
    auto luaFunction = GetLua()[functionName];
    if (luaFunction.valid())
    {
        auto const result = luaFunction(std::forward<Args>(args)...);
        if (!result.valid())
        {
            const sol::error err = result;
            throw std::exception(err.what());
        }
    }
    else
    {
        throw std::exception("Function not found.");
    }
}
