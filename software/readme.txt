Different project options. Just move the main.c file into the User folder in the submodule and build from there.

Use pico to load code onto board.
`sudo make flash`
erase with `sudo pyocd erase -t py32f002ax5 --chip --config ./Misc/pyocd.yaml`

may need to rebase the template submodule to 4c9de4301e8855a. That's where I was working from in development. New commits untested.


- Standalone has each micrcontroller controlling their own leds. No central control.
