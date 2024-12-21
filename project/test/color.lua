-- color.lua
Color = {}
Color.__index = Color

-- Constructor for creating a Color object
function Color:new(r, g, b)
    local instance = setmetatable({}, Color)
    instance.r = r
    instance.g = g
    instance.b = b
    return instance
end

-- Utility function to create a single hexadecimal value
function Color:toHex(r, g, b)
    return (b << 16) | (g << 8) | r
end

-- Static hexadecimal color constants
Color.RED     = Color:toHex(0xFF, 0x00, 0x00)
Color.GREEN   = Color:toHex(0x00, 0xFF, 0x00)
Color.BLUE    = Color:toHex(0x00, 0x00, 0xFF)
Color.WHITE   = Color:toHex(0xFF, 0xFF, 0xFF)
Color.BLACK   = Color:toHex(0x00, 0x00, 0x00)
Color.YELLOW  = Color:toHex(0xFF, 0xFF, 0x00)
Color.CYAN    = Color:toHex(0x00, 0xFF, 0xFF)
Color.MAGENTA = Color:toHex(0xFF, 0x00, 0xFF)

-- Return the Color module
return Color
