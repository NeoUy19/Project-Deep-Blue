import repoqr from '../assets/repoqr.svg'
import '../styles/home.css'
function Home(){
    return(
        <div className="homemaindiv">
        <h1 className="welcome">Welcome to Project Deep Blue!</h1>
        <p className="explain">Project Blue Dive is an ocean-themed guessing game for four players, two scientists and two raiders, each with their own arduino.
             The two teams take turns choosing and guessing. The choosing team picks a three-digit number while the other team tries to guess what they typed
             before time runs out. Who will merge victorius, the scientists, or the raiders?</p>
        <p className="idk">
            Created for CS362 <br/> Made by: Farshid Azarvash, Neo Uy, Santiago Villas, & Kostiantyn Pavlyshyn
        </p>
        <img className="qr" src={repoqr} alt="github repo"/>
        <p>Try it yourself!</p>
        </div>
    )
}
export default Home