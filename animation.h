#ifndef ANIMATION_H_
#define ANIMATION_H_

const i32 MAX_FRAMES = 128;
struct frame_builder_t {
    rect_i source_rect;
    f32 frame_duration;
};

struct animation_builder_t {
    char* bmp_filepath;
    i32 frame_count;
    i32 cycle_point;
    f32 frame_duration;
    v2 hotspot;
};

#define ANIMATION_BUILDER(name, body) animation_builder_t name##_builder() {\
    animation_builder_t result;\
    char* bmp_filepath;\
    i32 frame_count = -1;\
    i32 cycle_point = 0;\
    f32 frame_duration = -1.0f;\
    v2 hotspot = (v2){0};\
\
    body\
\
    result.bmp_filepath = bmp_filepath;\
    result.frame_count = frame_count;\
    result.cycle_point = cycle_point;\
    result.frame_duration = frame_duration;\
    result.hotspot = hotspot;\
\
    return result;\
}

struct animation_frame_t {
    f32 duration;
    tex2 texture;
    rect_i source_rect;
    f32 pixel_size;
    f32 orientation;
    v2 hotspot;
};

struct animation_spec_t {
    i32 cycle_point;
    array<animation_frame_t> frames;
};

struct animation_t {
	b32 freed;
    f32 frame_progress;
    i32 frame_index;
    f32 orientation;
    f32 z;
    v2 position;
    animation_spec_t* spec;
};

struct animation_group_t {
	vec<animation_t> animations;
	vec<i32> freed;
};

inline i32
add_animation(animation_group_t* animation_group,
              animation_spec_t* spec,
              f32 z) {
	i32 index;
	if (animation_group->freed.count) {
		index = animation_group->freed.pop();
	} else {
		index = animation_group->animations.push_unassigned();
	}

	animation_t* animation = animation_group->animations.at(index);
	animation->freed = false;
	animation->frame_progress = 0.0f;
	animation->frame_index = 0;
	animation->spec = spec;
	animation->z = z;
    animation->orientation = 0.0f;

	return index;
}

inline void
remove_animation(animation_group_t* animation_group, i32 index) {
	assert_(index < animation_group->animations.count);
	animation_t* animation = animation_group->animations.at(index);
	assert_(!animation->freed);
	animation->freed = true;
	animation_group->freed.push(index);

	assert_(false); //TODO(doug): ensure that the render object is removed from the group
}

void
reset_animation(animation_t* animation, animation_spec_t* spec) {
    animation->spec = spec;
    animation->frame_progress = 0.0f;
    animation->frame_index = 0;
}

void
set_animation(animation_t* animation, animation_spec_t* spec) {
    animation->spec = spec;
}

void
update_animations(animation_group_t* animation_group,
                  render_group_t* render_group,
                  f32 dt) {
	for (int i = 0; i < animation_group->animations.count; ++i) {
		animation_t* animation = animation_group->animations.at(i);
		animation->frame_progress += dt;
		animation->frame_index %= animation->spec->frames.count;
		f32 current_frame_duration = animation->spec->frames[animation->frame_index].duration;
		if (animation->frame_progress > current_frame_duration) {
			animation->frame_progress -= current_frame_duration;
			animation->frame_index++;
            if (animation->frame_index >= animation->spec->frames.count) {
                animation->frame_index = animation->spec->cycle_point;
            }
			animation->frame_index %= animation->spec->frames.count;
		}

		animation_frame_t frame = animation->spec->frames[animation->frame_index];

		push_texture(render_group,
		             animation->position,
		             frame.hotspot,
		             frame.pixel_size,
		             frame.texture,
                     frame.source_rect,
                     rgba_t{0},
		             animation->orientation + frame.orientation,
		             animation->z);
	}
}

#endif /* end of include guard: ANIMATION_H_ */
