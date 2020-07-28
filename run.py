#!/usr/bin/python
import time

import pigpio_count


# Lines that are commented out are not necessary

def main():
    c = pigpio_count.PulseCounter(23)
    c1 = pigpio_count.PulseCounter(25)
    # pigpio_count.initialize()
    c.start()
    c1.start()
    for i in range(50000000):
    #    time.sleep(1)
        print("throttle pulse={:4d}  steering pulse={:4d}".format(c.tick_count, c1.tick_count), end="\r")
    # c.stop()
    # pigpio_count.shutdown()


if __name__ == "__main__":
    main()
