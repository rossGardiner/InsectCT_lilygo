# InsectCT Project

## Overview
The InsectCT project is an example of how models from the [Ecto-Trigger code-base]() can be applied to microcontrollers in firmware, in this case we target the ESP32-S3 platform to capture and process images for insect classification and analysis. The firmware written in this example covers several key aspects: (1) compiler configuration and a tech-stack for writing firmware using the Arduino platform and Platformio, (2) code which performs key functions towards creating a camera trap system including image capture, neural network processing and saving images to SD cards, (3) libraries and dependencies compatible with our platform choice of the Lilygo T-SIMCAM develpoment board. 

## Project Structure
- **src/**: The main source directory containing setup and loop functions in **main.cpp** for the application and external configuration headers and model weights.
- **platformio.ini**: The configuration file for PlatformIO, specifying environment settings, build flags, library dependencies, and board configurations.

## Setup Instructions
1. **Clone the Repository**: 
   ```
   git clone https://github.com/rossGardiner/InsectCT_lilygo.git
   cd InsectCT_lilygo
   ```

2. **Install PlatformIO**: Ensure you have PlatformIO installed in your development environment. This is available for a wide range of operating systems, including Windows. The easiest way to install PlatformIO is as an extension package for VSCode. See [here](https://platformio.org/install) for more details. 

3. **Open the Project**: Open the project in your preferred IDE that supports PlatformIO.

4. **Build the Project**: Use the PlatformIO build command to compile the project:
   ```
   pio run
   ```

5. **Upload to ESP32-S3**: Connect your ESP32-S3 board and upload the firmware:
   ```
   pio run --target upload
   ```

## Usage
After uploading the firmware, the ESP32-S3 will start executing the code in `src/main.cpp`. You can modify this file to implement specific logic for your insect classification tasks, although at present it will run a given model and if a prediction is over a given threshold, it will save the image. 

## Contributing
Contributions are welcome! Please feel free to submit a pull request or open an issue for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.
