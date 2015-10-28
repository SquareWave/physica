#ifndef PLAYER_H__
#define PLAYER_H__

struct player_state_t {
	b32 facing_right;
	i32 animation_index;
};

sim_entity_t* create_player(game_state_t* game_state, v2 position);

UPDATE_FUNC(PLAYER);

#endif /* end of include guard: PLAYER_H__ */