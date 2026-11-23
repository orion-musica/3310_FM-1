/* SPDX-License-Identifier: GPL-3.0-only WITH GPL-3.0-linking-exception
 * 3310 - ARRANGE page for FM1 Open (monochrome LCD edition of HASTA LA VISTA).
 *
 * Included once by screen.c, after its drawing helpers and before
 * apply_controls(). Everything here runs on the control task and only uses
 * the public instrument API (instrument.h); the closed engine keeps playing
 * the notes, so timing stays sample accurate.
 *
 * Knobs on the ARRANGE page:
 *   SELECT    track 1..8
 *   PRESETS   sound (program) of the selected track
 *   ALGORITHM track length 16/32/48/64
 *   K1        mode RHYTHM / GROOVE / SPACE / USER
 *   K2..K4    the mode's three parameters
 * USER mode: hold EDIT, turn K2..K4 to pick parameters, release EDIT to keep
 * the new map, EDIT+PLAY to cancel. A short EDIT tap opens the sound editor.
 */

enum { HLV_RHYTHM, HLV_GROOVE, HLV_SPACE, HLV_USER, HLV_MODES };
enum {
    HLV_P_HITS, HLV_P_ROTATE, HLV_P_MUTATE,
    HLV_P_GATE, HLV_P_SWING, HLV_P_CHANCE,
    HLV_P_CUTOFF, HLV_P_PUNCH,
    HLV_P_THROW, HLV_P_REVERB, HLV_P_VOLUME,
    HLV_PARAMS
};
enum { HLV_NOTE_MODE, HLV_NOTE_SAVED, HLV_NOTE_CANCEL, HLV_NOTE_USER_ONLY, HLV_NOTE_PRESET };
#define HLV_ORIG 0xffU
#define HLV_RED 0x4287U /* 3310: one ink */
#define HLV_DARK_RED 0x8531U
#define HLV_GREY 0x4287U
#define HLV_OFF 0x8531U

static const char *const hlv_param_names[HLV_PARAMS] = {
    "HITS", "ROTATE", "MUTATE", "GATE", "SWING", "CHANCE",
    "CUTOFF", "PUNCH", "THROW", "REVERB", "VOLUME"};
static const char *const hlv_param_groups[HLV_PARAMS] = {
    "RHYTHM", "RHYTHM", "RHYTHM", "GROOVE", "GROOVE", "GROOVE",
    "SOUND", "SOUND", "FX", "FX", "FX"};
static const char *const hlv_mode_names[HLV_MODES] = {"RHYTHM", "GROOVE", "SPACE", "USER"};
/* Red, amber, cyan and the USER mode's young-grass green (RGB565). */
/* 3310: every mode uses the LCD ink; the mode chip names the mode. */
static const uint16_t hlv_mode_colors[HLV_MODES] = {HLV_RED, HLV_RED, HLV_RED, HLV_RED};
static const uint8_t hlv_factory_map[3][3] = {
    {HLV_P_HITS, HLV_P_ROTATE, HLV_P_MUTATE},
    {HLV_P_GATE, HLV_P_SWING, HLV_P_CHANCE},
    {HLV_P_CUTOFF, HLV_P_PUNCH, HLV_P_THROW}};

static unsigned hlv_mode;
static uint8_t hlv_user[3], hlv_pending[3];
static unsigned hlv_edit_down, hlv_edit_used, hlv_edit_knob;
static unsigned hlv_note_kind;
static uint32_t hlv_note_at;
static uint32_t hlv_seed;

/* Rhythm generator state. The base is the user's own pattern, captured the
 * first time a neutral track is touched; HITS/ROTATE/MUTATE always derive
 * from it, so returning every knob to neutral restores the original notes. */
static struct groove_step hlv_base[GROOVE_TRACKS][GROOVE_STEPS];
static struct groove_step hlv_out[GROOVE_STEPS], hlv_scratch[GROOVE_STEPS];
static uint8_t hlv_based[GROOVE_TRACKS], hlv_lane[GROOVE_TRACKS];
static uint8_t hlv_hits[GROOVE_TRACKS], hlv_rot[GROOVE_TRACKS];
static int8_t hlv_mut[GROOVE_TRACKS], hlv_cut[GROOVE_TRACKS];
static uint8_t hlv_chance[GROOVE_TRACKS], hlv_gate[GROOVE_TRACKS];
static uint8_t hlv_last_step[GROOVE_TRACKS];
/* THROW raises the delay send while the knob moves and returns it after. */
static uint8_t hlv_throw_amount[GROOVE_TRACKS], hlv_throw_base[GROOVE_TRACKS], hlv_throw_on[GROOVE_TRACKS];
static uint32_t hlv_throw_until[GROOVE_TRACKS];
static uint8_t hlv_patch[156];

static void hlv_init(void)
{
    hlv_mode = HLV_RHYTHM;
    memcpy(hlv_user, hlv_factory_map[0], sizeof hlv_user);
    memcpy(hlv_pending, hlv_user, sizeof hlv_pending);
    hlv_edit_down = hlv_edit_used = 0;
    hlv_edit_knob = 3;
    hlv_note_kind = HLV_NOTE_MODE;
    hlv_note_at = 0;
    hlv_seed = 0x48c27a13U;
    memset(hlv_based, 0, sizeof hlv_based);
    memset(hlv_lane, 0, sizeof hlv_lane);
    memset(hlv_hits, HLV_ORIG, sizeof hlv_hits);
    memset(hlv_rot, 0, sizeof hlv_rot);
    memset(hlv_mut, 0, sizeof hlv_mut);
    memset(hlv_cut, 0, sizeof hlv_cut);
    memset(hlv_chance, 8, sizeof hlv_chance);
    memset(hlv_gate, 8, sizeof hlv_gate);
    memset(hlv_last_step, 0, sizeof hlv_last_step);
    memset(hlv_throw_amount, 0, sizeof hlv_throw_amount);
    memset(hlv_throw_base, 0, sizeof hlv_throw_base);
    memset(hlv_throw_on, 0, sizeof hlv_throw_on);
    memset(hlv_throw_until, 0, sizeof hlv_throw_until);
}
static unsigned hlv_rand(void)
{
    hlv_seed ^= hlv_seed << 13;
    hlv_seed ^= hlv_seed >> 17;
    hlv_seed ^= hlv_seed << 5;
    return hlv_seed;
}
static void hlv_note(unsigned kind, uint32_t now)
{
    hlv_note_kind = kind;
    hlv_note_at = now ? now : 1U;
}
static const uint8_t *hlv_map(void)
{
    if (hlv_mode == HLV_USER) return hlv_edit_down ? hlv_pending : hlv_user;
    return hlv_factory_map[hlv_mode];
}
static const char *hlv_source_short(unsigned type)
{
    static const char *const names[] = {"DX7", "303", "808", "PSG", "PLK", "SMP"};
    return type < GROOVE_SOURCES ? names[type] : "---";
}

/* ---- Rhythm generator: HITS, ROTATE, MUTATE ---------------------------- */
static unsigned hlv_is_hit(unsigned t, const struct groove_step *cell)
{
    if (groove_pad_count(t)) return (cell->drums >> hlv_lane[t]) & 1U;
    return cell->velocity != 0;
}
static unsigned hlv_neutral(unsigned t)
{
    return hlv_hits[t] == HLV_ORIG && !hlv_rot[t] && !hlv_mut[t];
}
static void hlv_snapshot(unsigned t)
{
    groove_sequence_read(t, 0, hlv_base[t], GROOVE_STEPS);
    hlv_lane[t] = (uint8_t)(t == track && groove_pad_count(t) ? drum : 0);
    hlv_based[t] = 1;
}
static unsigned hlv_length(unsigned t)
{
    unsigned length = groove_sequence_length(t);
    return length && length <= GROOVE_STEPS ? length : 16U;
}
static unsigned hlv_base_hits(unsigned t)
{
    unsigned n = 0, length = hlv_length(t);
    if (!hlv_based[t]) {
        struct groove_view v = {0};
        groove_get_view(t, 0, &v);
        for (unsigned s = 0; s < length; ++s) n += (unsigned)((v.active_steps >> s) & 1U);
        return n;
    }
    for (unsigned s = 0; s < length; ++s) n += hlv_is_hit(t, &hlv_base[t][s]);
    return n;
}
/* Euclidean spread of hits over the track length, then rotation. */
static void hlv_generate(unsigned t)
{
    unsigned length = hlv_length(t), drums = groove_pad_count(t) != 0;
    uint16_t bit = (uint16_t)(1U << hlv_lane[t]);
    memcpy(hlv_out, hlv_base[t], sizeof hlv_out);
    if (hlv_hits[t] != HLV_ORIG) {
        unsigned found = 0, used = 0, hits = hlv_hits[t] > length ? length : hlv_hits[t];
        for (unsigned s = 0; s < length; ++s)
            if (hlv_is_hit(t, &hlv_base[t][s])) hlv_scratch[found++] = hlv_base[t][s];
        for (unsigned s = 0; s < length; ++s) {
            unsigned on = (s * hits) % length < hits;
            if (drums) {
                if (on) hlv_out[s].drums = (uint16_t)(hlv_out[s].drums | bit);
                else hlv_out[s].drums = (uint16_t)(hlv_out[s].drums & ~bit);
            } else if (on) {
                if (found) hlv_out[s] = hlv_scratch[used++ % found];
                else {
                    hlv_out[s].note = (uint8_t)(t == track && selected_note < 128 ? selected_note : 60);
                    hlv_out[s].velocity = 100;
                    hlv_out[s].drums = 0;
                }
            } else hlv_out[s].velocity = 0;
        }
    }
    if (hlv_rot[t] % length) {
        memcpy(hlv_scratch, hlv_out, sizeof hlv_scratch);
        for (unsigned s = 0; s < length; ++s) hlv_out[(s + hlv_rot[t]) % length] = hlv_scratch[s];
    }
}
static void hlv_set_hit(unsigned t, struct groove_step *cell, unsigned on)
{
    if (groove_pad_count(t)) {
        uint16_t bit = (uint16_t)(1U << hlv_lane[t]);
        cell->drums = (uint16_t)(on ? cell->drums | bit : cell->drums & ~bit);
    } else if (!on) cell->velocity = 0;
    else if (!cell->velocity) cell->velocity = 100;
}
/* ERODE: drop hits from the generated pattern, fresh on every loop.
 * EVOLVE: move hits of the live pattern, so the changes accumulate. */
static void hlv_mutate(unsigned t)
{
    unsigned length = hlv_length(t);
    int amount = hlv_mut[t];
    if (amount < 0) {
        hlv_generate(t);
        for (unsigned s = 0; s < length; ++s)
            if (hlv_is_hit(t, &hlv_out[s]) && hlv_rand() % 10U < (unsigned)-amount)
                hlv_set_hit(t, &hlv_out[s], 0);
    } else if (amount > 0) {
        groove_sequence_read(t, 0, hlv_out, GROOVE_STEPS);
        if (hlv_rand() % 8U >= (unsigned)amount) return;
        unsigned moves = (unsigned)(amount + 1) / 2;
        for (unsigned m = 0; m < moves; ++m) {
            unsigned from = hlv_rand() % length, to = hlv_rand() % length;
            if (from == to || !hlv_is_hit(t, &hlv_out[from]) || hlv_is_hit(t, &hlv_out[to])) continue;
            if (groove_pad_count(t)) {
                hlv_set_hit(t, &hlv_out[from], 0);
                hlv_set_hit(t, &hlv_out[to], 1);
            } else {
                struct groove_step cell = hlv_out[to];
                hlv_out[to] = hlv_out[from];
                hlv_out[from] = cell;
            }
        }
    } else return;
    groove_sequence_replace(t, 0, hlv_out, length);
}
static void hlv_apply(unsigned t)
{
    if (!hlv_based[t]) hlv_snapshot(t);
    hlv_generate(t);
    groove_sequence_replace(t, 0, hlv_out, hlv_length(t));
}
/* The first touch of a neutral track captures the user's latest edits. */
static void hlv_rhythm_touch(unsigned t)
{
    if (hlv_neutral(t)) hlv_snapshot(t);
}

/* ---- Step settings: GATE (setting 1) and CHANCE (setting 2) ------------- */
static void hlv_step_settings(unsigned t, unsigned setting, unsigned target)
{
    struct groove_view v = {0};
    groove_get_view(t, 0, &v);
    unsigned length = hlv_length(t);
    for (unsigned s = 0; s < length; ++s) {
        if (!((v.active_steps >> s) & 1U)) continue;
        int current = (int)groove_step_setting(t, s, setting);
        if (current != (int)target) groove_step_setting_edit(t, s, setting, (int)target - current);
    }
}

/* ---- Parameter knobs ---------------------------------------------------- */
static void hlv_param_turn(unsigned p, int delta, uint32_t now)
{
    unsigned t = track, length = hlv_length(t);
    int step = delta > 0 ? 1 : -1;
    switch (p) {
    case HLV_P_HITS: {
        hlv_rhythm_touch(t);
        /* Left past zero returns to ORIG, the user's own pattern; ORIG is the left end. */
        if (hlv_hits[t] == HLV_ORIG && delta < 0) break;
        int hits = (hlv_hits[t] == HLV_ORIG ? (int)hlv_base_hits(t) : (int)hlv_hits[t]) + delta;
        hlv_hits[t] = hits < 0 ? (uint8_t)HLV_ORIG : (uint8_t)clamp(hits, 0, (int)length);
        hlv_apply(t);
        break;
    }
    case HLV_P_ROTATE:
        hlv_rhythm_touch(t);
        hlv_rot[t] = (uint8_t)(((int)hlv_rot[t] + delta % (int)length + (int)length) % (int)length);
        hlv_apply(t);
        break;
    case HLV_P_MUTATE:
        hlv_rhythm_touch(t);
        hlv_mut[t] = (int8_t)clamp(hlv_mut[t] + step, -8, 8);
        if (!hlv_mut[t]) hlv_apply(t); /* HOLD restores the generated pattern. */
        break;
    case HLV_P_GATE:
        if (groove_pad_count(t)) { notice = "GATE: MELODIC TRACKS ONLY"; notice_until = now + 1500U; break; }
        hlv_gate[t] = (uint8_t)clamp(hlv_gate[t] + step, 1, 15);
        hlv_step_settings(t, 1, hlv_gate[t]);
        break;
    case HLV_P_SWING:
        groove_sequence_setting_edit(0, delta);
        break;
    case HLV_P_CHANCE:
        hlv_chance[t] = (uint8_t)clamp(hlv_chance[t] + step, 1, 8);
        hlv_step_settings(t, 2, hlv_chance[t] == 8 ? 0U : hlv_chance[t]);
        break;
    case HLV_P_CUTOFF:
        if (groove_source_type(t) == GROOVE_DX7) { notice = "CUTOFF: NOT ON DX7 TRACKS"; notice_until = now + 1500U; break; }
        groove_control_edit(t, 0, 0, delta);
        hlv_cut[t] = (int8_t)clamp(hlv_cut[t] + delta, -99, 99);
        break;
    case HLV_P_PUNCH: {
        if (groove_source_type(t) != GROOVE_DX7) { notice = "PUNCH: DX7 TRACKS ONLY"; notice_until = now + 1500U; break; }
        struct groove_fm_controls controls;
        memset(&controls, 0, sizeof controls);
        if (!groove_fm_copy(t, hlv_patch, &controls) && !controls.enabled) groove_adsr_enable(t, 1);
        groove_adsr_edit(t, 1, delta);
        break;
    }
    case HLV_P_THROW: {
        uint8_t sends[2];
        groove_sends_get(t, sends);
        if (!hlv_throw_on[t]) { hlv_throw_base[t] = sends[1]; hlv_throw_on[t] = 1; }
        hlv_throw_amount[t] = (uint8_t)clamp((int)hlv_throw_amount[t] + step, 0, 15);
        int target = hlv_throw_amount[t] > hlv_throw_base[t] ? hlv_throw_amount[t] : hlv_throw_base[t];
        if (target != sends[1]) groove_sends_edit(t, 1, target - (int)sends[1]);
        hlv_throw_until[t] = now + 1500U;
        break;
    }
    case HLV_P_REVERB:
        groove_sends_edit(t, 0, step);
        break;
    case HLV_P_VOLUME:
        groove_control_edit(t, 0, delta, 0);
        break;
    default:
        break;
    }
}
static void hlv_percent(char out[12], unsigned value)
{
    decimal(out, (int)value, 1);
    strcat(out, "%");
}
static void hlv_param_value(unsigned p, const struct groove_view *v, char out[12])
{
    unsigned t = track;
    out[0] = 0;
    switch (p) {
    case HLV_P_HITS:
        if (hlv_hits[t] == HLV_ORIG) { strcpy(out, "ORIG"); break; }
        decimal(out, hlv_hits[t], 1);
        strcat(out, "/");
        decimal(out + strlen(out), (int)hlv_length(t), 1);
        break;
    case HLV_P_ROTATE:
        if (hlv_rot[t]) { out[0] = '+'; decimal(out + 1, hlv_rot[t], 1); }
        else strcpy(out, "0");
        break;
    case HLV_P_MUTATE:
        if (!hlv_mut[t]) strcpy(out, "HOLD");
        else {
            strcpy(out, hlv_mut[t] < 0 ? "ERODE " : "EVOLV ");
            decimal(out + 6, hlv_mut[t] < 0 ? -hlv_mut[t] : hlv_mut[t], 1);
        }
        break;
    case HLV_P_GATE:
        if (groove_pad_count(t)) strcpy(out, "--");
        else hlv_percent(out, hlv_gate[t] == 15 ? 100U : (hlv_gate[t] * 100U + 8U) / 16U);
        break;
    case HLV_P_SWING:
        hlv_percent(out, groove_sequence_setting(0));
        break;
    case HLV_P_CHANCE:
        hlv_percent(out, hlv_chance[t] == 8 ? 100U : (hlv_chance[t] * 100U + 4U) / 8U);
        break;
    case HLV_P_CUTOFF:
        if (groove_source_type(t) == GROOVE_DX7) strcpy(out, "--");
        else if (!hlv_cut[t]) strcpy(out, "0");
        else { if (hlv_cut[t] > 0) { out[0] = '+'; out[1] = 0; } decimal(out + strlen(out), hlv_cut[t], 1); }
        break;
    case HLV_P_PUNCH: {
        if (groove_source_type(t) != GROOVE_DX7) { strcpy(out, "--"); break; }
        struct groove_fm_controls controls;
        memset(&controls, 0, sizeof controls);
        if (groove_fm_copy(t, hlv_patch, &controls) || !controls.enabled) strcpy(out, "PATCH");
        else decimal(out, controls.envelope[1], 1);
        break;
    }
    case HLV_P_THROW:
        if (!hlv_throw_amount[t]) strcpy(out, "OFF");
        else hlv_percent(out, (hlv_throw_amount[t] * 100U + 7U) / 15U);
        break;
    case HLV_P_REVERB: {
        uint8_t sends[2];
        groove_sends_get(t, sends);
        hlv_percent(out, (sends[0] * 100U + 7U) / 15U);
        break;
    }
    case HLV_P_VOLUME:
        decimal(out, v->volume, 1);
        break;
    default:
        strcpy(out, "--");
        break;
    }
}

/* ---- Controls ----------------------------------------------------------- */
/* Runs early in apply_controls so EDIT/PLAY are not taken by other gestures.
 * Returns 1 when the event was consumed. */
static int hlv_edit_gesture(const struct fm1_controls *c, uint32_t pressed, uint32_t now,
                            unsigned *play_handled)
{
    if (page != HLV) { hlv_edit_down = 0; return 0; }
    if (!hlv_edit_down) {
        if (!(pressed & BUTTON(EDIT)) || (c->buttons & ~BUTTON(EDIT))) return 0;
        hlv_edit_down = 1;
        hlv_edit_used = 0;
        hlv_edit_knob = 3;
        memcpy(hlv_pending, hlv_user, sizeof hlv_pending);
        return 1;
    }
    if (pressed & BUTTON(PLAY)) {
        *play_handled = 1;
        hlv_edit_down = 0;
        hlv_edit_knob = 3;
        memcpy(hlv_pending, hlv_user, sizeof hlv_pending);
        if (hlv_mode == HLV_USER) hlv_note(HLV_NOTE_CANCEL, now);
        return 1;
    }
    if (!(c->buttons & BUTTON(EDIT))) {
        hlv_edit_down = 0;
        hlv_edit_knob = 3;
        if (hlv_edit_used) {
            if (hlv_mode == HLV_USER) {
                memcpy(hlv_user, hlv_pending, sizeof hlv_user);
                hlv_note(HLV_NOTE_SAVED, now);
            }
            return 1;
        }
        /* A short tap keeps the original EDIT meaning: open the sound editor. */
        unsigned edit = groove_source_type(track) == GROOVE_DX7 ? FM_QUICK : TONE_EDIT;
        page = edit;
        advanced_edit = 0;
        if (page == TONE_EDIT) tone_pages[track][tone_slot()] = 0;
        if (page == TONE_EDIT && groove_pad_count(track)) drum_parts[track] = (uint8_t)drum;
        return 1;
    }
    for (unsigned i = 0; i < 3; ++i) {
        int delta = c->encoders[3 + i];
        if (!delta) continue;
        hlv_edit_used = 1;
        if (hlv_mode != HLV_USER) { hlv_note(HLV_NOTE_USER_ONLY, now); continue; }
        int next = clamp((int)hlv_pending[i] + (delta > 0 ? 1 : -1), 0, HLV_PARAMS - 1);
        for (unsigned j = 0; j < 3; ++j)
            if (j != i && hlv_pending[j] == (uint8_t)next) hlv_pending[j] = hlv_pending[i];
        hlv_pending[i] = (uint8_t)next;
        hlv_edit_knob = i;
    }
    if (c->encoders[2]) hlv_edit_used = 1;
    return 1;
}
static void hlv_controls(const struct fm1_controls *c, uint32_t now)
{
    select_track(c->encoders[0]);
    if (c->encoders[1]) {
        sequence_length_adjust(c->encoders[1]);
        if (!hlv_neutral(track)) hlv_apply(track);
    }
    if (c->encoders[6]) {
        groove_control_edit(track, c->encoders[6], 0, 0);
        hlv_note(HLV_NOTE_PRESET, now);
    }
    if (c->encoders[2]) {
        unsigned mode = (unsigned)clamp((int)hlv_mode + c->encoders[2], 0, HLV_MODES - 1);
        if (mode != hlv_mode) { hlv_mode = mode; hlv_note(HLV_NOTE_MODE, now); }
    }
    const uint8_t *map = hlv_map();
    for (unsigned i = 0; i < 3; ++i)
        if (c->encoders[3 + i])
            hlv_param_turn(map[i], fm_encoder_delta(3 + i, c->encoders[3 + i]), now);
}
/* Loop-synchronous work: MUTATE at each track's loop start, THROW release. */
static void hlv_poll(uint32_t now)
{
    for (unsigned t = 0; t < GROOVE_TRACKS; ++t) {
        if (hlv_throw_on[t] && (int32_t)(now - hlv_throw_until[t]) >= 0) {
            uint8_t sends[2];
            groove_sends_get(t, sends);
            if (sends[1] != hlv_throw_base[t]) groove_sends_edit(t, 1, (int)hlv_throw_base[t] - (int)sends[1]);
            hlv_throw_on[t] = 0;
            hlv_throw_amount[t] = 0;
        }
        if (!hlv_mut[t]) continue;
        struct groove_view v = {0};
        groove_get_view(t, 0, &v);
        if (v.playing && v.play_step < hlv_last_step[t]) {
            if (!hlv_based[t]) hlv_snapshot(t);
            hlv_mutate(t);
        }
        hlv_last_step[t] = v.play_step;
    }
}

/* ---- Drawing (240 x 240) ------------------------------------------------ */
enum { HLV_C_HEAD, HLV_C_INFO, HLV_C_ROW, HLV_C_ZOOM = HLV_C_ROW + 8, HLV_C_BOX, HLV_C_HINT, HLV_C_COUNT };
static unsigned hlv_level_frame(const struct groove_view v[8])
{
    for (unsigned t = 0; t < 8; ++t)
        if (v[t].audible && v[t].level >= 64) return 1;
    return 0;
}
static unsigned hlv_box_active(void)
{
    if (hlv_edit_down) return 1;
    return hlv_note_at && frame_time - hlv_note_at < 800U;
}
static void hlv_center(unsigned y, const char *s, unsigned scale, uint16_t fg, uint16_t bg)
{
    unsigned w = (unsigned)strlen(s) * 6U * scale;
    text(w < 180 ? 30 + (180 - w) / 2 : 30, y, s, scale, fg, bg);
}
/* 3310: one 12x12 phone symbol per track (bell, battery, keypad, handset,
 * snake, note, envelope, signal). Bit 11 is the leftmost pixel. */
static const uint16_t hlv_icons[8][12] = {
    {0x060, 0x0f0, 0x1f8, 0x1f8, 0x1f8, 0x3fc, 0x3fc, 0x7fe, 0xfff, 0x000, 0x060, 0x060} /* bell */,
    {0x0f0, 0x3fc, 0x204, 0x2f4, 0x204, 0x2f4, 0x204, 0x2f4, 0x204, 0x2f4, 0x204, 0x3fc} /* battery */,
    {0xeee, 0xeee, 0x000, 0xeee, 0xeee, 0x000, 0xeee, 0xeee, 0x000, 0xeee, 0xeee, 0x000} /* keypad */,
    {0xf00, 0xf08, 0xc04, 0xc44, 0xc22, 0xc22, 0xc22, 0xc44, 0xc04, 0xf08, 0xf00, 0x000} /* handset */,
    {0x004, 0x00e, 0xfce, 0xfc0, 0xc00, 0xc00, 0xffc, 0xffc, 0x00c, 0x00c, 0x1fc, 0x1fc} /* snake */,
    {0x0fe, 0x0fe, 0x082, 0x082, 0x082, 0x082, 0x082, 0x38e, 0x79e, 0x79e, 0x30c, 0x000} /* note */,
    {0x000, 0xfff, 0xc03, 0xa05, 0x909, 0x891, 0x861, 0x801, 0x801, 0xfff, 0x000, 0x000} /* envelope */,
    {0xf80, 0x200, 0x203, 0x203, 0x203, 0x21b, 0x21b, 0x21b, 0x2db, 0x2db, 0x2db, 0x000} /* signal */
};
static void hlv_icon(unsigned x, unsigned y, unsigned t, uint16_t fg, uint16_t bg)
{
    paint_fill(x, y, 14, 14, bg);
    for (unsigned r = 0; r < 12; ++r) {
        uint16_t bits = hlv_icons[t & 7U][r];
        for (unsigned c = 0; c < 12; ) {
            if (!(bits & (0x800U >> c))) { ++c; continue; }
            unsigned start = c;
            while (c < 12 && (bits & (0x800U >> c))) ++c;
            paint_fill(x + 1 + start, y + 1 + r, c - start, 1, fg);
        }
    }
}
static void hlv_draw_box(const struct groove_view *v, uint16_t mc)
{
    /* x 30..209 / 8..231, y 48..175: whole 16-row bands. */
    if (paint_rendering && (paint_top >= 176 || paint_top + PAINT_ROWS <= 48)) return;
    unsigned active = hlv_box_active();
    uint32_t sig = active | (hlv_note_kind << 1) | (hlv_mode << 4) | (hlv_edit_down << 6) |
        ((uint32_t)hlv_edit_knob << 7) | ((uint32_t)hlv_pending[0] << 9) | ((uint32_t)hlv_pending[1] << 13) |
        ((uint32_t)hlv_pending[2] << 17) | ((uint32_t)v->program << 21);
    if (sig == cache->hlv[HLV_C_BOX]) return;
    cache->hlv[HLV_C_BOX] = sig;
    if (!active) return; /* Rows repaint themselves when the box closes. */
    if (hlv_edit_down && hlv_mode == HLV_USER && hlv_edit_knob < 3) {
        unsigned k = hlv_edit_knob, current = hlv_pending[k];
        paint_fill(8, 48, 224, 128, BACK);
        frame(8, 48, 224, 128, mc);
        paint_fill(8, 48, 224, 14, mc);
        char title[16] = "K2 > ";
        title[1] = (char)('2' + k);
        strcat(title, hlv_param_names[current]);
        text(14, 52, title, 1, BACK, mc);
        int first = clamp((int)current - 4, 0, HLV_PARAMS - 8);
        for (unsigned r = 0; r < 8; ++r) {
            unsigned p = (unsigned)first + r, y = 66 + r * 13;
            uint16_t bg = p == current ? mc : BACK, fg = p == current ? BACK : TEXT;
            paint_fill(10, y - 2, 220, 12, bg);
            text(16, y, hlv_param_names[p], 1, fg, bg);
            text(104, y, hlv_param_groups[p], 1, p == current ? BACK : DIM, bg);
            const char *tag = "";
            for (unsigned j = 0; j < 3; ++j)
                if (j != k && hlv_pending[j] == p) tag = j == 0 ? "K2" : j == 1 ? "K3" : "K4";
            if (!*tag && hlv_user[k] == p) tag = "NOW";
            text(204, y, tag, 1, fg, bg);
        }
        return;
    }
    paint_fill(30, 64, 180, 96, hlv_edit_down ? BACK : hlv_note_kind == HLV_NOTE_MODE ||
               hlv_note_kind == HLV_NOTE_SAVED ? mc : PANEL);
    if (hlv_edit_down) {
        frame(30, 64, 180, 96, mc);
        hlv_center(72, hlv_mode == HLV_USER ? "EDIT MAP - USER" : "EDIT - USER ONLY", 1, mc, BACK);
        if (hlv_mode == HLV_USER) {
            hlv_center(92, "TURN K2 K3 K4: PICK", 1, TEXT, BACK);
            hlv_center(108, "RELEASE EDIT: SAVE", 1, TEXT, BACK);
            hlv_center(124, "EDIT + PLAY: CANCEL", 1, TEXT, BACK);
            hlv_center(140, "SAME PARAM: KNOBS SWAP", 1, DIM, BACK);
        } else {
            hlv_center(100, "K1 > USER TO EDIT KNOBS", 1, TEXT, BACK);
            hlv_center(120, "RELEASE EDIT: BACK", 1, DIM, BACK);
        }
        return;
    }
    uint16_t bg = hlv_note_kind == HLV_NOTE_MODE || hlv_note_kind == HLV_NOTE_SAVED ? mc : PANEL;
    uint16_t fg = bg == PANEL ? TEXT : BACK;
    if (hlv_note_kind == HLV_NOTE_MODE) {
        char top[8] = "MODE 1";
        top[5] = (char)('1' + hlv_mode);
        hlv_center(74, top, 1, fg, bg);
        hlv_center(94, hlv_mode_names[hlv_mode], 3, fg, bg);
        char hint[24] = "";
        for (unsigned i = 0; i < 3; ++i) {
            if (i) strcat(hint, " ");
            strcat(hint, hlv_param_names[hlv_map()[i]]);
        }
        hlv_center(132, hint, 1, fg, bg);
    } else if (hlv_note_kind == HLV_NOTE_PRESET) {
        hlv_center(78, "PRESET", 1, fg, bg);
        char out[12];
        decimal(out, v->program + 1, 3);
        hlv_center(98, out, 3, fg, bg);
        hlv_center(132, hlv_source_short(v->type), 1, fg, bg);
    } else {
        static const char *const titles[] = {"", "SAVED", "CANCELLED", "USER ONLY"};
        static const char *const subs[] = {"", "NEW KNOB MAP KEPT", "OLD KNOB MAP BACK", "K1 > USER TO EDIT"};
        hlv_center(80, "USER", 1, fg, bg);
        hlv_center(98, titles[hlv_note_kind], 2, fg, bg);
        hlv_center(128, subs[hlv_note_kind], 1, fg, bg);
    }
}
static void hlv_draw(const struct groove_view v[8])
{
    const struct groove_view *sel = &v[track];
    unsigned mode = hlv_mode;
    uint16_t mc = hlv_mode_colors[mode];
    unsigned lit_frame = hlv_level_frame(v), box = hlv_box_active();
    /* Header: title, mode chip, tempo and the 2-frame level LED. */
    if (!(paint_rendering && paint_top >= 30)) {
        uint32_t sig = mode | (lit_frame << 2) | ((uint32_t)sel->tempo << 3) | ((uint32_t)sel->clock_mode << 13);
        if (sig != cache->hlv[HLV_C_HEAD]) {
            cache->hlv[HLV_C_HEAD] = sig;
            paint_fill(0, 0, 240, 27, BACK);
            text(8, 8, "ARRANGE", 2, TEXT, BACK);
            paint_fill(98, 6, 64, 16, mc);
            text(98 + (64 - 6 * (unsigned)strlen(hlv_mode_names[mode])) / 2, 11, hlv_mode_names[mode], 1, BACK, mc);
            if (sel->clock_mode && !sel->tempo) text(168, 11, "EXT", 1, DIM, BACK);
            else number(168, 11, sel->tempo, 3, 1, DIM, BACK);
            text(190, 11, "BPM", 1, DIM, BACK);
            paint_fill(222, 8, 12, 12, lit_frame ? mc : PANEL);
        }
    }
    /* Info line: selected track, its sound and length. */
    if (!(paint_rendering && (paint_top >= 43 || paint_top + PAINT_ROWS <= 32))) {
        uint32_t sig = track | ((uint32_t)sel->type << 3) | ((uint32_t)sel->program << 6) |
            ((uint32_t)sel->next_length << 13) | ((uint32_t)sel->play_step << 20) | (mode << 27) |
            ((uint32_t)sel->playing << 29);
        if (sig != cache->hlv[HLV_C_INFO]) {
            cache->hlv[HLV_C_INFO] = sig;
            paint_fill(0, 32, 240, 11, BACK);
            text(8, 34, "T", 1, mc, BACK);
            number(14, 34, track + 1, 1, 1, mc, BACK);
            text(28, 34, hlv_source_short(sel->type), 1, TEXT, BACK);
            text(52, 34, "P", 1, DIM, BACK);
            number(58, 34, sel->program + 1, 3, 1, TEXT, BACK);
            text(124, 34, "LEN", 1, DIM, BACK);
            number(144, 34, sel->next_length, 2, 1, sel->next_length != sel->sequence_length ? AMBER : TEXT, BACK);
            text(170, 34, "STEP", 1, DIM, BACK);
            number(196, 34, sel->playing ? sel->play_step + 1 : 0, 2, 1, sel->playing ? TEXT : DIM, BACK);
        }
    }
    /* Eight rows: phone icon (inverts on hits), source, 4 sections x 4 bars. */
    for (unsigned t = 0; t < 8; ++t) {
        unsigned y = 46 + t * 16;
        if (paint_rendering && (y >= paint_top + PAINT_ROWS || y + 14 <= paint_top)) continue;
        unsigned length = v[t].sequence_length ? v[t].sequence_length : 16;
        uint32_t bars = 0;
        for (unsigned b = 0; b < 16; ++b)
            if ((v[t].active_steps >> (b * 4)) & 0xfU) bars |= 1U << b;
        unsigned playing_bar = v[t].playing ? v[t].play_step / 4U : 31U;
        unsigned lit = v[t].audible && v[t].level >= 64;
        unsigned tag = mode == HLV_GROOVE ? hlv_chance[t] : length;
        uint32_t meta = (uint32_t)playing_bar | ((length / 16U) << 5) | ((t == track) << 8) | (lit << 9) |
            (mode << 10) | (box << 12) | ((uint32_t)v[t].muted << 13) | ((uint32_t)v[t].type << 14) | (tag << 17);
        uint32_t sig = signature(meta, &bars, sizeof bars);
        if (sig == cache->hlv[HLV_C_ROW + t]) continue;
        cache->hlv[HLV_C_ROW + t] = sig;
        paint_fill(0, y, 240, 14, BACK);
        if (t == track) paint_fill(0, y, 3, 14, mc);
        uint16_t bg = lit ? mc : PANEL;
        hlv_icon(6, y, t, lit ? BACK : TEXT, bg); /* 2-frame: inverts on hits */
        text(24, y + 4, hlv_source_short(v[t].type), 1, t == track ? mc : v[t].muted ? DIM : TEXT, BACK);
        for (unsigned b = 0; b < 16; ++b) {
            unsigned x = 46 + (b / 4) * 42 + (b % 4) * 10;
            if (b * 4 >= length) { frame(x, y + 2, 8, 10, HLV_OFF); continue; }
            /* One ink: the playing bar is drawn as an outline around its fill. */
            uint16_t color = (bars >> b) & 1U ? mc : PANEL;
            if (b == playing_bar) {
                paint_fill(x, y + 2, 8, 10, TEXT);
                paint_fill(x + 2, y + 4, 4, 6, (bars >> b) & 1U ? DIM : BACK);
            } else paint_fill(x, y + 2, 8, 10, color);
        }
        char value[12];
        if (mode == HLV_GROOVE) {
            decimal(value, hlv_chance[t] == 8 ? 100 : (int)((hlv_chance[t] * 100U + 4U) / 8U), 1);
        } else decimal(value, (int)length, 1);
        text(216, y + 4, value, 1, DIM, BACK);
    }
    /* Zoom: the sixteen steps of the section the selected track plays. */
    if (!(paint_rendering && (paint_top >= 194 || paint_top + PAINT_ROWS <= 178))) {
        unsigned length = sel->sequence_length ? sel->sequence_length : 16;
        unsigned at = sel->playing ? sel->play_step : cursor % length, first = at & ~15U;
        uint64_t lane = groove_pad_count(track) ? groove_drum_steps(track, drum) : sel->active_steps;
        uint32_t bits = (uint32_t)((lane >> first) & 0xffffU);
        uint32_t sig = signature(bits | ((uint32_t)first << 16) | ((uint32_t)(at & 15U) << 22) | (mode << 26) |
            ((uint32_t)sel->playing << 28), &length, sizeof length);
        if (sig != cache->hlv[HLV_C_ZOOM]) {
            cache->hlv[HLV_C_ZOOM] = sig;
            paint_fill(0, 178, 240, 16, BACK);
            for (unsigned i = 0; i < 16; ++i) {
                unsigned x = 8 + i * 14 + (i / 4) * 2, s = first + i;
                if (s >= length) { frame(x, 180, 12, 12, HLV_OFF); continue; }
                unsigned on = (bits >> i) & 1U, head = sel->playing && s == at;
                if (head) {
                    paint_fill(x, 180, 12, 12, TEXT);
                    paint_fill(x + 3, 183, 6, 6, on ? DIM : BACK);
                } else paint_fill(x, 180, 12, 12, on ? mc : PANEL);
            }
        }
    }
    hlv_draw_box(sel, mc);
    /* K1 mode and the three parameters of this mode. */
    knob(0, "K1 MODE", hlv_mode_names[mode], mc);
    const uint8_t *map = hlv_map();
    for (unsigned i = 0; i < 3; ++i) {
        char label[12], value[12];
        strcpy(label, hlv_param_names[map[i]]);
        if (mode == HLV_USER && hlv_edit_down && hlv_pending[i] != hlv_user[i]) strcat(label, "*");
        hlv_param_value(map[i], sel, value);
        knob(1 + i, label, value, mc);
    }
    if (!(paint_rendering && paint_top + PAINT_ROWS <= 229)) {
        const char *hint = notice ? notice : hlv_edit_down ? "EDIT HELD" : "SEL TRACK  PRESETS  ALG LENGTH";
        uint32_t sig = signature(0, hint, (unsigned)strlen(hint));
        if (sig != cache->hlv[HLV_C_HINT]) {
            cache->hlv[HLV_C_HINT] = sig;
            paint_fill(0, 229, 240, 11, BACK);
            text(8, 231, hint, 1, DIM, BACK);
        }
    }
}
