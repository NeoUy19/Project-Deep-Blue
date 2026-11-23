import { Link } from "react-router";
import "../styles/story.css";

// One comic-style panel. `alert` is optional (the ? means it can be left out);
// panels with alert: true get the red "danger" styling.
type Panel = {
  caption: string;
  alert?: boolean;
};

// A chapter holds a list of panels, so this type uses the Panel type inside it.
type Chapter = {
  id: number;
  title: string;
  panels: Panel[];
};

const chapters: Chapter[] = [
  {
    id: 1,
    title: "The Energy Problem",
    panels: [
      { caption: "Global energy demand keeps rising." },
      { caption: "Scientists use algae to generate electricity." },
      { caption: "It works well in sunlight." },
      { caption: "But at night, power drops." },
      { caption: "They need a new, constant light source…" },
      { caption: "…so they look to the deep ocean for answers." },
    ],
  },
  {
    id: 2,
    title: "The Deep-Sea Discovery",
    panels: [
      { caption: "The team explores the deep ocean." },
      { caption: "They find organisms that glow on their own." },
      { caption: "And algae-like life that may use that light." },
      { caption: "They hypothesize it could inspire stable, renewable power." },
      { caption: "They plan a research mission." },
      { caption: "A compact, unarmed submarine is prepared." },
    ],
  },
  {
    id: 3,
    title: "The First Mission… and a Threat",
    panels: [
      { caption: "The submarine dives for the first time." },
      { caption: "It collects valuable footage and data." },
      { caption: "The first mission is a success!" },
      { caption: "On later dives, something unknown begins to appear…" },
      { caption: "The submarine is targeted by torpedoes!", alert: true },
      { caption: "Now the crew must decode signals and survive.", alert: true },
    ],
  },
];

function Story() {
  return (
    <div className="story">
      <h1>The Story</h1>

      {/* Outer map: one <section> per chapter */}
      {chapters.map((chapter) => (
        <section key={chapter.id} className="chapter">
          <h2>
            Chapter {chapter.id}: {chapter.title}
          </h2>

          {/* Inner map: one <li> per panel. The index gives the panel number. */}
          <ol className="panels">
            {chapter.panels.map((panel, index) => (
              <li key={panel.caption} className={panel.alert ? "panel alert" : "panel"}>
                <span className="panel-number">{index + 1}</span>
                <p>{panel.caption}</p>
              </li>
            ))}
          </ol>
        </section>
      ))}

      <p className="story-cta">
        Ready to dive? <Link to="/howtoplay">Learn how to play →</Link>
      </p>
    </div>
  );
}

export default Story;
