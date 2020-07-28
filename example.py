#!/usr/bin/python
import time
import pigpio_count


#  1. Before using the modul run the service pigpiod
#       > systemctl start pigpiod

#  2. During the run change the logic level at io 23 to produce ticks
#     (IO21 in the lower outter pin in the connector)

def main():
    c = pigpio_count.PulseCounter(21)
    c.start()

    for i in range(15):
        time.sleep(1)
        print(c.tick_count)


if __name__ == "__main__":
    main()

