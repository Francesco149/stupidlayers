/*
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * For more information, please refer to <http://unlicense.org/>
 */

#define STUPIDLAYERS_IMPLEMENTATION
#include "stupidlayers.c"

/*
 * motospeed ck62 keybinds
 * - esc acts as fn key if held down
 * - caps lock is esc
 * - esc + q is `
 * - esc + shift + q is ~
 * - esc + e is ~
 */

typedef struct {
  stupidlayers_t* sl;
  int matched_hotkey;
} ck62_state_t;

static int pre_handler(void* data, struct input_event* ev, char* k) {
  ck62_state_t* ck62 = data;
  stupidlayers_t* sl = ck62->sl;
  if (ev->code == KEY_CAPSLOCK) { ev->code = KEY_ESC; }
  if (!k[KEY_ESC]) { return 0; }
  switch (ev->code) {
    case KEY_Q: ev->code = KEY_GRAVE; break;
    case KEY_E:
      /* inject tilde */
      ev->value = 1;
      ev->code = KEY_LEFTSHIFT;
      stupidlayers_send(sl, ev);
      ev->code = KEY_GRAVE;
      stupidlayers_send(sl, ev);
      ev->value = 0;
      ev->code = KEY_LEFTSHIFT;
      stupidlayers_send(sl, ev);
      return 1;
    case KEY_1: ev->code = KEY_F1; break;
    case KEY_2: ev->code = KEY_F2; break;
    case KEY_3: ev->code = KEY_F3; break;
    case KEY_4: ev->code = KEY_F4; break;
    case KEY_5: ev->code = KEY_F5; break;
    case KEY_6: ev->code = KEY_F6; break;
    case KEY_7: ev->code = KEY_F7; break;
    case KEY_8: ev->code = KEY_F8; break;
    case KEY_9: ev->code = KEY_F9; break;
    case KEY_0: ev->code = KEY_F10; break;
    case KEY_MINUS: ev->code = KEY_F11; break;
    case KEY_EQUAL: ev->code = KEY_F12; break;
    case KEY_LEFTBRACE: ev->code = KEY_SYSRQ; break;
    case KEY_K: ev->code = KEY_INSERT; break;
    case KEY_L: ev->code = KEY_HOME; break;
    case KEY_SEMICOLON: ev->code = KEY_PAGEUP; break;
    case KEY_APOSTROPHE: ev->code = KEY_PAGEDOWN; break;
    case KEY_COMMA: ev->code = KEY_DELETE; break;
    case KEY_DOT: ev->code = KEY_END; break;
    case KEY_SLASH: ev->code = KEY_UP; break;
    case KEY_RIGHTALT: ev->code = KEY_LEFT; break;
    case KEY_COMPOSE: ev->code = KEY_DOWN; break;
    case KEY_RIGHTCTRL: ev->code = KEY_RIGHT; break;
    case KEY_ESC:
      if (!ev->value) {
        if (ck62->matched_hotkey) {
          ck62->matched_hotkey = 0;
          return 1;
        }
        /* releasing esc and no hotkey matched, inject esc we skipped */
        ev->value = 1;
        stupidlayers_send(sl, ev);
        /* release will be passed through normally */
        ev->value = 0;
      }
    default:
      return 0;
  }
  /*
   * if we get here, a hotkey has matched. I remember this so I can
   * suppress the esc keystroke later
   */
  ck62->matched_hotkey = 1;
  return 0;
}

static int post_handler(void* data, struct input_event* ev, char* k) {
  /* ignore esc keydown. we will send it ourselves if no hotkeys match */
  return ev->code == KEY_ESC && ev->value;
}

#define die(x) { fprintf(stderr, x); exit(1); }

static int run_cli(char* device) {
  ck62_state_t ck62;
  stupidlayers_t* sl;
  ck62.sl = sl = new_stupidlayers(device);
  ck62.matched_hotkey = 0;
  if (!sl) { return 1; }
  if (sl->errstr) die(sl->errstr);
  stupidlayers_run(sl, pre_handler, post_handler, &ck62);
  if (sl->errstr) die(sl->errstr);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 1) {
    fprintf(stderr, "usage: %s /dev/input/eventX\n"
      "you can use use evtest to list devices\n", argv[0]);
    return 1;
  }
  return run_cli(argv[1]);
}
