#include "background.h"
#include "random.h"

const f32 MOTE_AREA_RADIUS_Y = 5.0f;
const f32 MOTE_AREA_RADIUS_X = 10.0f;
const f32 FLOATY_AREA_RADIUS_Y = 15.0f;
const f32 FLOATY_AREA_RADIUS_X = 25.0f;

global u32 palette[] = {
	0xffd7eaf1, // 0: sky blue
	0xffd7eaf1, // 0: sky blue
	0xffd7eaf1, // 0: sky blue
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xfff8fde3, // 1: cloud white
	0xffbcc4a1, // 2: dark grass green
	0xffbcc4a1, // 2: dark grass green
	0xffefdfb8, // 3: orange-ish
	0xffefdfb8, // 3: orange-ish
	0xffefdfb8, // 3: orange-ish
	0xffefdfb8, // 3: orange-ish
	0xffc3b9a3, // 4: brown
	0xffc3b9a3, // 4: brown
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
	0xffd4e3ec, // 5: light blue
};

global char* texture_paths[] = {
    "assets/cloud.bmp",
    "assets/cloud_2.bmp",
    "assets/tower.bmp",
    "assets/castle.bmp"
};

void create_background(game_state_t* game_state,
                       background_t* background) {

    background->background_color = to_rgb(0xffc8dfec);
    const i32 mote_count = 1000;
    background->motes.count = mote_count;
    background->motes.values = PUSH_ARRAY(&game_state->render_arena,
                                          mote_count,
                                          mote_t);
    const i32 floaty_count = 30;
    background->floaties.count = floaty_count;
    background->floaties.values = PUSH_ARRAY(&game_state->render_arena,
                                             floaty_count,
                                             floaty_t);

    const i32 texture_count = ARRAY_SIZE(texture_paths);
    background->textures.count = texture_count;
    background->textures.values = PUSH_ARRAY(&game_state->render_arena,
                                             texture_count,
                                             tex2);

    for (int i = 0; i < texture_count; ++i) {
        tex2 texture = load_bmp(texture_paths[i], 1);
        background->textures.set(i, texture);
    }

    for (int i = 0; i < floaty_count; ++i) {
        floaty_t* floaty = background->floaties.at(i);

        i32 texture_index = random_i32(0, texture_count);
        floaty->texture = background->textures[texture_index];
        floaty->source_rect = rect_i {0,0,floaty->texture.width, floaty->texture.height};
        floaty->center = v2 {
            random_f32(-FLOATY_AREA_RADIUS_X, FLOATY_AREA_RADIUS_X),
            random_f32(-FLOATY_AREA_RADIUS_Y, FLOATY_AREA_RADIUS_Y)
        };

        floaty->velocity = vpixels_to_meters(random_f32(0.0f, 0.5f));
    }

	for (i32 i = 0; i < mote_count; ++i) {
		mote_t* mote = background->motes.at(i);

		mote->width = random_i32(2, 5);
		mote->color = palette[random_i32(0, ARRAY_SIZE(palette))];

		mote->center = v2 {
			random_f32(-MOTE_AREA_RADIUS_X, MOTE_AREA_RADIUS_X),
			random_f32(-MOTE_AREA_RADIUS_Y, MOTE_AREA_RADIUS_Y)
		};
		mote->velocity = vpixels_to_meters(random_f32(-2.0f, 2.0f));
	}
}

void update_background(background_t* background,
                       render_group_t* render_group,
                       camera_t camera,
                       f32 dt) {
    u32 u32_background_color = from_rgb(background->background_color);

    for (i32 i = 0; i < background->floaties.count; ++i) {
        floaty_t* floaty = background->floaties.at(i);

        floaty->center.x += floaty->velocity * dt;

        f32 x = floaty->center.x;
        f32 y = floaty->center.y;
        f32 min_x = camera.center.x - FLOATY_AREA_RADIUS_X;
        f32 max_x = camera.center.x + FLOATY_AREA_RADIUS_X;
        f32 min_y = camera.center.y - FLOATY_AREA_RADIUS_Y;
        f32 max_y = camera.center.y + FLOATY_AREA_RADIUS_Y;
        x = wrap(x, min_x, max_x);
        y = wrap(y, min_y, max_y);
        floaty->center.x = x;
        floaty->center.y = snap_to(y, VIRTUAL_PIXEL_SIZE);

        push_texture(render_group,
                     v2 { snap_to(floaty->center.x, VIRTUAL_PIXEL_SIZE), floaty->center.y },
                     v2 { floaty->texture.width / 2.0f, floaty->texture.height / 2.0f },
                     VIRTUAL_PIXEL_SIZE,
                     floaty->texture,
                     floaty->source_rect,
                     0.0f,
                     0.95f);
    }

	for (i32 i = 0; i < background->motes.count; ++i) {
		mote_t* mote = background->motes.at(i);

        mote->center.x += mote->velocity * dt;

        f32 x = mote->center.x;
        f32 y = mote->center.y;
        f32 min_x = camera.center.x - MOTE_AREA_RADIUS_X;
        f32 max_x = camera.center.x + MOTE_AREA_RADIUS_X;
        f32 min_y = camera.center.y - MOTE_AREA_RADIUS_Y;
        f32 max_y = camera.center.y + MOTE_AREA_RADIUS_Y;
        x = wrap(x, min_x, max_x);
        y = wrap(y, min_y, max_y);
        mote->center.x = x;
        mote->center.y = snap_to(y, VIRTUAL_PIXEL_SIZE);

		push_rect(render_group,
		          to_rgb(mote->color),
		          v2 { snap_to(mote->center.x, VIRTUAL_PIXEL_SIZE), mote->center.y },
		          v2 { (f32)mote->width, 1.0f } * VIRTUAL_PIXEL_SIZE,
		          0.0f,
                  0.9f,
                  0);
	}
}