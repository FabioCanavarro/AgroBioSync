import { useEffect } from 'react';

const TestAPI = () => {
  useEffect(() => {
    // Test GET
    fetch('/api/sensor-data')
      .then(res => res.json())
      .then(data => console.log('GET Response:', data));

    // Test POST
    const mockData = {
      SoilTemp: Math.floor(Math.random() * 30 + 20),
      AirTemp: Math.floor(Math.random() * 25 + 18),
      Humidity: Math.floor(Math.random() * 30 + 50),
      SoilMoisture: Math.floor(Math.random() * 20 + 40)
    };

    fetch('/api/sensor-data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(mockData)
    })
    .then(res => res.json())
    .then(data => console.log('POST Response:', data));
  }, []);

  return (
    <div>
      <h2>API Testing Component</h2>
      <p>Check browser console for results</p>
    </div>
  );
};

export default TestAPI;