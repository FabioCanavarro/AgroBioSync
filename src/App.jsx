import './App.css';
import './ChartStyles.css';
import './components/styles/navstyle.css';
import './components/styles/homestyle.css';
import './components/styles/pagestyle.css';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { Navbar, HomePage, AboutPage, ContactPage } from './components/navbar';
import Dashboard from './dashboard';

const App = () => {
  return (
    <Router>
      <div className="app-wrapper">
        <Navbar />
        <main className="main-content">
          <Routes>
            <Route path="/" element={<HomePage />} />
            <Route path="/about" element={<AboutPage />} />
            <Route path="/contact" element={<ContactPage />} />
            <Route path="/dashboard" element={<Dashboard />} />
          </Routes>
        </main>
      </div>
    </Router>
  );
};

export default App;