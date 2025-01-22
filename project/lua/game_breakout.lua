local math = require("math")
require("event")
require("color")

-- Constants
local WIDTH  = 500
local HEIGHT = 500
local FRAME  =  50

local PADDLE_WIDTH  = 100
local PADDLE_HEIGHT =  20
local BALL_SIZE     =  10
local BRICK_ROWS    =   6
local BRICK_COLUMNS =  10
local BRICK_PADDING =  10
local BRICK_WIDTH   = (WIDTH - ((BRICK_COLUMNS + 1) * BRICK_PADDING)) // BRICK_COLUMNS
local BRICK_HEIGHT  = 20
local COLORS        = {0x4b50ba, 0x4271bb, 0x3c7dad, 0x3fa3a4, 0x519f5f, 0xC24641}

-- Game state
local paddle_x
local paddle_y
local ball_x
local ball_y
local ball_dx
local ball_dy
local paddle_dx
local bricks
local score
local game_over
local game_win

local audio = Audio.new("bump.wav")

function reset()
    paddle_x  = (WIDTH - PADDLE_WIDTH) // 2
    paddle_y  = HEIGHT - PADDLE_HEIGHT
    ball_x    = WIDTH // 2
    ball_y    = HEIGHT // 2
    ball_dx   =  5
    ball_dy   = -5
    paddle_dx = 10
    bricks    = {}
    score     = 0
    game_over = false
    game_win  = false

    -- Create bricks
    for row = 1, BRICK_ROWS do
        bricks[row] = {}
        for col = 1, BRICK_COLUMNS do
            bricks[row][col] = true -- Brick is present
        end
    end
end

function initialize()
    GAME_ENGINE:set_title("Breakout")
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
end

function start()
    reset()
end

function on_destroy()
end

function paint(rect)
    -- Clear the screen
    GAME_ENGINE:fill_window_rect(Color.BLACK)

    -- Draw the paddle
    GAME_ENGINE:set_color(0x4b50ba) -- Red
    GAME_ENGINE:fill_rect(paddle_x, paddle_y, paddle_x + PADDLE_WIDTH, paddle_y + PADDLE_HEIGHT)

    -- Draw the ball
    GAME_ENGINE:set_color(Color.WHITE)
    GAME_ENGINE:fill_rect(ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE)

    -- Draw the bricks
    for row = 1, BRICK_ROWS do
        GAME_ENGINE:set_color(COLORS[row])
        for col = 1, BRICK_COLUMNS do
            if bricks[row][col] then
                local brick_x = (col - 1) * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING
                local brick_y = (row - 1) * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING
                GAME_ENGINE:fill_rect(brick_x, brick_y, brick_x + BRICK_WIDTH, brick_y + BRICK_HEIGHT)
            end
        end
    end

    -- Draw the score
    GAME_ENGINE:set_color(Color.WHITE)
    --GAME_ENGINE:draw_string("Score: " .. score, 10, 10)

    -- Draw "Game Over" if necessary
    if game_over then
        GAME_ENGINE:draw_string("Game Over", math.tointeger(WIDTH / 2 - 50), math.tointeger(HEIGHT / 2))
        GAME_ENGINE:draw_string("Press 'R' to restart", math.tointeger(WIDTH / 2 - 70), math.tointeger(HEIGHT / 2 + 20))
    end

    -- Draw "You Win" if necessary
    if game_win then
        GAME_ENGINE:draw_string("You Win", math.tointeger(WIDTH / 2 - 30), math.tointeger(HEIGHT / 2))
        GAME_ENGINE:draw_string("Press 'R' to restart", math.tointeger(WIDTH / 2 - 70), math.tointeger(HEIGHT / 2 + 20))
    end
end

function tick()
    if game_over or game_win then return end

    audio:tick()

    -- Move the ball
    ball_x = ball_x + ball_dx
    ball_y = ball_y + ball_dy

    -- Ball collision with walls
    if ball_x <= 0 or ball_x >= WIDTH - BALL_SIZE then
        ball_dx = -ball_dx

        -- Adjust the ball position to prevent sticking
        if ball_x <= 0 then
            ball_x = 0
        else
            ball_x = WIDTH - BALL_SIZE
        end

        audio:stop()
        audio:play()
    end
    if ball_y <= 0 then
        ball_dy = -ball_dy
        -- Adjust the ball position to prevent sticking
        ball_y = 0

        audio:stop()
        audio:play()

    end

    -- Ball collision with paddle
    if ball_y + BALL_SIZE >= paddle_y and
            ball_x + BALL_SIZE >= paddle_x and
            ball_x <= paddle_x + PADDLE_WIDTH then

        -- Move the ball out of the paddle to prevent sticking
        ball_y = paddle_y - BALL_SIZE

        -- Reverse vertical direction
        ball_dy = -ball_dy

        -- Adjust horizontal direction based on hit position
        local paddle_center = paddle_x + PADDLE_WIDTH // 2
        local hit_position = (ball_x + BALL_SIZE // 2) - paddle_center
        local hit_ratio = hit_position // (PADDLE_WIDTH // 2)

        -- Scale ball_dx based on hit position
        ball_dx = ball_dx + hit_ratio * 2 -- Adjust the multiplier (2) for sensitivity
        audio:stop()
        audio:play()
    end

    -- Ball collision with bricks
    for row = 1, BRICK_ROWS do
        for col = 1, BRICK_COLUMNS do
            if bricks[row][col] then
                local brick_x = (col - 1) * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING
                local brick_y = (row - 1) * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_PADDING
                if ball_x + BALL_SIZE >= brick_x and ball_x <= brick_x + BRICK_WIDTH and
                        ball_y + BALL_SIZE >= brick_y and ball_y <= brick_y + BRICK_HEIGHT then
                    bricks[row][col] = false -- Destroy the brick
                    ball_dy = -ball_dy
                    score = score + 1
                    audio:stop()
                    audio:play()
                end
            end
        end
    end

    -- Ball falls below the paddle
    if ball_y > HEIGHT then
        game_over = true
    end

    -- Maximum points gained
    if score == BRICK_ROWS * BRICK_COLUMNS then
        game_win = true
    end
end

function mouse_button_action(isLeft, isDown, x, y, param)

end

function mouse_wheel_action(x, y, distance, param)

end

function mouse_move(x, y, param)

end

function check_keyboard()
    if GAME_ENGINE:is_key_down(KeyCode.A) or GAME_ENGINE:is_key_down(KeyCode.LeftArrow) then
        paddle_x = paddle_x - paddle_dx
    end
    if GAME_ENGINE:is_key_down(KeyCode.D) or GAME_ENGINE:is_key_down(KeyCode.RightArrow) then
        paddle_x = paddle_x + paddle_dx
    end
    if (game_over or game_win) and GAME_ENGINE:is_key_down(KeyCode.R) then
        reset()
    end

    -- Ensure paddle stays within the screen
    if paddle_x <= 0 then paddle_x = 0 end
    if paddle_x >= WIDTH - PADDLE_WIDTH then paddle_x = WIDTH - PADDLE_WIDTH end
end

function key_pressed(key)
end

function call_action(caller)

end