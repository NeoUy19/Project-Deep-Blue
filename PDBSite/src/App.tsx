import './App.css'
import{ HashRouter, Routes, Route, } from "react-router"
import Home from "./pages/Home.tsx"
import HowToPlay from "./pages/HowToPlay.tsx"
import Story from "./pages/Story.tsx"
import NavBar from "./components/NavBar.tsx"
import { UnderwaterReef } from "./components/Asciibg"

function App() {

  return (
    <div>
    <UnderwaterReef/>
     <HashRouter>
      <NavBar/>
      <Routes>
        <Route path="/" element={<Home/>} />
        <Route path="/story" element={<Story/>} />
        <Route path ="/howtoplay" element={<HowToPlay/>} />
      </Routes>

     </HashRouter>
     </div>
  )
}

export default App
