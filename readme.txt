This project contains everything used to construct a Habit Calender inspired from Simone Geirtz https://yetch.store/products/every-day-goal-calendar

This project contains the PCB designs designed in kicad as well as BOM and position files that wer sent off to JLCPCB for manufacturing.

Each board cost around $4 to make and ship and contains two PY32F002A microcontrollers and 32 lights/buttons. It takes 16 boards to make a full 365 day calendar.

The lights are ws2812b which provide endless lighting possibilities to be used outside of a calender concept. The LEDs can be controlled from the microcontroller with a bitbanged driver. A neat accomplishment from a super cheap 8 MHz microcontroller.

This wasn't intended to be public facing yet so the code is somewhat messy from when I was just trying to protoype stuff out, and documentation is not the best. In the software folder, the standalone folder is what you want, but check the readme for helpful info. Hopefully I can find some time to clean everything up.

More information and pictures can be found on www.bradleyconn.space
I need to properly cross link everything still. Sorry about that.
