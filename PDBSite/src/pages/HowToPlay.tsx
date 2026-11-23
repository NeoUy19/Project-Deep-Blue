import submarine from "../assets/submarine.svg";
import "../styles/howtoplay.css";

type RuleBox = {
  step: number;
  desc: string;
};

const rules: RuleBox[] = [
  {
    step: 1,
    desc: "Teams and Roles: Four players are divided into two teams: Team A and Team B. One team plays as Scientists, controlling a submarine, while the other team plays as Invaders, launching attacks.",
  },
  {
    step: 2,
    desc: "Starting the Game: One Scientist presses the joystick button to start the round. The submarine begins with 100% health, and the game starts at Easy difficulty.",
  },
  {
    step: 3,
    desc: "Launching Torpedoes: The Invaders use their IR remotes to enter two secret digits, X and Y. The allowed digits depend on the difficulty: Easy (0-5), Medium (0-7), and Hard (0-9). Each level includes three main attacks.",
  },
  {
    step: 4,
    desc: "Decoding the Signal: The Scientists hear the two digits through buzzer signals. They must count the beeps and enter their guesses using their IR remotes. If both digits are correct and in the right order, the attack is intercepted.",
  },
  {
    step: 5,
    desc: "Dodging Torpedoes: If the Scientists guess incorrectly, an evasion mini-game begins. Three torpedoes approach the submarine from the right side of the LCD. Scientists use their joysticks to move the submarine between the upper and lower rows. Each successful hit reduces submarine health by 25%. If health reaches zero, the round ends immediately.",
  },
  {
    step: 6,
    desc: "Levels, Scoring, and Winning: After completing all three attacks in a level, the game automatically advances from Easy to Medium to Hard, provided the submarine survives. The remaining submarine health determines the points awarded to each team. Teams alternate between Scientists and Invaders for four rounds. At the end, the team with the highest total score wins.",
  },
];
function HowToPlay() {
  return (
    <div className="howtoplay">
      <h1>How To Play - Deep Blue Dive</h1>
      <ol>
      {
        rules.map((rule) => (
            
            <li key={rule.step}> 
            <img src={submarine} alt=""/>
            <p>{rule.desc}</p>
            </li>
            
        ))}
        </ol>
    </div>
  );
}
export default HowToPlay;
