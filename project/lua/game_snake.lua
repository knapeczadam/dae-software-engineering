require("color")
local math = require("math")
local os = require("os")
local table = require("table")

-- Constants
local TITLE  = "Snake Game"
local WIDTH  = 500
local HEIGHT = 500
local FRAME  = 10
local CELL_SIZE = 20

-- Snake and Game State
local snake = {}
local food = {x = 0, y = 0}
local direction = {x = 1, y = 0}
local game_over = false

-- Initialize game variables
function initialize()
    GAME_ENGINE:set_title(TITLE)
    GAME_ENGINE:set_width(WIDTH)
    GAME_ENGINE:set_height(HEIGHT)
    GAME_ENGINE:set_frame(FRAME)
    reset_game()
end

function start()

end

function on_destroy()

end

-- Reset the game
function reset_game()
    snake = {{x = 10, y = 10}}
    direction = {x = 1, y = 0}
    spawn_food()
    game_over = false
end

-- Spawn food at a random position
function spawn_food()
    math.randomseed(os.time())
    food.x = math.random(0, WIDTH // CELL_SIZE - 1) * CELL_SIZE
    food.y = math.random(0, HEIGHT // CELL_SIZE - 1) * CELL_SIZE
end

-- Check collision with walls or itself
function check_collision()
    local head = snake[1]
    -- Check wall collision
    if head.x < 0 or head.x >= WIDTH or head.y < 0 or head.y >= HEIGHT then
        return true
    end
    -- Check self-collision
    for i = 2, #snake do
        if head.x == snake[i].x and head.y == snake[i].y then
            return true
        end
    end
    return false
end

-- Update game state each tick
function tick()
    if game_over then
        return
    end

    -- Move the snake
    local new_head = {x = snake[1].x + direction.x * CELL_SIZE, y = snake[1].y + direction.y * CELL_SIZE}
    table.insert(snake, 1, new_head)

    -- Check if the snake eats the food
    if new_head.x == food.x and new_head.y == food.y then
        spawn_food()
    else
        -- Remove the tail
        table.remove(snake)
    end

    -- Check for collisions
    if check_collision() then
        game_over = true
    end
end

function mouse_button_action(isLeft, isDown, x, y, param)

end

function mouse_wheel_action(x, y, distance, param)

end

function mouse_move(x, y, param)

end

-- Draw the game
function paint(rect)
    GAME_ENGINE:fill_window_rect(Color.BLACK)

    -- Draw the snake
    for _, segment in ipairs(snake) do
        GAME_ENGINE:set_color(Color.GREEN)
        GAME_ENGINE:fill_rect(segment.x, segment.y, segment.x + CELL_SIZE, segment.y + CELL_SIZE)
    end

    -- Draw the food
    GAME_ENGINE:set_color(Color.RED)
    GAME_ENGINE:fill_rect(food.x, food.y, food.x + CELL_SIZE, food.y + CELL_SIZE)

    -- Draw Game Over text
    if game_over then
        GAME_ENGINE:draw_string("Game Over! Press R to Restart", WIDTH // 4, HEIGHT // 2)
    end
end

-- Handle keyboard input
function key_pressed(key)
end

function check_keyboard()
    if GAME_ENGINE:is_key_down(82) then
        reset_game()
        return
    end

    if GAME_ENGINE:is_key_down(87) and direction.y == 0 then
        direction = {x = 0, y = -1}
    elseif GAME_ENGINE:is_key_down(83) and direction.y == 0 then
        direction = {x = 0, y = 1}
    elseif GAME_ENGINE:is_key_down(65) and direction.x == 0 then
        direction = {x = -1, y = 0}
    elseif GAME_ENGINE:is_key_down(68) and direction.x == 0 then
        direction = {x = 1, y = 0}
    end
end
