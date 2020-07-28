from setuptools import setup, Extension


pigpio_count_ext = Extension(
    'pigpio_count',
    sources=['pigpio_count.c'],
    libraries=['pigpiod_if2'],
)


setup(
    name='pigpio_count',
    version='1.0',
    ext_modules=[
        pigpio_count_ext,
    ],
)
