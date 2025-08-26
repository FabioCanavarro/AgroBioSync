# AgroBioSync: Intelligent Organic Cultivation

[![PlatformIO Build](https://github.com/FabioCanavarro/AgroBioSync/actions/workflows/PlatformIOBuild.yml/badge.svg)](https://github.com/FabioCanavarro/AgroBioSync/actions/workflows/PlatformIOBuild.yml)
[![Vite Build](https://github.com/FabioCanavarro/AgroBioSync/actions/workflows/ViteBuild.yml/badge.svg)](https://github.com/FabioCanavarro/AgroBioSync/actions/workflows/ViteBuild.yml)

AgroBioSync is a fundamental research project aimed at automating farming with a green alternative for farmers. The project consists of two main components: the Machine and the Organic Fertilizer.

## Table of Contents

* [Features](#features)
* [Planned Features](#planned-features)
* [Website](#website)
* [Technologies Used](#technologies-used)
* [How To Run Locally](#how-to-run-locally)
* [Contributing](#contributing)
* [License](#license)
* [Note](#note)

## Features

* **Web Interface**: A user-friendly interface for monitoring and controlling the system.
* **Dedicated Server**: A reliable backend for data processing and storage.
* **Data Logging**: Comprehensive data logging with historical data visualization.
* **Organic Fertilizer**: An eco-friendly fertilizer to enhance soil quality.
* **Farming Automation**: Automated systems for watering, lighting, and fertilization.
* **Clean Backend**: A well-structured and maintainable backend architecture.

## Planned Features

* [ ] Authentication
* [ ] Cookies for session management
* [x] Navigation Bar
* [x] Home Page
* [x] Contact Us Page
* [x] About Us Page with team member profiles
* [x] WhatsApp Phone number redirect
* [x] Embedded Video
* [x] FAQ Section
* [ ] AI-powered insights based on data analytics

## Website

Link: **https://agrobiosync.netlify.app/**

### HomePage Start
![image](https://github.com/user-attachments/assets/c6b1117c-fbc9-4593-af2b-c50b8068eb1b)
![image](https://github.com/user-attachments/assets/3092fb9f-955b-47f4-b9ee-4f648239eaf0)
![image](https://github.com/user-attachments/assets/dbfa9c2a-af5f-4a72-80df-79f1643d068d)

### Contact Us
![image](https://github.com/user-attachments/assets/f9eb7d7a-0d6a-4c9f-a302-b030ddb7e639)

### About Us
![image](https://github.com/user-attachments/assets/b499d2b1-f19b-49c3-aafa-26290bee3c6c)

### DashBoard
![image](https://github.com/user-attachments/assets/91840514-6dd0-4deb-bfc0-0381b3a2f146)

## Technologies Used

* **Frontend**: React, Tailwind CSS, Vite, Framer Motion, Recharts
* **Backend**: Netlify Functions
* **Hardware**: ESP8266, PlatformIO, Arduino, C++
* **Styling**: Stylelint, Shadcn UI
* **Linting**: ESLint

## How To Run Locally

1.  **Clone the repository**:
    ```bash
    git clone [https://github.com/FabioCanavarro/AgroBioSync.git](https://github.com/FabioCanavarro/AgroBioSync.git)
    cd AgroBioSync
    ```
2.  **Install dependencies**:
    ```bash
    npm install
    ```
3.  **Configure Environment Variables**:
    * Change all variables in the `.env` files to fit your needs.
4.  **Setup the NodeMCU**:
    * Ensure that the NodeMCU is plugged in.
    * Ensure that `platformio.exe` is in your PATH environmental variable.
    * Open a new terminal and run:
        ```bash
        cd NodeMcuHTTPcode
        platformio run --target upload
        platformio device monitor --baud 115200
        ```
    * Use any device to connect the NodeMCU to your Wi-Fi network.
5.  **Start the development server**:
    ```bash
    npm run start
    ```
6.  **Open the application**:
    Open [http://localhost:5173/](http://localhost:5173/) in your browser.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any changes.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Note

* The code is primarily developed by a single person, so unexpected bugs may appear.
* This project was developed within a limited timeframe.
