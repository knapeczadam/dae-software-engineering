local math = require("math")
require("event")
require("color")

-- Constants
local WIDTH  = 500
local HEIGHT = 300
local FRAME  =  50

local PADDLE_WIDTH  = 10
local PADDLE_HEIGHT = 60
local BALL_SIZE     = 10

local PADDLE_SPEED = 8
local BALL_SPEED   = 5

-- Game state
local paddle1_y
local paddle2_y
local ball_x
local ball_y
local ball_dx
local ball_dy
local score1
local score2
local game_over

-- Audio
local audio = Audio.new("bump.wav")

function reset()
    -- Reset paddle positions
    paddle1_y = (HEIGHT - PADDLE_HEIGHT) // 2
    paddle2_y = (HEIGHT - PADDLE_HEIGHT) // 2

    -- Reset ball position and direction
    ball_x = WIDTH // 2
    ball_y = HEIGHT // 2
    ball_dx = BALL_SPEED * (math.random(2) == 1 and 1 or -1) -- Randomize initial direction
    ball_dy = BALL_SPEED * (math.random(2) == 1 and 1 or -1) -- Randomize initial direction

    -- Reset scores
    score1 = 0
    score2 = 0

    game_over = false
end

function initialize()
    GAME_ENGINE:set_title("Pong")
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

    -- Draw the paddles
    GAME_ENGINE:set_color(Color.RED)
    GAME_ENGINE:fill_rect(10, paddle1_y, 10 + PADDLE_WIDTH, paddle1_y + PADDLE_HEIGHT)
    GAME_ENGINE:set_color(Color.BLUE)
    GAME_ENGINE:fill_rect(WIDTH - 10 - PADDLE_WIDTH, paddle2_y, WIDTH - 10, paddle2_y + PADDLE_HEIGHT)

    -- Draw the ball
    GAME_ENGINE:set_color(Color.WHITE)
    GAME_ENGINE:fill_rect(ball_x, ball_y, ball_x + BALL_SIZE, ball_y + BALL_SIZE)

    -- Draw the score
    GAME_ENGINE:draw_string("Player 1: " .. score1, 10, 10)
    GAME_ENGINE:draw_string("Player 2: " .. score2, WIDTH - 110, 10)

    -- Draw "Game Over" if necessary
    if game_over then
        local winner = score1 > score2 and "Player 1" or "Player 2"
        GAME_ENGINE:draw_string(winner .. " Wins!", WIDTH // 2 - 50, HEIGHT // 2)
        GAME_ENGINE:draw_string("Press 'R' to restart", WIDTH // 2 - 70, HEIGHT // 2 + 20)
    end
end

function tick()
    if game_over then return end

    audio:tick()

    -- Move the ball
    ball_x = ball_x + ball_dx
    ball_y = ball_y + ball_dy

    -- Ball collision with top and bottom walls
    if ball_y <= 0 or ball_y >= HEIGHT - BALL_SIZE then
        ball_dy = -ball_dy
        audio:stop()
        audio:play()
    end

    -- Ball collision with paddles
    if ball_x <= 10 + PADDLE_WIDTH and
            ball_y + BALL_SIZE >= paddle1_y and
            ball_y <= paddle1_y + PADDLE_HEIGHT then
        ball_dx = -ball_dx
        ball_x = 10 + PADDLE_WIDTH -- Move the ball out of the paddle
        audio:stop()
        audio:play()
    elseif ball_x >= WIDTH - 10 - PADDLE_WIDTH - BALL_SIZE and
            ball_y + BALL_SIZE >= paddle2_y and
            ball_y <= paddle2_y + PADDLE_HEIGHT then
        ball_dx = -ball_dx
        ball_x = WIDTH - 10 - PADDLE_WIDTH - BALL_SIZE -- Move the ball out of the paddle
        audio:stop()
        audio:play()
    end

    -- Ball out of bounds
    if ball_x < 0 then
        score2 = score2 + 1
        reset_ball()
    elseif ball_x > WIDTH then
        score1 = score1 + 1
        reset_ball()
    end

    -- Check for game over (optional rule: first to 10 points wins)
    if score1 >= 10 or score2 >= 10 then
        game_over = true
    end
end

function reset_ball()
    ball_x = WIDTH // 2
    ball_y = HEIGHT // 2
    ball_dx = BALL_SPEED * (math.random(2) == 1 and 1 or -1)
    ball_dy = BALL_SPEED * (math.random(2) == 1 and 1 or -1)
end

function check_keyboard()
    -- Player 1 controls (W/S)
    if GAME_ENGINE:is_key_down(KeyCode.W) then
        paddle1_y = paddle1_y - PADDLE_SPEED
    elseif GAME_ENGINE:is_key_down(KeyCode.S) then
        paddle1_y = paddle1_y + PADDLE_SPEED
    end

    -- Player 2 controls (Up/Down)
    if GAME_ENGINE:is_key_down(KeyCode.UpArrow) then
        paddle2_y = paddle2_y - PADDLE_SPEED
    elseif GAME_ENGINE:is_key_down(KeyCode.DownArrow) then
        paddle2_y = paddle2_y + PADDLE_SPEED
    end

    -- Restart the game
    if game_over and GAME_ENGINE:is_key_down(KeyCode.R) then
        reset()
    end

    -- Ensure paddles stay within the screen
    if paddle1_y < 0 then paddle1_y = 0 end
    if paddle1_y > HEIGHT - PADDLE_HEIGHT then paddle1_y = HEIGHT - PADDLE_HEIGHT end
    if paddle2_y < 0 then paddle2_y = 0 end
    if paddle2_y > HEIGHT - PADDLE_HEIGHT then paddle2_y = HEIGHT - PADDLE_HEIGHT end
end

function key_pressed(key)
end

function mouse_button_action(isLeft, isDown, x, y, param)
end

function mouse_wheel_action(x, y, distance, param)
end

function mouse_move(x, y, param)
end
