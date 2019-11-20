one of my keyboards (motospeed ck62) doesn't have a normal fn key, it
toggles layers instead, so I made this tool that captures all keyboard
input through evdev and emulates a fn key by forwarding modified inputs
to a virtual keyboard created with uinput. it has basically zero latency

you can reuse stupidlayers.c as a mini library to make your own keyboard
remapper

keybinds:

* esc acts as a normal fn key if held down
* caps lock is esc
* esc + q is `
* esc + shift + q is ~
* esc + e is ~
* no esc keypress is sent on release if you used it as fn key

# usage

use evtest to figure out which /dev/input/event* device is your keyboard.
for me it's `/dev/input/event3`

```sh
gcc main.c -o stupidlayers
sudo ./stupidlayers /dev/input/event3
```
