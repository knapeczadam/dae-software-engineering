local math = require("math")
local os = require("os")
local table = require("table")
require("event")

-- Constants
local WIDTH = 500
local HEIGHT = 500
local FRAME = 10

local CELL_SIZE = 20
local GRID_WIDTH = WIDTH // CELL_SIZE
local GRID_HEIGHT = HEIGHT // CELL_SIZE

-- Directions
local UP = {x = 0, y = -1}
local DOWN = {x = 0, y = 1}
local LEFT = {x = -1, y = 0}
local RIGHT = {x = 1, y = 0}

-- Game state
local snake
local direction
local food
local score
local game_over

function reset()
    -- Initialize the snake
    snake = {
        {x = GRID_WIDTH // 2, y = GRID_HEIGHT // 2}, -- Head
        {x = GRID_WIDTH // 2 - 1, y = GRID_HEIGHT // 2},
        {x = GRID_WIDTH // 2 - 2, y = GRID_HEIGHT // 2}
    }

    -- Initial direction
    direction = RIGHT

    -- Place initial food
    place_food()

    -- Reset score and game state
    score = 0
    game_over = false
end

function initialize()
    GAME_ENGINE:set_title("Snake")
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
    GAME_ENGINE:fill_window_rect(0)

    -- Draw the snake
    GAME_ENGINE:set_color(0x00FF00) -- Green
    for _, segment in ipairs(snake) do
        GAME_ENGINE:fill_rect(
            segment.x * CELL_SIZE,
            segment.y * CELL_SIZE,
            (segment.x + 1) * CELL_SIZE,
            (segment.y + 1) * CELL_SIZE
        )
    end

    -- Draw the food
    GAME_ENGINE:set_color(0xFF0000) -- Red
    GAME_ENGINE:fill_rect(
        food.x * CELL_SIZE,
        food.y * CELL_SIZE,
        (food.x + 1) * CELL_SIZE,
        (food.y + 1) * CELL_SIZE
    )

    -- Draw the score
    GAME_ENGINE:set_color(0xFFFFFF) -- White
    GAME_ENGINE:draw_string("Score: " .. score, 10, 10)

    -- Draw "Game Over" if necessary
    if game_over then
        GAME_ENGINE:draw_string("Game Over", WIDTH // 2 - 50, HEIGHT // 2)
        GAME_ENGINE:draw_string("Press 'R' to restart", WIDTH // 2 - 70, HEIGHT // 2 + 20)
    end
end

function tick()
    if game_over then return end

    -- Move the snake by adding a new head
    local head = snake[1]
    local new_head = {
        x = head.x + direction.x,
        y = head.y + direction.y
    }

    -- Check for collisions
    if check_collision(new_head) then
        game_over = true
        return
    end

    -- Add the new head
    table.insert(snake, 1, new_head)

    -- Check if the snake eats the food
    if new_head.x == food.x and new_head.y == food.y then
        score = score + 1
        place_food()
    else
        -- Remove the tail if no food is eaten
        table.remove(snake)
    end
end

function place_food()
    while true do
        local x = math.random(0, GRID_WIDTH - 1)
        local y = math.random(0, GRID_HEIGHT - 1)
        local food_position = {x = x, y = y}

        if not is_snake(food_position) then
            food = food_position
            break
        end
    end
end

function check_collision(position)
    -- Check for wall collision
    if position.x < 0 or position.x >= GRID_WIDTH or position.y < 0 or position.y >= GRID_HEIGHT then
        return true
    end

    -- Check for self-collision
    return is_snake(position)
end

function is_snake(position)
    for _, segment in ipairs(snake) do
        if segment.x == position.x and segment.y == position.y then
            return true
        end
    end
    return false
end

function check_keyboard()
    -- Change direction based on key press
    if GAME_ENGINE:is_key_down(KeyCode.UpArrow) and direction ~= DOWN then
        direction = UP
    elseif GAME_ENGINE:is_key_down(KeyCode.DownArrow) and direction ~= UP then
        direction = DOWN
    elseif GAME_ENGINE:is_key_down(KeyCode.LeftArrow) and direction ~= RIGHT then
        direction = LEFT
    elseif GAME_ENGINE:is_key_down(KeyCode.RightArrow) and direction ~= LEFT then
        direction = RIGHT
    end

    -- Restart the game
    if game_over and GAME_ENGINE:is_key_down(KeyCode.R) then
        reset()
    end
end

function key_pressed(key)
end

function mouse_button_action(isLeft, isDown, x, y, param)
end

function mouse_wheel_action(x, y, distance, param)
end

function mouse_move(x, y, param)
end
