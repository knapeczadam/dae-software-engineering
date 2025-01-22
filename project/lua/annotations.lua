--- @meta
---
--- @class GameEngine
GameEngine = {}

--- @param title string
--- @return nil
function GameEngine:set_title(title) end

--- @param frame number
--- @return nil
function GameEngine:set_frame(frame) end

--- @param width number
--- @return nil
function GameEngine:set_width(width) end

--- @param height number
--- @return nil
function GameEngine:set_height(height) end

--- @param value boolean
--- @return nil
function GameEngine:show_mouse_pointer(value) end

--- @param key number
--- @return boolean
function GameEngine:is_key_down(key) end

--- @param message string
--- @return nil
function GameEngine:message_box(message) end

--- @param message string
--- @return nil
function GameEngine:message_continue(message) end

--- @param text string
--- @param font Font
--- @return Size
function GameEngine:calculate_text_dimensions(text, font) end

--- @param text string
--- @param font Font
--- @param rect Rect
--- @return Size
function GameEngine:calculate_text_dimensions(text, font, rect) end

--- @param color Color
--- @return nil
function GameEngine:set_color(color) end

--- @param font Font
--- @return nil
function GameEngine:set_font(font) end

--- @param color Color
--- @return boolean
function GameEngine:fill_window_rect(color) end

--- @param x1 number
--- @param y1 number
--- @param x2 number
--- @param y2 number
--- @return boolean
function GameEngine:draw_line(x1, y1, x2, y2) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @return boolean
function GameEngine:draw_rect(left, top, right, bottom) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @return boolean
function GameEngine:fill_rect(left, top, right, bottom) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param opacity number
--- @return boolean
function GameEngine:fill_rect(left, top, right, bottom, opacity) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param radius number
--- @return boolean
function GameEngine:draw_round_rect(left, top, right, bottom, radius) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param radius number
--- @return boolean
function GameEngine:fill_round_rect(left, top, right, bottom, radius) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @return boolean
function GameEngine:draw_oval(left, top, right, bottom) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @return boolean
function GameEngine:fill_oval(left, top, right, bottom) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param opacity number
--- @return boolean
function GameEngine:fill_oval(left, top, right, bottom, opacity) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param startDegree number
--- @param angle number
--- @return boolean
function GameEngine:draw_arc(left, top, right, bottom, startDegree, angle) end

--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @param startDegree number
--- @param angle number
function GameEngine:fill_arc(left, top, right, bottom, startDegree, angle) end

--- @param text string
--- @param left number
--- @param top number
--- @return number
function GameEngine:draw_string(text, left, top) end

--- @param text string
--- @param left number
--- @param top number
--- @param right number
--- @param bottom number
--- @return number
function GameEngine:draw_string(text, left, top, right, bottom) end

--- @param bitmap Bitmap
--- @param left number
--- @param top number
--- @return boolean
function GameEngine:draw_bitmap(bitmap, left, top) end

--- @param bitmap Bitmap
--- @param left number
--- @param top number
--- @param sourceRect Rect
--- @return boolean
function GameEngine:draw_bitmap(bitmap, left, top, sourceRect) end

--- @param ptsArr Point[]
--- @param count number
--- @return boolean
function GameEngine:draw_polygon(ptsArr, count) end

--- @param ptsArr Point[]
--- @param count number
--- @param close boolean
--- @return boolean
function GameEngine:draw_polygon(ptsArr, count, close) end

--- @param ptsArr Point[]
--- @param count number
--- @return boolean
function GameEngine:fill_polygon(ptsArr, count) end
--- @param ptsArr Point[]
--- @param count number
--- @param close boolean
--- @return boolean
function GameEngine:fill_polygon(ptsArr, count, close) end

--- @return Color
function GameEngine:get_draw_color() end

--- @param message string
--- @return nil
function GameEngine:debug(message) end

--- @type GameEngine
GAME_ENGINE = nil

--- @class Bitmap
Bitmap = {}

--- @param fileName string
--- @param createAlphaChannel boolean
--- @return Bitmap
function Bitmap.new(fileName, createAlphaChannel) end

--- @param color Color
--- @return nil
function Bitmap:set_transparency_color(color) end

--- @param opacity number
--- @return nil
function Bitmap:set_opacity(opacity) end

--- @return number
function Bitmap:get_width() end

--- @return number
function Bitmap:get_height() end

--- @class POINT
POINT = {}

--- @param x number
--- @param y number
--- @return POINT
function POINT.new(x, y) end

--- @class Audio
Audio = {}

--- @param fileName string
--- @return Bitmap
function Audio.new(fileName) end

--- @return nil
function Audio:tick() end

--- @return nil
function Audio:play()
    Audio:play(0, -1)
end
--- @param msec_start number
--- @param msec_stop number
--- @return nil
function Audio:play(msec_start, msec_stop) end

--- @return nil
function Audio:pause() end

--- @return nil
function Audio:stop() end
