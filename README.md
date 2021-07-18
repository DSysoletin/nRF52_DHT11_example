# nRF52_DHT11_example
Example of working with DHT11 temperature/humidity sensor on nRF52840

Warning! To use this example, you need to know how to work with Nordic SDK, how to compile SDK's examples and load them on the Devkit. 
This example WILL NOT WORK with Arduino IDE.

Example is tested with DHT11 and nRF52840 DevKit (PCA10056)

Connection is very simple:

DHT11 pin 1 <-> VDD (3.3V)
DHT11 pin 2 <-> P0.31
DHT11 pin 3 <-> do not connect
DHT11 pin 4 <-> GND

There is NO pull-up resistor required, we're using internal nRF52's pullup.

How to compile:
1) Download Nordic Zigbee and Thread SDK and arm-gcc-none toolchain.
2) copy dht11_example from this repo to this path: SDK_4.0.0/examples/peripheral/dht11_example
3) cd to SDK_4.0.0/examples/peripheral/dht11_example/pca10056/blank/armgcc
4) run make without parameters
5) resulting binary will be at _build/nrf52840_xxaa.bin. Use nrfjprog to programm it to your Nordic DK
6) Start JLinkRTTViewer, connect to the Nordic chip and watch the logs. In good case there will be something like this:
00> <info> app: DHT11 data: temp: 25.3 , hum: 84.0