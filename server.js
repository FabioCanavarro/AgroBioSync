import express from "express"
import cors from "cors";

const app = express();
const port = 3001;

app.use(cors({
  origin: 'http://localhost:5173',
  credentials: true
}));

app.use(express.json());

let latestSensorData = {
  SoilTemp: 25,
  AirTemp: 22,
  Humidity: 60,
  SoilMoisture: 40,
  IdealSoilTemp: 23,
  IdealAirTemp: 21,
  IdealHumidity: 65,
  IdealSoilMoisture: 41
};

app.post('/api/sensor-data', (req, res) => {
  const { SoilTemp, AirTemp, Humidity, SoilMoisture } = req.body;
  
  latestSensorData = {
    ...latestSensorData,
    SoilTemp,
    AirTemp,
    Humidity,
    SoilMoisture
  };
  console.log('Received data from NodeMCU:', latestSensorData);
  res.status(200).json({ message: 'Data received successfully' });
});

// Added 1 second delay to the GET endpoint
app.get('/api/sensor-data', (req, res) => {
  setTimeout(() => {
    res.json(latestSensorData);
  }, 1000); // 1000 milliseconds = 1 second
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});