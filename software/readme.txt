Different project options. Just move the main.c file into the User folder in the submodule and build from there.

Use pico to load code onto board.
XXX: TO PROGRAM MUST CONNECT TO 3.3V!!! (Not 5V)
Connect pico pin GP3 to PA14
Connect pico pin GP2 to PA13
`sudo make flash`
erase with `sudo pyocd erase -t py32f002ax5 --chip --config ./Misc/pyocd.yaml`

may need to rebase the template submodule to 4c9de4301e8855a. That's where I was working from in development. New commits untested.


- Standalone has each micrcontroller controlling their own leds. No central control.
    - Probably only want to keep PF0 or PF1 resistors which connects micro to LED Din, remove the 7 others

(Work In Progress)
XXX: Turns out for the 002a version there's only tx or rx on a pin, not both... i2c could be an option if slave mode actually works but I'm unsure with the chinese datasheet. The english datasheet is for the 030 or 003 parts. Bitbang uart probably the best option with interrupts for nonblocking. And probably ditch polling, and just have one way comms.
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

- Pico msater with wled option
    - wip but use a toggle switch on the ws2812b line
