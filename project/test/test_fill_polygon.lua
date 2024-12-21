require("color")

-- Constants
local TITLE  = "Core"
local WIDTH  = 500
local HEIGHT = 500
local FRAME  = 50

local bitmap = Bitmap.new("test.bmp", true)

function initialize()
    GAME_ENGINE:set_title(TITLE)
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()

end

function on_destroy()

end

points = {}

local p1 = POINT.new(0, 200)
local p2 = POINT.new(300, 0)
local p3 = POINT.new(300, 200)
local p4 = POINT.new(500, 500)

points[1] = p1
points[2] = p2
points[3] = p3
points[4] = p4

function paint(rect)
    GAME_ENGINE:fill_window_rect(Color.BLACK)
    GAME_ENGINE:set_color(Color.YELLOW)
    GAME_ENGINE:fill_polygon(points, #points)
end

function tick()

end

function mouse_button_action(isLeft, isDown, x, y, param)

end

function mouse_wheel_action(x, y, distance, param)

end

function mouse_move(x, y, param)

end

function check_keyboard()

end

function key_pressed(key)

end

function call_action(caller)

end
