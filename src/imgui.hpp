#pragma once

struct Window;
struct Graphics;

void init_imgui(Window *, Graphics *);
void end_imgui(Window *, Graphics *);

void imgui_begin_frame(Graphics *);
void imgui_render_frame(Graphics *);
