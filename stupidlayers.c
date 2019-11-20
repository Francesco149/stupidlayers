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

#ifndef STUPIDLAYERS_H
#define STUPIDLAYERS_H

#include <linux/input.h>

/* opaque handle created by new_stupidlayers() */
typedef struct stupidlayers stupidlayers_t;

/* grab /dev/input/event* device exclusively and create a uinput device */
stupidlayers_t* new_stupidlayers(char* device);

/* write input_event ev to the uinput device */
int stupidlayers_send(stupidlayers_t* sl, struct input_event* ev);

/* see stupidlayers_run */
typedef int input_handler_t(void* data, struct input_event* ev,
  char* key_states);

/*
 * read keyboard events in a blocking loop and forward them to the virtual
 * keyboard unless handler returns non-zero.
 *
 * handlesr take the input event and a char array that represents the state
 * of all keys at that moment. it's allowed to modify both the event and
 * the state of the keys
 *
 * data can be used to pass custom data to handlers
 *
 * pre_handler is called before the key_states array is modified
 * post_handler is called after the key_states array is modified
 */
void stupidlayers_run(stupidlayers_t* sl, input_handler_t* pre_handler,
  input_handler_t* post_handler, void* data);

/* stop stupidlayers_run loop */
void stupidlayers_stop(stupidlayers_t* sl);

#endif

/* --------------------------------------------------------------------- */

#ifdef STUPIDLAYERS_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct stupidlayers {
  char* errstr;
  int fd, uinput;
  char keys[255];
  int stop;
};

stupidlayers_t* new_stupidlayers(char* device) {
  stupidlayers_t* sl = calloc(sizeof(stupidlayers_t), 1);
  struct uinput_user_dev uidev;
  int i;
  if (!sl) {
    perror("calloc");
    return 0;
  }
  /* capture all input from the device */
  sl->fd = open(device, O_RDONLY);
  if (sl->fd < 0) {
    perror("open");
    sl->errstr = "failed to open device";
    return sl;
  }
  if (ioctl(sl->fd, EVIOCGRAB, (void*)1) < 0) {
    perror("ioctl");
    sl->errstr = "failed to grab device";
    return sl;
  }
  /* create a virtual device that will forward keystrokes to xorg */
  sl->uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (sl->uinput < 0) {
    perror("open");
    sl->errstr = "failed to open /dev/uinput";
    return sl;
  }
  #define evbit(x) \
  if (ioctl(sl->uinput, UI_SET_EVBIT, x) < 0) { \
    perror("ioctl"); \
    sl->errstr = "failed to set " #x; \
    return sl; \
  }
  evbit(EV_KEY)
  #undef evbit
  for (i = 1; i < 255; ++i) {
    if (ioctl(sl->uinput, UI_SET_KEYBIT, i) < 0) {
      perror("ioctl");
      sl->errstr = "failed to set keybit";
      return sl;
    }
  }
  memset(&uidev, 0, sizeof(uidev));
  strcpy(uidev.name, "stupidlayers");
  if (write(sl->uinput, &uidev, sizeof(uidev)) < 0) {
    perror("write");
    sl->errstr = "failed to write to uinput";
    return sl;
  }
  if (ioctl(sl->uinput, UI_DEV_CREATE) < 0) {
    perror("ioctl");
    sl->errstr = "UI_DEV_CREATE failed";
    return sl;
  }
  return sl;
}

void free_stupidlayers(stupidlayers_t* sl) {
  if (sl->fd >= 0) {
    ioctl(sl->fd, EVIOCGRAB, 0);
    close(sl->fd);
  }
  if (sl->uinput) { close(sl->uinput); }
  free(sl);
}

int stupidlayers_send(stupidlayers_t* sl, struct input_event* ev) {
  if (write(sl->uinput, ev, sizeof(struct input_event)) < 0) {
    perror("write");
    sl->errstr = "failed to write to uinput";
    return 0;
  }
  return 1;
}

void stupidlayers_run(stupidlayers_t* sl, input_handler_t* pre_handler,
  input_handler_t* post_handler, void* data)
{
  struct input_event ev;
  while (!sl->stop) {
    if (read(sl->fd, &ev, sizeof(ev)) != sizeof(ev)) {
      perror("read");
      sl->errstr = "failed to read from device";
      return;
    }
    if (ev.type == EV_KEY) {
      if (ev.code > sizeof(sl->keys)) {
        fprintf(stderr, "W: ignoring large keycode 0x%x\n", ev.code);
        continue;
      }
      if (!(ev.value & 2)) {
        /* ignore repeat status, we don't care */
        /* modify event here if desired */
        if (pre_handler && pre_handler(data, &ev, sl->keys)) {
          continue;
        }
        sl->keys[ev.code] = (char)ev.value;
        if (post_handler && post_handler(data, &ev, sl->keys)) {
          continue;
        }
      }
    }
    /* forward event to the virtual device */
    if (!stupidlayers_send(sl, &ev)) { break; }
  }
}

void stupidlayers_stop(stupidlayers_t* sl) { sl->stop = 1; }

#endif /* STUPIDLAYERS_IMPLEMENTATION */
