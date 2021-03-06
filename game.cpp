//
// Created by doug on 4/16/15.
//

#include <cfloat>
#include "game.h"
#include "renderer.h"
#include "physica.h"
#include "player.h"
#include "tile.h"
#include "wiz_animation.h"
#include "background.h"
#include "animations.h"
#include "random.h"
#include "enemies.h"
#include "npcs.h"

void
initialize_render_arena(game_state_* game_state, window_description_ window) {
    game_state->gl_programs = load_programs();

    rgba_ lighting = to_rgba(0xff818dba);
    // rgba_ lighting = (1.0f / 255.0f) * rgba_ {237.0f,222.0f,213.0f,255.0f};
    // rgba_ lighting = (1.0f / 255.0f) * rgba_ {255.0f,255.0f,255.0f,255.0f};

    const i32 max_render_objects = 70000;
    game_state->main_render_group.objects.count = 0;
    game_state->main_render_group.objects.capacity = max_render_objects;
    game_state->main_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_render_objects, render_object_);
    game_state->main_render_group.frame_buffer =
        create_frame_buffer(window.width, window.height);
    game_state->main_render_group.lighting = lighting;

    const i32 max_ui_objects = 10000;
    game_state->ui_render_group.objects.count = 0;
    game_state->ui_render_group.objects.capacity = max_ui_objects;
    game_state->ui_render_group.objects.values =
        PUSH_ARRAY(&game_state->render_arena, max_ui_objects, render_object_);
    game_state->ui_render_group.frame_buffer =
        game_state->main_render_group.frame_buffer;
    game_state->ui_render_group.lighting = to_rgba(0xffffffff);


    glClearColor(0.784f * lighting.r, 0.8745f * lighting.g, 0.925f * lighting.b, 1.0f);

    const i32 max_animations = 1024 * 6;
    game_state->main_animation_group.animations.count = 0;
    game_state->main_animation_group.animations.capacity = max_animations;
    game_state->main_animation_group.animations.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, animation_);

    game_state->main_animation_group.freed.count = 0;
    game_state->main_animation_group.freed.capacity = max_animations;
    game_state->main_animation_group.freed.values =
        PUSH_ARRAY(&game_state->render_arena, max_animations, i32);

    const i32 max_animation_frames = 1024 * 36;
    game_state->animation_frames.count = 0;
    game_state->animation_frames.capacity = max_animation_frames;
    game_state->animation_frames.values =
        PUSH_ARRAY(&game_state->render_arena, max_animation_frames, animation_frame_);

    game_state->wiz_bmp = load_wiz_bmp();
    game_state->wiz_walking_right = wiz_walking_right(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_walking_left = wiz_walking_left(&game_state->animation_frames,
                                                    game_state->wiz_bmp);
    game_state->wiz_standing_right = wiz_standing_right(&game_state->animation_frames,
                                                        game_state->wiz_bmp);
    game_state->wiz_standing_left = wiz_standing_left(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_jumping_right = wiz_jumping_right(&game_state->animation_frames,
                                                      game_state->wiz_bmp);
    game_state->wiz_jumping_left = wiz_jumping_left(&game_state->animation_frames,
                                                    game_state->wiz_bmp);

    game_state->animations = load_animations(&game_state->render_arena);

    game_state->terrain_1 = load_image("assets/terrain_1.png");
}

void
setup_world(game_state_ *game_state) {

    const char* tile_map =
    "                                                                                                     *"
    "                 p                                                                                    "
    "                  pppp                                                                                "
    "               pp    p                                                                                "
    "                p pp                                                                                  "
    "                                                                                                      "
    "             p   ppp                                                                                  "
    "                                                                                                      "
    "                p p                                                                                   "
    "                                                                                                      "
    "               p                                                                                      "
    "                    p                                                                                 "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                                                                                                      "
    "                s                                                                                     "
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "       p  p     MMMMMMMMMMMMMM########################################################################"
    "      pp   p                  # M#####################################################################"
    "                              # M#####################################################################"
    "   ggggggggm          #         M#####################################################################"
    "#################  ##   ##   ## M#####################################################################"
    "#################MM##MMM##MMM## M#####################################################################"
    "############################### M#####################################################################"
    "############################### M#####################################################################"
    "#############>        ##MM##### M#####################################################################"
    "############## ####M  ##  ##    M#####################################################################"
    "############## ####M       #    M#####################################################################"
    "###################M          + M#####################################################################"
    "####################MMMMMMMMMM########################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################"
    "######################################################################################################";

    i32 tile_map_width = i32(strchr(tile_map, '*') - tile_map + 1);
    i32 tile_map_height = ((i32)strlen(tile_map)) / tile_map_width;

    for (int y = 0; y < tile_map_height; ++y) {
        for (int x = 0; x < tile_map_width; ++x) {
            f32 tile_size = 1.0f;
            v2 position = v2 {
                (tile_size / 2) + (x * tile_size),
                (tile_size / 2) + (-(y + 1) * tile_size)
            };

            char c = tile_map[y * tile_map_width + x];
            switch(c) {
                case '#': {
                    tile_info_ info;
                    info.tex_coord_x = random_i32(0, 8);
                    info.tex_coord_y = 0;
                    if (y && (tile_map[(y-1) * tile_map_width + x] == '#')) {
                        info.tex_coord_y = 1;
                    }
                    create_tile(game_state, position, info);
                } break;
                case 'M': {
                    i32 direction;
                    if (tile_map[(y-1) * tile_map_width + x] == ' ') {
                        direction = 0;
                    } else if (tile_map[(y+1) * tile_map_width + x] == ' ') {
                        direction = 2;
                    } else if (tile_map[y * tile_map_width + (x + 1)] == ' ') {
                        direction = 1;
                    } else if (tile_map[y * tile_map_width + (x - 1)] == ' ') {
                        direction = 3;
                    } else {
                        assert_(false);
                    }
                    create_spikes(game_state, position, direction);
                } break;
                case '+': {
                    create_save_point(game_state, position);
                } break;
                case 's': {
                    create_save_point(game_state, position);
                    create_player(game_state, position);
                } break;
                case '<': {
                    v2 turret_direction = v2 {-1.0f, 0.0f};
                    create_turret(game_state, position, turret_direction);
                } break;
                case '>': {
                    v2 turret_direction = v2 {1.0f, 0.0f};
                    create_turret(game_state, position, turret_direction);
                } break;
                case '^': {
                    v2 turret_direction = v2 {0.0f, 1.0f};
                    create_turret(game_state, position, turret_direction);
                } break;
                case 'v': {
                    v2 turret_direction = v2 {0.0f, -1.0f};
                    create_turret(game_state, position, turret_direction);
                } break; break;
                case 'p': {
                    const v2 tile_diagonal = v2 {1.0f, 1.0f};
                    const f32 tile_mass = 10.0f;

                    create_fillet_block_entity(game_state,
                                               TILE,
                                               position,
                                               tile_diagonal,
                                               0.15f,
                                               tile_mass,
                                               0.3f,
                                               0);
                } break;
                case 'g': {
                    create_lilguy(game_state, position, 0);
                } break;
                case 'm': {
                    create_lilguy(game_state, position, LILGUY_MAYOR);
                } break;
            }
        }
    }

    game_state->gravity_normal = v2 {0.0f, -1.0f};
    game_state->gravity_magnitude = BASE_GRAVITY_MAGNITUDE;

    game_state->rotation_state.target_direction = DIR_DOWN;
    game_state->rotation_state.current_direction = DIR_DOWN;
    game_state->rotation_state.progress = 1.0f;
}

void
initialize_game_state(game_state_* game_state, window_description_ window) {
    // i64 memory_index = 0;
    u8* memory_location = (u8*)game_state + sizeof(game_state_);

    #define __MAKE_ARENA(arena, count) do {\
        arena.size = (count);\
        arena.base = memory_location;\
        memory_location += (count);\
    } while (0)

    __MAKE_ARENA(game_state->world_arena, 1024L * 1024L * 64L);
    __MAKE_ARENA(game_state->render_arena, 1024L * 1024L * 64L);

    initialize_render_arena(game_state, window);

    f32 camera_height_meters =
        (window.height /
        (f32)uround((window.height / TARGET_VIEW_HEIGHT_IN_METERS) * VIRTUAL_PIXEL_SIZE)) *
        VIRTUAL_PIXEL_SIZE;
    game_state->main_camera.center = v2 {0.0f, 0.0f};
    f32 aspect_ratio = (f32)window.width / (f32)window.height;
    game_state->main_camera.to_top_right = v2 {
        camera_height_meters * 0.5f * aspect_ratio,
        camera_height_meters * 0.5f
    };
    game_state->main_camera.orientation = 0.0f;
    game_state->main_camera.zoom.factor = 1.0f;

    v2 half_window_dim = v2 {(f32)window.width / 2.0f, (f32)window.height / 2.0f};
    game_state->ui_camera.center = half_window_dim;
    game_state->ui_camera.to_top_right = half_window_dim;
    game_state->ui_camera.orientation = 0.0f;
    game_state->ui_camera.zoom.factor = 1.0f;

    game_state->physics_state = phy_init(&game_state->world_arena);

    const i32 entity_capacity = 4000;
    game_state->entities.allocate(&game_state->world_arena, entity_capacity);
    game_state->next_entity_id = 1L;

    const i32 entity_map_capacity = 8000;
    game_state->entity_map.pairs.values = PUSH_ARRAY(&game_state->world_arena,
                                                        entity_map_capacity,
                                                        hashpair<sim_entity_*>);
    game_state->entity_map.pairs.count = entity_map_capacity;

    const i32 collision_capacity = 2000;
    game_state->collision_map.pairs.values = PUSH_ARRAY(&game_state->world_arena,
                                                        collision_capacity,
                                                        hashpair<entity_ties_>);
    game_state->collision_map.pairs.count = collision_capacity;

    setup_world(game_state);

    create_background(game_state, &game_state->background);


    game_state->initialized = true;
}

void
game_update_and_render(platform_services_ platform,
                       game_state_* game_state,
                       transient_state_* transient_state,
                       f32 dt,
                       window_description_ window,
                       game_input_* game_input,
                       tools_state_* tools_state) {

    TIMED_FUNC();

    if (!game_state->initialized) {
        initialize_game_state(game_state, window);

        tools_init(tools_state);
    }

    if (!game_state->paused || game_state->advance_one_frame) {
        clear_hashmap(&game_state->collision_map);

        phy_set_gravity(&game_state->physics_state, 
                        game_state->gravity_magnitude * game_state->gravity_normal);

        phy_update(&game_state->physics_state, &game_state->collision_map, dt);

        for (int i = 0; i < game_state->entities.size; ++i) {
			TIMED_BLOCK(update_entities);
            sim_entity_* entity = game_state->entities.try_get(i);

            if (!entity) {
                continue;
            }

            #define __UPDATE_CASE(type) case type: {\
                update_##type(game_state, game_input, entity, dt);\
            } break

            #define __EMPTY_CASE(type) case type: {\
            } break

            switch (entity->type) {
                __UPDATE_CASE(PLAYER);
                __UPDATE_CASE(TILE);
                __UPDATE_CASE(TURRET);
                __UPDATE_CASE(TURRET_SHOT);
                __UPDATE_CASE(SPIKES);
                __UPDATE_CASE(LILGUY);
                __EMPTY_CASE(BOGGER);
                __EMPTY_CASE(BOGGER_BALL);
                __EMPTY_CASE(WIZ_BUZZ);
                __EMPTY_CASE(SAVE_POINT);
            }
        }

        i32 target_direction = game_state->rotation_state.target_direction;
        i32 current_direction = game_state->rotation_state.current_direction;
        if (target_direction != current_direction) {
            v2 base_normal;
            switch (current_direction) {
                case DIR_DOWN:  { base_normal = v2 {0.0f, -1.0f}; } break;
                case DIR_LEFT:  { base_normal = v2 {-1.0f, 0.0f}; } break;
                case DIR_UP:    { base_normal = v2 {0.0f, 1.0f}; } break;
                case DIR_RIGHT: { base_normal = v2 {1.0f, 0.0f}; } break;
            }
            
            game_state->rotation_state.progress += (1.0f / TIME_TO_ROTATE) * dt;
            game_state->rotation_state.progress = fmin(1.0f,
                                                       game_state->rotation_state.progress);

            f32 progess_with_easing =
                ((cos(game_state->rotation_state.progress * fPI) * -0.5f) + 0.5f)
                * fPI_OVER_2;
            if (target_direction == ((current_direction + 1) % 4)) { // clockwise
                game_state->player->body->gravity_normal =
                    rotate(base_normal, -progess_with_easing);
            } else {
                assert_(target_direction == (current_direction ? (current_direction - 1) : 3));
                game_state->player->body->gravity_normal =
                    rotate(base_normal, progess_with_easing);
            }

            if (game_state->rotation_state.progress == 1.0f) {
                game_state->rotation_state.current_direction = target_direction;
            }
        }


        update_background(&game_state->background,
                          &game_state->main_render_group,
                          game_state->main_camera,
                          dt);

        update_animations(&game_state->main_animation_group,
                          &game_state->main_render_group,
                          dt);

    }

    tools_update_and_render(game_state, tools_state, dt, window, game_input);
    setup_frame_buffer(game_state->main_render_group.frame_buffer);

    v2 viewport = v2 {
        (f32)game_state->main_render_group.frame_buffer.width,
        (f32)game_state->main_render_group.frame_buffer.height
    };

    f32 rotation = atanv(game_state->player->body->gravity_normal) + fPI_OVER_2;
    // rgba_ up_color = to_rgba(0xffc4f0e7);
    // rgba_ down_color = to_rgba(0xfff7b798);
    rgba_ up_color = to_rgba(0xff562f77);
    rgba_ down_color = to_rgba(0xff66a8bd);
    draw_gradient(&game_state->gl_programs,
                  viewport,
                  rotate(v2 {0.0f, -1.0f}, -rotation),
                  rotate(v2 {0.0f, 1.0f}, -rotation),
                  down_color,
                  up_color);

    draw_render_group(transient_state,
                      &game_state->gl_programs,
                      game_state->main_camera,
                      &game_state->main_render_group);
    draw_render_group(transient_state,
                      &game_state->gl_programs,
                      game_state->ui_camera,
                      &game_state->ui_render_group);

    present_frame_buffer(&game_state->gl_programs,
                         game_state->main_render_group.frame_buffer,
                         default_frame_buffer(window.width, window.height));


    if (was_pressed(game_input->keyboard.p)) {
        game_state->paused = !game_state->paused;
    }

    game_state->advance_one_frame = false;
    if (was_pressed(game_input->keyboard.f)) {
        game_state->advance_one_frame = true;
    }
    
    if (!game_state->paused || game_state->advance_one_frame) {
        clear_render_group(&game_state->main_render_group);
    }
    
    clear_render_group(&game_state->ui_render_group);
}
