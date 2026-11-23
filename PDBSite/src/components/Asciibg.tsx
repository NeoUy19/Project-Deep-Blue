import { useEffect, useRef } from "react";
import { play } from "ascii-magic-art/runtime/player.js";
import * as piece from "ascii-magic-art/pieces/underwater-reef.js";
import '../styles/asciibg.css'

export function UnderwaterReef() {
  const ref = useRef<HTMLPreElement>(null);
  // play() draws the piece on a canvas in your look and returns stop(), which React runs on unmount
  useEffect(() => {
    if (ref.current)
        return play(ref.current, piece, { look: { look: "ascii", gamma: 1.15 } })
}, []);
return(
<div className="ascii-bg" aria-hidden="true">
 <pre ref={ref} />
  </div>
)
}

//credit: https://www.ascii-magic.com/ascii/underwater-reef