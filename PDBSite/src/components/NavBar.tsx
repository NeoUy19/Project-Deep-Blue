import{  NavLink } from "react-router"
import '../styles/navbar.css'
function NavBar() {
  return (
    <nav>
     <ul>
          <li>
            <NavLink to="/">Home</NavLink>
          </li>
          <li>
            <NavLink to="/story">Story</NavLink>
          </li>
          <li>
            <NavLink to="/howtoplay">How To Play</NavLink>
          </li>
        </ul>
    </nav>
  )
}
export default NavBar
