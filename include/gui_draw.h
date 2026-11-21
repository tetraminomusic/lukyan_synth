/**
 * @file gui_draw.h
 * @brief Низкоуровневые функции отрисовки 2D-графики.
 *
 * Содержит примитивы для рисования панелей, текста, кнопок и крутилок.
 */

#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Копирует фоновое изображение в основной буфер пикселей.
 * @param dst   Указатель на буфер-назначение.
 * @param src   Указатель на буфер-источник.
 * @param count Количество пикселей.
 */
void draw_copy_bg(uint32_t *dst, const uint32_t *src, int count);

/**
 * @brief Рисует прямоугольную панель с рамкой и полупрозрачным фоном.
 *
 * @param pixels       Указатель на буфер пикселей.
 * @param x, y, w, h   Координаты и размеры панели.
 * @param bg_color     Цвет фона в формате ARGB.
 * @param border_color Цвет рамки в формате ARGB.
 */
void draw_rect_panel(uint32_t *pixels, int x, int y, int w, int h, uint32_t bg_color, uint32_t border_color);

/**
 * @brief Рисует текстовую строку с поддержкой UTF-8.
 * @param pixels Указатель на буфер пикселей.
 * @param x, y   Координаты верхнего левого угла текста.
 * @param str    Строка в кодировке UTF-8.
 * @param color  Цвет текста в формате ARGB.
 */
void draw_text(uint32_t *pixels, int x, int y, const char *str, uint32_t color);

/**
 * @brief Рисует кнопку с текстовой меткой.
 * @param pixels      Указатель на буфер пикселей.
 * @param x, y, w, h  Координаты и размеры кнопки.
 * @param label       Текст на кнопке.
 * @param is_selected Выделена ли кнопка.
 */
void draw_button(uint32_t *pixels, int x, int y, int w, int h, const char *label, bool is_selected);

/**
 * @brief Рисует крутилку с индикатором и подписями.
 *
 * @param pixels    Указатель на буфер пикселей.
 * @param cx, cy    Координаты центра крутилки.
 * @param radius    Радиус крутилки.
 * @param norm_val  Нормализованное значение для угла индикатора.
 * @param name      Подпись.
 * @param val_str   Строка с текущим значением параметра.
 * @param is_active Активна ли крутилка в данный момент.
 */
void draw_knob(uint32_t *pixels, int cx, int cy, int radius, float norm_val, const char *name, const char *val_str, bool is_active);

#endif