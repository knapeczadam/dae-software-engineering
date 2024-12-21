require("color")

-- Constants
local TITLE  = "Core"
local WIDTH  = 500
local HEIGHT = 500
local FRAME  = 50

function initialize()
    GAME_ENGINE:set_title(TITLE)
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()

end

function end_()

end

function paint(rect)
    GAME_ENGINE:fill_window_rect(Color.BLACK)
    GAME_ENGINE:set_color(Color.YELLOW)
    GAME_ENGINE:fill_round_rect(10, 10, 100, 100, 10)
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
