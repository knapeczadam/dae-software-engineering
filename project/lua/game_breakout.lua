local WIDTH = 500
local HEIGHT = 500
local FRAME = 50
local x = 0
local y = 0

function initialize()
    GAME_ENGINE:set_title("Breakout")
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()
    GAME_ENGINE:message_box("Hello, Breakout!")
end

function end_()
end

function paint(rect)
    GAME_ENGINE:fill_window_rect(0)
    GAME_ENGINE:set_color(255, 255, 255)
    GAME_ENGINE:draw_line(0, 0, WIDTH, HEIGHT)
    GAME_ENGINE:draw_rect(x, y, WIDTH, HEIGHT)
end

function tick()
    x = x + 1
    y = y + 1
    if x > WIDTH then
        x = 0
    end
    if y > HEIGHT then
        y = 0
    end
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
