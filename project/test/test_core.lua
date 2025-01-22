require("color")

-- Constants
local TITLE  = "Core"
local WIDTH  = 500
local HEIGHT = 500
local FRAME  = 50

function initialize()
    print("Initialize")
    GAME_ENGINE:set_title(TITLE)
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()
    print("Start")
end

function on_destroy()
    print("Destroy")
end

function paint(rect)
    GAME_ENGINE:fill_window_rect(Color.BLACK)
end

function tick()

end

function mouse_button_action(isLeft, isDown, x, y, param)
    print("Mouse button: isLeft=" .. tostring(isLeft) .. ", isDown=" .. tostring(isDown) .. ", x=" .. x .. ", y=" .. y)
end

function mouse_wheel_action(x, y, distance, param)
    print("Mouse wheel: x=" .. x .. ", y=" .. y .. ", distance=" .. distance)
end

function mouse_move(x, y, param)
    print("Mouse move: x=" .. x .. ", y=" .. y)
end

function check_keyboard()

end

function key_pressed(key)
    print("Key pressed: " .. key)
end

function call_action(caller)

end
