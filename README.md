# ESP32++ (esp32pp)

ESP32++ (esp32pp) is a personal collection of C++ classes designed for use with ESP32 microcontrollers. This library is intended to simplify and enhance the development process on the ESP32 platform by providing a powerful set of reusable components.

## Table of Contents
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [License](#license)

## Features

- **Button**: Class for handling button input.
- **CriticalSection**: Safe handling of critical sections.
- **DHT**: Interface for DHT sensors.
- **HttpServer**: Basic HTTP server functionality.
- **I2C**: Simplified I2C communication.
- **Ina226**: Driver for INA226 power monitor.
- **LedStrip**: Control for LED strips.
- **RF433**: Interface for RF433 modules.
- **Task**: Task management utilities.
- **WiFiManager**: Easy Wi-Fi connection management.

## Installation

Clone the repository:

```bash
git clone https://github.com/realsba/esp32pp.git
```

## Usage

You can use the provided examples to get started with each component. 

For instance, to use the Button class, include it in your project:

### Modify CMakeLists.txt
Edit the CMakeLists.txt file in your main/ directory to include the Button component:
```cmake
idf_component_register(
    SRCS main.cpp
    REQUIRES Button
    INCLUDE_DIRS .
)
```

### Include the Header

In your main.cpp or main.hpp, include the Button class:
```c++
#include "Button/Button.hpp"
```

### Instantiate and Use the Class
Now you can instantiate and use the Button class in your application:
```c++
Button myButton(GPIO_NUM_0); // Example using GPIO 0
myButton.setPressedHandler([](){
    // Action on button press
});
```

### Build and Flash
```bash
idf.py build
idf.py flash
```

## Dependencies

- ESP-IDF: The official development framework for ESP32.
- CMake: Build system generator.

## Contributing
Contributions are welcome! Feel free to open an issue or submit a pull request.

## License
This project is licensed under the MIT License - see the [LICENSE](https://github.com/realsba/esp32pp/blob/main/LICENSE) file for details.

## Author
- Bohdan Sadovyak

## Bugs/Issues
Please report any bugs or issues [here](https://github.com/realsba/esp32pp/issues).