#pragma once

#include <sol/sol.hpp>

inline auto GetLua() -> sol::state&
{
    static sol::state m_Lua;
    return m_Lua;
}
