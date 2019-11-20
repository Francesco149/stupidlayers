one of my keyboards (motospeed ck62) doesn't have a normal fn key, it
toggles layers instead, so I made this tool that captures all keyboard
input through evdev and emulates a fn key by forwarding modified inputs
to a virtual keyboard created with uinput. it has basically zero latency

you can reuse stupidlayers.c as a mini library to make your own keyboard
remapper

keybinds:

* caps lock is esc
* esc is `
* esc acts as a normal fn key if held down
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

if you run this by hand on the keyboard that gets captured you might
end up with the enter key getting stuck from missing the release event.
just press enter again to stop it. it's better if you put this in your
xinitrc though

example of finding a device by name (what I have in my xinitrc):

```sh
sudo killall stupidlayers
for x in /dev/input/event*; do
  devname=$(cat $(echo $x | sed 's|dev|sys/class|g')/device/name)
  if [ "$devname" = "SONiX USB DEVICE" ]; then
    sudo stupidlayers $x &
    break
  fi
done
```

keep in mind that if you put this in .xinitrc you must have no password
required on sudo
