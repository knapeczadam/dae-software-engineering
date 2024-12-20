local math = require("math")

-- Constants
local WIDTH = 500
local HEIGHT = 500
local FRAME = 50

local PADDLE_WIDTH = 100
local PADDLE_HEIGHT = 20
local BALL_SIZE = 10
local BRICK_ROWS = 5
local BRICK_COLUMNS = 8
local BRICK_WIDTH = 60
local BRICK_HEIGHT = 20
local BRICK_PADDING = 10

-- Game state
local paddle_x = (WIDTH - PADDLE_WIDTH) / 2
local paddle_y = HEIGHT - 30
local ball_x = WIDTH / 2
local ball_y = HEIGHT / 2
local ball_dx = 3
local ball_dy = -3
local bricks = {}
local score = 0
local game_over = false

function initialize()
    GAME_ENGINE:set_title("Breakout")
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()
    -- Create bricks
    for row = 1, BRICK_ROWS do
        bricks[row] = {}
        for col = 1, BRICK_COLUMNS do
            bricks[row][col] = true -- Brick is present
        end
    end
end

function end_()
end

function paint(rect)
    -- Clear the screen
    GAME_ENGINE:fill_window_rect(0)

    -- Draw the paddle
    GAME_ENGINE:set_color(0x0000FF) -- Blue
    GAME_ENGINE:fill_rect(math.tointeger(paddle_x), math.tointeger(paddle_y), math.tointeger(paddle_x + PADDLE_WIDTH), math.tointeger(paddle_y + PADDLE_HEIGHT))

    -- Draw the ball
    GAME_ENGINE:set_color(0xFF0000) -- Red
    GAME_ENGINE:fill_rect(math.tointeger(ball_x), math.tointeger(ball_y), math.tointeger(ball_x + BALL_SIZE), math.tointeger(ball_y + BALL_SIZE))

    -- Draw the bricks
    GAME_ENGINE:set_color(0x00FF00) -- Green
    for row = 1, BRICK_ROWS do
        for col = 1, BRICK_COLUMNS do
            if bricks[row][col] then
                local brick_x = (col - 1) * (BRICK_WIDTH + BRICK_PADDING)
                local brick_y = (row - 1) * (BRICK_HEIGHT + BRICK_PADDING)
                GAME_ENGINE:fill_rect(brick_x, brick_y, brick_x + BRICK_WIDTH, brick_y + BRICK_HEIGHT)
            end
        end
    end

    -- Draw the score
    GAME_ENGINE:set_color(0xFF00FF) -- White
    GAME_ENGINE:draw_string("Score: " .. score, 10, 10)

    -- Draw "Game Over" if necessary
    if game_over then
        GAME_ENGINE:draw_string("Game Over", math.tointeger(WIDTH / 2 - 50), math.tointeger(HEIGHT / 2))
    end
end

function tick()
    if game_over then return end

    -- Move the ball
    ball_x = ball_x + ball_dx
    ball_y = ball_y + ball_dy

    -- Ball collision with walls
    if ball_x < 0 or ball_x > WIDTH - BALL_SIZE then
        ball_dx = -ball_dx
    end
    if ball_y < 0 then
        ball_dy = -ball_dy
    end

    -- Ball collision with paddle
    if ball_y + BALL_SIZE >= paddle_y and
            ball_x + BALL_SIZE >= paddle_x and
            ball_x <= paddle_x + PADDLE_WIDTH then
        ball_dy = -ball_dy
    end

    -- Ball collision with bricks
    for row = 1, BRICK_ROWS do
        for col = 1, BRICK_COLUMNS do
            if bricks[row][col] then
                local brick_x = (col - 1) * (BRICK_WIDTH + BRICK_PADDING)
                local brick_y = (row - 1) * (BRICK_HEIGHT + BRICK_PADDING)
                if ball_x + BALL_SIZE > brick_x and ball_x < brick_x + BRICK_WIDTH and
                        ball_y + BALL_SIZE > brick_y and ball_y < brick_y + BRICK_HEIGHT then
                    bricks[row][col] = false -- Destroy the brick
                    ball_dy = -ball_dy
                    score = score + 1
                end
            end
        end
    end

    -- Ball falls below the paddle
    if ball_y > HEIGHT then
        game_over = true
    end
end

function mouse_button_action(isLeft, isDown, x, y, param)

end

function mouse_wheel_action(x, y, distance, param)

end

function mouse_move(x, y, param)

end

function check_keyboard()
    if GAME_ENGINE:is_key_down(65) then -- A key
        GAME_ENGINE:debug("A key is down")
        paddle_x = paddle_x - 5
    end
    if GAME_ENGINE:is_key_down(68) then -- D key
        paddle_x = paddle_x + 5
    end

    -- Ensure paddle stays within the screen
    if paddle_x < 0 then paddle_x = 0 end
    if paddle_x > WIDTH - PADDLE_WIDTH then paddle_x = WIDTH - PADDLE_WIDTH end
end

function key_pressed(key)
end

function call_action(caller)

end