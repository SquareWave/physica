#include <stdio.h>
#include <stdarg.h>

#include "typedefs.h"
#include "game.h"
#include "stb_truetype.h"

void print_debug_log() {
    char buffer[256];

    OutputDebugStringA("\n\n");
    for (int i = 0; i < max_debug_blocks; ++i) {
        if (debug_blocks[i].id) {
            sprintf(buffer, "%-32s %14lld cy,    %5d calls\n",
                   debug_blocks[i].id,
                   debug_blocks[i].total_cycles,
                   debug_blocks[i].call_count);
            OutputDebugStringA(buffer);
            debug_blocks[i].id = 0;
            debug_blocks[i].total_cycles = 0;
            debug_blocks[i].call_count = 0;
        }
    }
}

inline f32
debug_get_seconds_elapsed(u64 old, u64 current) {
    return ((f32)(current - old) / (f32)(SDL_GetPerformanceFrequency()));
}

void
debug_load_monospace_font(game_state_t* game_state) {
    game_state->debug_state.monospace_font =
        load_font("C:/Windows/fonts/arial.ttf", 20.0f);
}

void
debug_push_ui_text(game_state_t* game_state,
                   window_description_t window,
                   v2 bottom_left,
                   rgba_t color,
                   char* text) {
    TIMED_FUNC();

    v2 original_pos = bottom_left;

    bottom_left.y = (f32)window.height - bottom_left.y - 1;

    font_spec_t* font = &game_state->debug_state.monospace_font;
    tex2 texture = font->texture;

    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(font->baked_chars,
                               font->texture.width,
                               font->texture.height,
                               *text-32,
                               &bottom_left.x,
                               &bottom_left.y,
                               &q,
                               1);

            f32 qt_width = (q.s1 - q.s0) * texture.width;
            f32 qt_height = (q.t1 - q.t0) * texture.height;

            rect_i source_rect;
            source_rect.min_x = (i32)(q.s0 * (f32)texture.width);
            source_rect.max_x = (i32)(q.s1 * (f32)texture.width);
            source_rect.max_y = (i32)((1.0f - q.t0) * (f32)texture.height);
            source_rect.min_y = (i32)((1.0f - q.t1) * (f32)texture.height);

            v2 min;
            v2 max;
            min.x = q.x0;
            max.x = q.x1;
            max.y = window.height - q.y0 - 1;
            min.y = window.height - q.y1 - 1;

            v2 center = 0.5f * (max + min);
            v2 hotspot = v2 {
                0.5f * qt_width,
                0.5f * qt_height,
            };

            push_texture(&game_state->ui_render_group,
                         center,
                         hotspot,
                         1.0f,
                         texture,
                         source_rect,
                         color,
                         0.0f,
                         0.0f);
        }
        ++text;
    }

    // f32 current_left = 0.0f;
    // while (*text) {
    //     i32 c = (i32)*text;

    //     glyph_spec_t glyph = game_state->debug_state.monospace_font[c - 32];

    //     tex2 texture = glyph.texture;
    //     v2 hotspot = v2{0};

    //     f32 width = (f32)texture.width;
    //     f32 height = (f32)texture.height;

    //     push_texture(&game_state->ui_render_group,
    //                  bottom_left + v2{ current_left + glyph.left_side_bearing,0.0f},
    //                  hotspot,
    //                  1.0f,
    //                  texture,
    //                  rect_i {0,0,texture.width,texture.height},
    //                  color,
    //                  0.0f,
    //                  0.0f);

    //     current_left += glyph.advance_width;
    //     text++;
    // }
}

void debug_push_ui_text_f(game_state_t* game_state,
                          window_description_t window,
                          v2 bottom_left,
                          rgba_t color,
                          char* format,
                          ...) {
    TIMED_FUNC();
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, ARRAY_SIZE(buffer), format, args);
    debug_push_ui_text(game_state, window, bottom_left, color, buffer); 
    va_end(args);
}

void debug_draw_aabb_tree(game_state_t* game_state) {
    phy_aabb_tree_t* tree = &game_state->physics_state.aabb_tree;

    i32 stack[LARGE_STACK_SIZE] = {0};

    i32 stack_index = 0;
    stack[stack_index++] = tree->root;

    while (stack_index > 0) {
        assert(stack_index < ARRAY_SIZE(stack));

        phy_aabb_tree_node_t* node = tree->nodes.at(stack[--stack_index]);

        phy_aabb_t aabb = node->fat_aabb;
        v2 diagonal = aabb.max - aabb.min;
        v2 center = 0.5f * (aabb.max + aabb.min);

        color_t color = node->is_asleep ?
            color_t {0.5f,0.5f,0.5f} :
            color_t {0.9f, 0.4f, 0.2f};

        f32 z = node->is_asleep ? 0.01f : 0.0f;

        push_rect_outline(&game_state->main_render_group,
                          color,
                          center,
                          diagonal,
                          0.0f,
                          z);

        if (node->type != LEAF_NODE) {
            stack[stack_index++] = node->left;
            stack[stack_index++] = node->right;
        }
    }
}

void
debug_draw_hulls(game_state_t* game_state) {
    phy_state_t* physics = &game_state->physics_state;
    for (int i = 0; i < physics->bodies.size; ++i) {
        phy_body_t* body = physics->bodies.try_get(i);
        if (!body) {
            continue;
        }

        for (int j = 0; j < body->hulls.count; ++j) {
            phy_hull_t* hull = body->hulls.at(j);
            switch (hull->type) {
                case HULL_RECT: {
                    push_rect_outline(&game_state->main_render_group,
                              color_t {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f);
                } break;
                case HULL_FILLET_RECT: {
                    push_rect_outline(&game_state->main_render_group,
                              color_t {0.2f, 0.9f, 0.2f},
                              hull->position,
                              v2 {hull->width, hull->height},
                              hull->orientation,
                              0.0f);
                    f32 fillet = hull->fillet;
                    f32 inner_width = hull->width / 2.0f - fillet;
                    f32 inner_height = hull->height / 2.0f - fillet;
                    m2x2 rotation = get_rotation_matrix(hull->orientation);
                    // push_circle(&game_state->main_render_group,
                    //             color_t {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {inner_width, inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_t {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {inner_width, -inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_t {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {-inner_width, -inner_height},
                    //             hull->fillet,
                    //             0.0f);
                    // push_circle(&game_state->main_render_group,
                    //             color_t {0.9f, 0.2f, 0.2f},
                    //             hull->position + rotation * v2 {-inner_width, inner_height},
                    //             hull->fillet,
                    //             0.0f);
                } break;
            }
        }
    }
}

void debug_init(game_state_t* game_state) {
    debug_load_monospace_font(game_state);
}

void debug_update_and_render(game_state_t* game_state,
                             f32 dt,
                             window_description_t window,
                             game_input_t* game_input) {

    persist u64 last_counter = SDL_GetPerformanceCounter();
    f32 actual_dt = debug_get_seconds_elapsed(last_counter, SDL_GetPerformanceCounter());
    last_counter = SDL_GetPerformanceCounter();
    if (true) {
        rgba_t color;

        f32 fps = 1.0f / actual_dt;

        if (fps > (0.9f * FRAMES_PER_SECOND)) {
            color = COLOR_GREEN;
        } else if (fps > (0.75f * FRAMES_PER_SECOND)) {
            color = COLOR_ORANGE;
        } else {
            color = COLOR_RED;
        }
        
        debug_push_ui_text_f(game_state,
                             window,
                             v2 {20.0f, 20.0f},
                             color,
                             "%3.1f fps", fps);
    }

    if (game_input->mouse.left_click.transition_count &&
        game_input->mouse.left_click.ended_down) {
        m3x3 inverse_view_transform = get_inverse_view_transform_3x3(game_state->main_camera);
        v2 world_position = inverse_view_transform * game_input->mouse.normalized_position;
        game_state->debug_state.selected = pick_body(&game_state->physics_state,
                                                     world_position);
    }
    
    if (game_state->debug_state.selected) {
        push_circle(&game_state->main_render_group,
                    color_t {0.4f, 1.0f, 0.4f},
                    game_state->debug_state.selected->position,
                    2.0f * VIRTUAL_PIXEL_SIZE,
                    0.0f,
                    0);

        debug_push_ui_text_f(game_state,
                             window,
                             v2 {0.0f, 0.0f},
                             COLOR_RED,
                             "%lld", game_state->debug_state.selected->entity.id);
    }

    if (false) {
        debug_draw_hulls(game_state);
    }

    if (false) {
        debug_draw_aabb_tree(game_state);
    }
}