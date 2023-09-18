Different project options. Just move the main.c file into the User folder in the submodule and build from there.

Use pico to load code onto board.
`sudo make flash`
erase with `sudo pyocd erase -t py32f002ax5 --chip --config ./Misc/pyocd.yaml`

may need to rebase the template submodule to 4c9de4301e8855a. That's where I was working from in development. New commits untested.


- Standalone has each micrcontroller controlling their own leds. No central control.
    - Probably only want to keep PF0 or PF1 resistors which connects micro to LED Din, remove the 7 others

(Work In Progress)
- Pico master, each micro gets polled for it's state over a uart chain and pico controls the LEDs 
    - Each micro needs a different ID
    - Top and bottom micro have different code (see below), but same code board to board (except for ID)
    - Remove all the micro<->LED resistors
    - Top micro remove PF0 resistor
        - Same board micro to micro half duplex over PF1
    - Bottom micro remove PF0 resistor
        - Board to board half duplex over PF0
    - So resistors should be
        - So no horizontal resistors
        - Top micro veritcal resistors should be DNP, DNP, P (boot), DNP, DNP, P (PF1)
        - Bottom micro veritcal resistors should be DNP, DNP, P (boot), DNP, P (PF0), DNP
