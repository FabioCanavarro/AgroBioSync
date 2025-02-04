import fs from 'fs';
import path from 'path';

// Initialize sensor data
let latestSensorData = {
  SoilTemp: 25,
  AirTemp: 22,
  Humidity: 60,
  SoilMoisture: 40,
  IdealSoilTemp: 23,
  IdealAirTemp: 21,
  IdealHumidity: 65,
  IdealSoilMoisture: 45
};

// Try to load persisted data in development
if (process.env.NETLIFY_DEV) {
  try {
    const dataPath = path.join(process.cwd(), 'tmp', 'sensor-data.json');
    latestSensorData = JSON.parse(fs.readFileSync(dataPath, 'utf8'));
  } catch (error) {
    console.log('No existing data file, using default values');
  }
}

const saveData = () => {
  if (process.env.NETLIFY_DEV) {
    const dataPath = path.join(process.cwd(), 'tmp', 'sensor-data.json');
    fs.mkdirSync(path.dirname(dataPath), { recursive: true });
    fs.writeFileSync(dataPath, JSON.stringify(latestSensorData));
  }
};

export const handler = async (event) => {
  // CORS headers
  const headers = {
    'Access-Control-Allow-Origin': process.env.NETLIFY_DEV 
      ? 'http://localhost:5173' 
      : process.env.URL,
    'Access-Control-Allow-Credentials': 'true',
    'Content-Type': 'application/json'
  };

  // Handle OPTIONS preflight
  if (event.httpMethod === 'OPTIONS') {
    return {
      statusCode: 204,
      headers: {
        ...headers,
        'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
        'Access-Control-Allow-Headers': 'Content-Type'
      }
    };
  }

  // Handle GET requests
  if (event.httpMethod === 'GET') {
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify(latestSensorData)
    };
  }

  // Handle POST requests
  if (event.httpMethod === 'POST') {
    try {
      const newData = JSON.parse(event.body);
      latestSensorData = { ...latestSensorData, ...newData };
      saveData();
      
      return {
        statusCode: 200,
        headers,
        body: JSON.stringify({ message: 'Data updated successfully' })
      };
    } catch (error) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Invalid JSON format' })
      };
    }
  }

  // Method not allowed
  return {
    statusCode: 405,
    headers,
    body: JSON.stringify({ error: 'Method not allowed' })
  };
};