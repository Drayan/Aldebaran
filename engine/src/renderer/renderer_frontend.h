#pragma once

#include "defines.h"

#include "renderer_types.inl"

struct static_mesh_data;
struct platform_state;

b8 initialize_renderer(const char *application_name, struct platform_state *plat_state);
void shutdown_renderer();

void renderer_on_resize(u16 width, u16 height);

b8 renderer_draw_frame(render_packet *packet);
