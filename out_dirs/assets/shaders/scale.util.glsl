float scale(float x, vec2 old, vec2 new) {
    return ((x - old.x) / (old.y - old.x)) * (new.y - new.x) + new.x;
}