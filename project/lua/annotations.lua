Game = {}


GameEngine = {}

function GameEngine:set_title(title) end
function GameEngine:set_window_position(left, top) end
function GameEngine:set_window_region(region) end
function GameEngine:set_key_list(keyList) end
function GameEngine:set_frame(frame) end
function GameEngine:set_width(width) end
function GameEngine:set_height(height) end

function GameEngine:go_fullscreen() end
function GameEngine:go_window_mode() end
function GameEngine:show_mouse_pointer(value) end
function GameEngine:quit() end

function GameEngine:has_window_region() end
function GameEngine:is_fullscreen() end

function GameEngine:is_key_down(key) end

function GameEngine:message_box(message) end
function GameEngine:message_continue(message) end

function GameEngine:calculate_text_dimensions(text, font) end
function GameEngine:calculate_text_dimensions(text, font, rect) end

function GameEngine:set_color(color) end
function GameEngine:set_font(font) end

function GameEngine:fill_window_rect(color) end

function GameEngine:draw_line(x1, y1, x2, y2) end

function GameEngine:draw_rect(left, top, right, bottom) end
function GameEngine:fill_rect(left, top, right, bottom) end
function GameEngine:fill_rect(left, top, right, bottom, opacity) end
function GameEngine:draw_round_rect(left, top, right, bottom, radius) end
function GameEngine:fill_round_rect(left, top, right, bottom, radius) end
function GameEngine:draw_oval(left, top, right, bottom) end
function GameEngine:fill_oval(left, top, right, bottom) end
function GameEngine:fill_oval(left, top, right, bottom, opacity) end
function GameEngine:draw_arc(left, top, right, bottom, startDegree, angle) end
function GameEngine:fill_arc(left, top, right, bottom, startDegree, angle) end

function GameEngine:draw_string(text, left, top) end
function GameEngine:draw_string(text, left, top, right, bottom) end

function GameEngine:draw_bitmap(bitmap, left, top) end
function GameEngine:draw_bitmap(bitmap, left, top, sourceRect) end

function GameEngine:draw_polygon(ptsArr, count) end
function GameEngine:draw_polygon(ptsArr, count, close) end
function GameEngine:fill_polygon(ptsArr, count) end
function GameEngine:fill_polygon(ptsArr, count, close) end

function GameEngine:get_draw_color() end
function GameEngine:repaint() end

function GameEngine:get_title() end
function GameEngine:get_instance() end
function GameEngine:get_window() end
function GameEngine:get_width() end
function GameEngine:get_height() end
function GameEngine:get_frame_rate() end
function GameEngine:get_frame_delay() end
function GameEngine:get_window_position() end

function GameEngine:tab_next(child_window) end
function GameEngine:tab_previous(child_window) end

function GameEngine:debug(message) end

GAME_ENGINE = nil

