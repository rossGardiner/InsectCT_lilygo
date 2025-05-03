# InsectCT Project

## Overview
The InsectCT project is designed for the ESP32-S3 platform, utilizing the T-Camera module to capture and process images for insect classification and analysis. This project aims to provide a robust framework for developing applications that leverage the capabilities of the ESP32-S3, including camera functionality and machine learning.

## Project Structure
- **src/**: The main source directory containing setup and loop functions in **main.cpp** for the application and external configuration headers and model weights.
- **platformio.ini**: The configuration file for PlatformIO, specifying environment settings, build flags, library dependencies, and board configurations.

## Setup Instructions
1. **Clone the Repository**: 
   ```
   git clone <repository-url>
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
After uploading the firmware, the ESP32-S3 will start executing the code in `src/main.cpp`. You can modify this file to implement specific logic for your insect classification tasks.

## Contributing
Contributions are welcome! Please feel free to submit a pull request or open an issue for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.