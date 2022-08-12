# DHT22 Example

This example shows how use DHT22 class to get temperature and humidity 
## How to use Example

### Hardware Required

* A development board with any supported Espressif SOC chip
* A USB cable for power supply and programming
* A DHT22 sensor

### Connection:

![](media/connection.svg)

The GPIO number used in this example can be changed according to your board, by the macro `RMT_GPIO_NUM` defined in the [source file](main/main.cpp).

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

## Console Output

```
I (445) Task: >> DHT:0
I (445) DHT: Create RMT RX channel
I (445) gpio: GPIO[5]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0 
I (445) DHT: Register RX done callback
I (1455) DHT: Decode a response 0x3ffbb164 43
I (1455) main: onReadSensor. humidity: 55.900002, temperature: 24.400000
I (6455) DHT: Decode a response 0x3ffbb164 43
I (6455) main: onReadSensor. humidity: 55.700001, temperature: 24.400000
I (11455) DHT: Decode a response 0x3ffbb164 43
I (11455) main: onReadSensor. humidity: 55.799999, temperature: 24.500000
I (16455) DHT: Decode a response 0x3ffbb164 43
I (16455) main: onReadSensor. humidity: 55.900002, temperature: 24.500000
```
