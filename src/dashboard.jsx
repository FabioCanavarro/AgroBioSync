import './dashboard.css';
import './ChartStyles.css';
import React, { useState, useEffect } from 'react';
import { Thermometer, Droplet, Wind, Sprout } from 'lucide-react';
import { 
  LineChart, 
  Line, 
  XAxis, 
  YAxis, 
  CartesianGrid, 
  Tooltip, 
  ResponsiveContainer,
  ReferenceLine
} from 'recharts';

const Dashboard = () => {
  const baseUrl = import.meta.env.VITE_API_BASE 
    ? `${import.meta.env.VITE_API_BASE}/sensor-data`
    : '/api/sensor-data';

  const [fetchInterval, setFetchInterval] = useState(500);
  const [sensorData, setSensorData] = useState(null);
  const [error, setError] = useState(null);
  const [historicalData, setHistoricalData] = useState({
    SoilTemp: [],
    AirTemp: [],
    Humidity: [],
    SoilMoisture: []
  });

  // Initialize with some sample data points
  useEffect(() => {
    const now = Date.now();
    const initialData = {
      SoilTemp: [],
      AirTemp: [],
      Humidity: [],
      SoilMoisture: []
    };

    // Create 10 data points for the last 10 minutes
    for (let i = 9; i >= 0; i--) {
      const time = now - (i * 60000); // Every minute
      const baseValues = {
        SoilTemp: 0,
        AirTemp: 0,
        Humidity: 0,
        SoilMoisture:0
      };

      Object.keys(initialData).forEach(key => {
        initialData[key].push({
          time: time,
          value: baseValues[key],
          displayTime: new Date(time).toLocaleTimeString()
        });
      });
    }

    setHistoricalData(initialData);
  }, []);

  useEffect(() => {
    const fetchData = async () => {
      try {
        // Generate mock data instead of fetching when endpoint is not available
        const mockData = {
          SoilTemp: 25,
          AirTemp: 22,
          Humidity: 60,
          SoilMoisture: 40,
          IdealSoilTemp: 23,
          IdealAirTemp: 21,
          IdealHumidity: 65,
          IdealSoilMoisture: 45
        };

        try {
          const response = await fetch(baseUrl, {
            method: 'GET',
            headers: {
              'Accept': 'application/json',
              'Content-Type': 'application/json'
            },
            credentials: 'include', // Include cookies if needed
            mode: 'cors'
          });

          if (response.ok) {
            const data = await response.json();
            setSensorData(data);
            setError(null);
          } else {
            // If API fails, use mock data
            setSensorData(mockData);
            setError('Using simulated data - API endpoint not available');
          }
        } catch (fetchError) {
          // If fetch fails completely, use mock data
          setSensorData(mockData);
          setError('Using simulated data - API connection failed');
        }

        const currentTime = Date.now();

        setHistoricalData(prevData => {
          const newData = { ...prevData };
          Object.keys(newData).forEach(key => {
            if (sensorData?.[key] !== undefined || mockData[key] !== undefined) {
              const value = sensorData?.[key] ?? mockData[key];
              // Add new data point
              const newPoint = {
                time: currentTime,
                value: value,
                displayTime: new Date().toLocaleTimeString()
              };

              // Keep only last 10 minutes of data
              const tenMinutesAgo = currentTime - (10 * 60 * 1000);
              const filteredData = prevData[key]
                .filter(point => point.time > tenMinutesAgo)
                .slice(-60); // Keep maximum 60 points

              newData[key] = [...filteredData, newPoint];
            }
          });
          return newData;
        });
      } catch (error) {
        console.error('Error in data handling:', error);
        setError('Error in data handling. Using placeholder values.');
      }
    };

    const interval = setInterval(fetchData, fetchInterval);

    return () => clearInterval(interval);
  }, [baseUrl, sensorData, fetchInterval]);

  // Rest of the component remains exactly the same...
  const factors = [
    { 
      name: 'Soil Temp', 
      key: 'SoilTemp', 
      icon: Thermometer, 
      current: sensorData?.SoilTemp || 25, 
      ideal: sensorData?.IdealSoilTemp || 23, 
      unit: '°C', 
      colorClass: 'yellow',
      lineColor: '#f59e0b'
    },
    { 
      name: 'Air Temp', 
      key: 'AirTemp', 
      icon: Wind, 
      current: sensorData?.AirTemp || 22, 
      ideal: sensorData?.IdealAirTemp || 21, 
      unit: '°C', 
      colorClass: 'blue',
      lineColor: '#3b82f6'
    },
    { 
      name: 'Humidity', 
      key: 'Humidity', 
      icon: Droplet, 
      current: sensorData?.Humidity || 60, 
      ideal: sensorData?.IdealHumidity || 65, 
      unit: '%', 
      colorClass: 'green',
      lineColor: '#10b981'
    },
    { 
      name: 'Soil Moisture', 
      key: 'SoilMoisture', 
      icon: Sprout, 
      current: sensorData?.SoilMoisture || 40, 
      ideal: sensorData?.IdealSoilMoisture || 45, 
      unit: '%', 
      colorClass: 'brown',
      lineColor: '#8b4513'
    }
  ];

  // Rest of the JSX remains exactly the same...
  const CustomTooltip = ({ active, payload, label }) => {
  if (active && payload && payload.length) {
    const factor = factors.find(f => 
      f.key === Object.keys(historicalData).find(key => 
        historicalData[key].some(data => 
          data.time === label
        )
      )
    );
    
    return (
      <div className="custom-tooltip">
        <p className="tooltip-time">{new Date(label).toLocaleTimeString()}</p>
        <p className="tooltip-value">
          {`${payload[0].value.toFixed(1)}${factor ? factor.unit : ''}`}
        </p>
      </div>
    );
  }
  return null;
};

  return (
    <div className="dashboard">
      <div className="dashboard-container">
        <div className="dashboard-content">
          <h1 className="dashboard-title">Plant Environment Dashboard</h1>
          
          {error && (
            <div className="error-notification">
              <p className="error-title">Note</p>
              <p>{error}</p>
            </div>
          )}
          
          <div className="factors-grid">
            {factors.map((factor) => (
              <div key={factor.name} className="factor-card">
                <div className="factor-content">
                  <div className={`factor-icon ${factor.colorClass}`}>
                    <factor.icon className="icon" aria-hidden="true" />
                  </div>
                  <div className="factor-details">
                    <div className="factor-name">{factor.name}</div>
                    <div className="factor-values">
                      <span className="current-value">
                        {factor.current.toFixed(1)}{factor.unit}
                      </span>
                      <span className="ideal-value">
                        Ideal: {factor.ideal}{factor.unit}
                      </span>
                    </div>
                  </div>
                </div>
                <div className="factor-chart">
                  <ResponsiveContainer width="100%" height={300}>
                    <LineChart 
                      data={historicalData[factor.key]} 
                      margin={{ top: 20, right: 30, left: 20, bottom: 20 }}
                    >
                      <CartesianGrid 
                        strokeDasharray="3 3" 
                        stroke="#e5e7eb"
                      />
                      <XAxis 
                        dataKey="time" 
                        tickFormatter={(time) => new Date(time).toLocaleTimeString()}
                        type="number"
                        domain={['dataMin', 'dataMax']}
                        tick={{ fontSize: 12, fill: '#6b7280' }}
                        stroke="#e5e7eb"
                      />
                      <YAxis 
                        domain={[
                          (dataMin) => Math.floor(dataMin * 0.9),
                          (dataMax) => Math.ceil(dataMax * 1.1)
                        ]}
                        tick={{ fontSize: 12, fill: '#6b7280' }}
                        stroke="#e5e7eb"
                      />
                      <Tooltip content={<CustomTooltip />} />
                      <ReferenceLine 
                        y={factor.ideal} 
                        stroke={factor.lineColor} 
                        strokeDasharray="3 3"
                        strokeWidth={2}
                        label={{ 
                          value: 'Ideal',
                          position: 'right',
                          fill: factor.lineColor,
                          fontSize: 12
                        }} 
                      />
                      <Line 
                        type="monotone" 
                        dataKey="value" 
                        stroke={factor.lineColor}
                        strokeWidth={2}
                        dot={false}
                        animationDuration={300}
                      />
                    </LineChart>
                  </ResponsiveContainer>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
};

export default Dashboard;