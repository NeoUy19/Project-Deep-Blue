# Study Context: Project Deep Blue website (TSX + CSS)

Paste this file into a new chat to set up a quiz tutor.

---

## Instructions for the tutor

You are quizzing a student on the React + TypeScript (TSX) and CSS they used to build a 3-page static website for a group project. The student built most of it themselves, with a TA guiding them, and learned by making mistakes. Those mistakes are listed below and are the best material for questions.

**How to run the quiz**
- Ask **one question at a time**, then wait for the answer before saying whether it's right.
- After each answer, say whether it's right, explain **why** in 2–4 sentences, and then ask the next question.
- Mix the question types:
  1. **"What does this do?"**: show a short snippet and ask what it renders or what it styles.
  2. **"Write it"**: describe a goal and have the student write the syntax from memory (a type, a `.map()`, a selector, a CSS rule).
  3. **"Find the bug"**: show a snippet with one or two bugs, usually one of the real mistakes listed below, and have the student find and fix them.
  4. **"Why?"**: conceptual questions, such as why React needs `key` or why inline styles beat a stylesheet.
  5. **"Predict the output"**: what appears on screen, or which TypeScript or ESLint error appears.
- Start easy, then make the questions harder as the student gets them right. Come back later to topics they missed.
- Use the student's **real code and file names** (below) whenever you can.
- Don't give the answer before the student has attempted it. If they're stuck, give a hint first.
- Every 10 questions or so, give a short score summary and list the weak topics.

---

## Project overview

- **Stack:** Vite + React 19 + TypeScript (strict), React Router v7 (`HashRouter`), plain CSS files (global, imported per component), and the `ascii-magic-art` npm package for an animated background.
- **Pages:** Home (`/`), Story (`/story`), How to Play (`/howtoplay`).
- **Theme:** "Deep Blue", an underwater game. The CSS variables are in `src/styles/index.css`:
  ```css
  :root{
      --color-bg: #012A40;
      --color-accent: #A9D6E5;
      --text-color:#FFF780;
      --color-alert: #FF6B6B;
  }
  ```
- **Commands:** `npm run dev`, `npm run build` (runs `tsc -b`, which type-checks, then `vite build`), and `npm run lint` (ESLint).

### File map
```
PDBSite/src/
  main.tsx                 createRoot + <StrictMode>, imports styles/index.css
  App.tsx                  <UnderwaterReef/>, then HashRouter > NavBar + Routes
  components/
    NavBar.tsx             <nav><ul><li><NavLink>…
    Asciibg.tsx            UnderwaterReef: useRef + useEffect calling play()
  pages/
    Home.tsx               heading, paragraphs, QR code <img>
    HowToPlay.tsx          typed rules array + .map() → <ol> of submarine cards
    Story.tsx              nested types + nested .map() → chapters of panels
  types/
    ascii-magic-art.d.ts   ambient module declarations for the JS package
  styles/
    index.css  navbar.css  home.css  asciibg.css  howtoplay.css  story.css
  assets/
    submarine.svg  repoqr.svg
```

### Who wrote what
- **The student wrote:** routing, NavBar, `Asciibg.tsx`, the `.d.ts` file, Home and its CSS, the navbar CSS, the `HowToPlay.tsx` data and `.map()`, and the first versions of the How to Play CSS.
- **The TA wrote**, and the student should be able to explain it: the **final `howtoplay.css`** (with `aspect-ratio`, `object-fit`, percentage padding, container queries and `cqi`, `clamp()`, `min()`) and the whole **Story page** (`Story.tsx` and `story.css`). Quiz these as "explain what this does" more than "write it from memory".

---

## Key code snippets (current state)

**App.tsx (routing)**
```tsx
<div>
  <UnderwaterReef/>
  <HashRouter>
    <NavBar/>
    <Routes>
      <Route path="/" element={<Home/>} />
      <Route path="/story" element={<Story/>} />
      <Route path="/howtoplay" element={<HowToPlay/>} />
    </Routes>
  </HashRouter>
</div>
```

**Asciibg.tsx (ref, effect and cleanup)**
```tsx
export function UnderwaterReef() {
  const ref = useRef<HTMLPreElement>(null);
  useEffect(() => {
    if (ref.current)
      return play(ref.current, piece, { look: { look: "ascii", gamma: 1.15 } })
  }, []);
  return (
    <div className="ascii-bg" aria-hidden="true">
      <pre ref={ref} />
    </div>
  )
}
```
`play()` hides the `<pre>`, inserts a `<canvas>` as a **sibling** right after it, and returns a `stop()` function. The wrapper `div` exists so that (a) CSS can position the canvas and (b) React removes the canvas when the component unmounts.

**ascii-magic-art.d.ts**
```ts
declare module "ascii-magic-art/runtime/player.js"{
    export function play(pre: HTMLPreElement, mod: unknown, opts?:unknown): ()=> void;
}
declare module "ascii-magic-art/pieces/underwater-reef.js";
```

**HowToPlay.tsx (typed data and map)**
```tsx
type RuleBox = { step: number; desc: string; };
const rules: RuleBox[] = [ { step: 1, desc: "Teams and Roles: …" }, /* …6 total */ ];

<ol>
  {rules.map((rule) => (
    <li key={rule.step}>
      <img src={submarine} alt=""/>
      <p>{rule.desc}</p>
    </li>
  ))}
</ol>
```

**Story.tsx (nested types, optional property, nested map, conditional class)**
```tsx
type Panel = { caption: string; alert?: boolean; };
type Chapter = { id: number; title: string; panels: Panel[]; };

{chapters.map((chapter) => (
  <section key={chapter.id} className="chapter">
    <h2>Chapter {chapter.id}: {chapter.title}</h2>
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
<p className="story-cta">Ready to dive? <Link to="/howtoplay">Learn how to play →</Link></p>
```

**howtoplay.css (final)**
```css
.howtoplay ol {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(min(24rem, 100%), 1fr));
    gap: 1.5rem; padding: 0; max-width: 60rem; margin-inline: auto;
}
.howtoplay li { display: grid; container-type: inline-size; }
.howtoplay li img,
.howtoplay li p { grid-area: 1 / 1; }          /* stack image + text in one cell */
.howtoplay li img { width: 100%; aspect-ratio: 8 / 5; object-fit: cover; }
.howtoplay li p {
    align-self: stretch; display: flex; align-items: center; margin: 0;
    padding: 16% 6% 5% 22%;                     /* % padding is relative to WIDTH */
    font-size: clamp(0.65rem, 2.75cqi, 0.85rem);
    line-height: 1.3;
}
```

**navbar.css**
```css
nav { background-color: color-mix(in srgb, var(--color-bg) 50%, transparent);
      z-index: 1; position: sticky; top: 0; }
nav ul { list-style: none; max-width: 25rem; margin-inline: auto; padding: 0;
         display: flex; justify-content: center; }
nav li { flex: 1; text-align: center; }
nav a { text-decoration: none; color: var(--text-color); }
nav a:hover { font-weight: bold; }
nav a.active { text-decoration: underline; text-decoration-thickness: .1rem; }
```

**asciibg.css**
```css
.ascii-bg { position: fixed; inset: 0; z-index: -1; pointer-events: none;
            overflow: hidden; opacity: 50%; }
.ascii-bg pre { font-family: monospace; font-size: 1.66vw; }
/* canvas width = 100 cols × 0.602 × font-size → 100vw ÷ 60.2 ≈ 1.66vw */
```

---

## Topics, with the student's real mistakes (use these for find-the-bug questions)

### 1. Project, tooling and imports
- They ran `npm install` in the **wrong folder** (the repo root instead of `PDBSite/`). It worked locally only because Node searches parent folders for packages, so it would have broken for teammates. Lesson: run `pwd` first.
- **Import paths:** they wrote `"/pages/Home.tsx"` (an absolute path from the filesystem root) instead of `"./pages/Home.tsx"` (relative). Leading `./` means "this folder" and `../` means "the parent folder".
- **Case sensitivity:** they imported `HowToplay.tsx` when the file was `HowToPlay.tsx`. macOS ignores case, but Linux and deploy servers don't. To rename case-only on macOS, use `git mv` twice, through a temporary name.
- **CSS only applies if some file imports it.** They created `asciibg.css` and `navbar.css` but forgot to import them.
- Moving `index.css` into `styles/` without updating the import in `main.tsx` broke the build.
- Unused imports (`useState`, `Link`, `HashRouter` in the wrong file) cause build failures: TS6133 errors, and ESLint `no-unused-vars`.
- An image imported from `src/assets` gives a URL string, and the `vite/client` types make it type-check. Files in `public/` are referenced by an absolute path, without importing them.

### 2. JSX rules
- A component must return **one parent element**. They returned `<HashRouter>…</HashRouter><NavBar/>` side by side (error TS2657).
- **Component names must be PascalCase.** They wrote `function navbar()`, and `<navbar/>` would be treated as an HTML tag.
- In JSX, `className` replaces `class`, and `{}` holds a JS expression.
- **Quotes vs braces:** `src="submarine"` is a literal string, which gives a broken image. `src={submarine}` uses the imported variable.
- A **stray `;` inside JSX** (`<pre ref={ref} />;`) renders as visible text.
- Spaces inside tags (`> {x} <`) add space characters to the output.
- `{" "}` is a Prettier artifact and isn't needed.

### 3. React Router v7
- `HashRouter` wraps the app **once**. Pages don't import router pieces.
- `<Route path="..." element={<Page/>} />`: `element` takes **JSX**, not a bare component name.
- Never put `#` in a route path (they wrote `path="#/home"`). `HashRouter` handles the hash itself.
- Paths in `NavLink to=` must match `Route path=`. They used `"/how to play"`: no spaces in URLs, use lowercase.
- Without a route for `/`, the landing page is blank. Use `path="/"` or the `index` prop.
- `NavLink` must be **inside** the router (it uses router context). They tried wrapping `<Routes>` inside `<NavLink>`, which made the whole page one link.
- `NavLink` automatically adds `class="active"` and `aria-current="page"`. The `end` prop matters for root links.
- `Link` doesn't reload the page. `<a href>` does a full reload.
- JSX order matters: `NavBar` placed after `<Routes>` renders at the bottom of the page.

### 4. TypeScript
- `type Name = { field: type; }`. Use PascalCase and a singular noun. They wrote lowercase `rulebox` at first.
- An array of objects: `const rules: RuleBox[] = [...]`. A misspelled or missing field produces an error.
- **Optional property:** `alert?: boolean`. **Optional parameter:** `opts?: unknown` (the `?` goes after the name and before the colon).
- **Nested types:** `panels: Panel[]` inside `Chapter`.
- **Function type syntax:** `function play(a: A, b: B, c?: C): () => void;`. The return type goes **after** the `)`. They first put `()=> void` **inside** the parameter list (error "Parameter declaration expected").
- **`.d.ts` / ambient modules:** `declare module "exact/import/path";` (shorthand, everything is `any`) or with a body `{ export function … }`. Their mistakes:
  - the file was outside `src/`, so `tsconfig` `"include": ["src"]` never saw it
  - the module string didn't match the import exactly (`"/runtime/player.js"`)
  - a missing `;` on the shorthand
  - declaring `meta` and `default` as functions, when `meta` is an object and `default` is a reserved word
  - copying `play` into the wrong module
- **`unknown` vs `any`:** `unknown` must be narrowed before use, so it's safer.
- **`useRef<HTMLPreElement>(null)`:** without the generic, the ref's type is only `null`.
- **Narrowing:** `ref.current` is `HTMLPreElement | null`. They got TS2345 passing it to `play`. The fix is an `if (ref.current)` guard. `ref.current!` (non-null assertion) silences the error without checking, so avoid it.
- TS7016, "Could not find a declaration file": the package is JS-only, and strict mode won't assume `any`.

### 5. Hooks: `useEffect` and `useRef`
- `useEffect(fn, [])` runs once after mount. **Two arguments**, separated by a comma.
- The effect may return a **cleanup function** or nothing. Returning `play(...)` hands React the `stop()` function, which runs on unmount.
- Without cleanup, animation loops stack up on every navigation. `StrictMode` runs effects twice in development specifically to expose this.
- Refs are `null` during render and attached after it, so you access them inside `useEffect`.
- **Arrow function forms:** `() => expr` returns implicitly. `() => { … }` needs an explicit `return`.
- **Their bracket mistakes:**
  - `useEffect(() => {if ref.current play(...), []})`: `if` needs parentheses, and the `[]` was inside the function body
  - a missing `}` to close the arrow function after closing the `if` block
  - `return play(...)\n , []});`, where the **comma operator** made the function return `[]`. The error was `'() => never[] | undefined' is not assignable to 'EffectCallback'`.

### 6. Lists: `.map()` and keys
- `array.map((item) => (<li key={…}>{item.field}</li>))`: parentheses for an implicit return, curly braces to switch from JSX into JS.
- The parameter is a **plain new name**. They first wrote `rules.map((.desc) => …)`. A dot needs an object on its left: `rule.desc`.
- **The `key` prop is literally named `key`** (they wrote `.step={...}`). It goes on the **outermost** element returned from the map, and must be unique and stable. Avoid the array index for keys.
- **Shadowing:** `rules.map((rules) => …)` reuses the array's name, so use the singular `rule`.
- **Inside vs outside the map:** they put `<ol>` inside the map, which made six lists each numbered "1.". Repeated things go inside the map; the single wrapper goes outside.
- `<ol>`/`<ul>` may only contain `<li>` as direct children. The `<img>` goes inside the `<li>`.
- `.map((item, index) => …)`: the second parameter is the index. `index + 1` is used to display numbers in Story.
- **Conditional className:** `className={cond ? "a b" : "a"}`.

### 7. CSS fundamentals
- **Class vs element selectors:** `.pre` and `.nav` didn't match because the elements had no class. Use `pre` / `nav`, or add the class.
- **Descendant selector** `.ascii-bg pre` (with a space) vs **compound selector** `a.active` and `.panel.alert` (no space).
- **Selector list:** `.howtoplay li img, .howtoplay li p { }`. Repeat the parent in **each** part.
- **Scoping:** a bare `div`, `ul`, `li` or `a` hits the whole site, because Vite CSS is global.
- **Specificity:** an inline `style={{fontSize: 10}}` overrode the stylesheet's `font-size`.
- **Shorthand overriding longhand:** `margin-inline: auto;` followed by `margin: 0;` sets the sides back to 0. The later declaration wins.
- **Invalid values are silently dropped** (shown crossed out with ⚠️ in DevTools). The student's invalid values were:
  - `margin: 10` (no unit; only `0` may be unitless)
  - `color-mix(var(--x) 50%, transparent)` (missing `in srgb,`)
  - `backdrop-filter: blur(white)` (`blur` takes a length)
  - `font-family: "Menlo, Consolas, monospace"` (quotes around the whole list make it one font name)
  - `minmax(MIN, 1fr)` and `max-width: aw` (placeholders typed literally)
  - `list-style: ;` (empty value)
  - an unfinished `gri`
- **Variable naming mismatch:** they defined `--text-color` and used `var(--color-text)`, and it only worked by accident through inheritance.
- **Inheritance vs user-agent styles:** `font-family` set on a parent doesn't reach a `<pre>`, because browsers give `<pre>` its own `monospace`. Links don't inherit `color`, because browsers style `<a>` themselves.
- **Units:**
  - `rem` (relative to the root font size, the usual choice for text and spacing)
  - `em`
  - `px`
  - `%`
  - `vw` / `vh` / `dvh`
  - `ch` (character width, good for `max-width` on paragraphs)
  - `cqi` (1% of the container's width)
- **Functions:**
  - `var()`
  - `color-mix(in srgb, A n%, transparent)` for translucency (better than `opacity`, which also fades the text)
  - `clamp(min, preferred, max)`
  - `min()`
  - `calc()`
  - `repeat()`
  - `minmax()`
- Default browser styles to reset: `body` `margin: 0`, and the `ul`/`ol` padding and margin.

### 8. Layout: Flexbox and Grid
- **Flex:** `display: flex` goes on the **parent**. The main axis is set by `flex-direction`. `justify-content` works on the main axis and `align-items` on the cross axis. With `flex-direction: column`, **the axes swap**.
- **Their flex mistake:** `display: flex` on `nav` instead of the `ul`, so the links stayed vertical.
- `gap` adds space between items only.
- `flex: 1` gives equal shares of the row, so the links spread too far without a `max-width`.
- **Vertical centring needs a height.** `justify-content: center` did nothing until the container had `min-height` or padding.
- `margin-inline: auto` only centres a block narrower than its container (so it needs a `max-width`).
- **Grid:** `grid-template-columns: repeat(auto-fit, minmax(MIN, 1fr))` gives responsive columns with no media queries. `fr` units. `MIN` must be smaller than a phone width (≈24rem). For `max-width`, use N × card width + (N−1) × gap. They first used `minmax(100rem, …)` with `max-width: 5rem`, then `minmax(6rem, …)` with `max-width: 4rem`.
- **`grid-area` only works on grid items**, meaning **direct children** of a grid container. They moved `display: grid` from `li` to `.howtoplay`, which broke the stacking.
- **Grid stacking trick:** put `grid-area: 1 / 1` on several children to overlap them, then use `align-self` and `justify-self` to position them.
- Setting `display: grid` on an `li` removes the list-item numbers, because it's no longer `display: list-item`.

### 9. Positioning, stacking and images
- `position: fixed` (relative to the viewport, doesn't scroll) vs `absolute` (relative to the nearest positioned ancestor, scrolls) vs `sticky` (`top: 0` keeps the nav in view while it stays in the normal flow).
- `inset: 0`
- `z-index`, and the stacking-context gotcha with negative values. The page background goes on `body` so the `z-index: -1` reef stays visible.
- `pointer-events: none` and `overflow: hidden` on the background.
- Fixed and absolute elements are **out of normal flow**, so they don't affect the flex centring.
- `position: relative` on a parent plus `position: absolute` on a child (Story's number badges).
- **Image sizing:**
  - `width: 100%`, `height: auto`
  - `width: 115%` caused overflow into neighbouring images
  - `aspect-ratio`, `object-fit: cover` and `object-position` crop the SVG's transparent space
- **Percentage padding is measured from the container's WIDTH**, even for top and bottom padding.
- **Container queries:** `container-type: inline-size` on the card, so `cqi` units in children scale with the card instead of the screen.
- **Layout shift:** `font-weight: bold` on hover makes text wider, so the neighbouring links move.

### 10. Accessibility and semantics
- Use `<nav>` instead of a fragment, `<ol>` for ordered steps, and `<section>` plus `<h2>` for chapters.
- **`alt`:** meaningful text for informative images (the QR code), and `alt=""` for decorative ones (the repeated submarines).
- `aria-hidden="true"` on the decorative ASCII background and on the emoji.
- `prefers-reduced-motion`: the ascii package already respects it.
- Contrast: aim for 4.5:1 for body text. The original QR code had yellow squares on white, which is too low-contrast to scan.
- `:hover` vs `:focus-visible` (keyboard users).

---

## Remaining issues in the current code (good "find the bug" questions)
- `nav a:hover { font-weight: bold; }` causes layout shift. It's better to change `color`, `opacity` or `text-decoration`.
- `Home.tsx`:
  - the class `idk` isn't a meaningful name
  - the text says "Project **Blue Dive**" but the title is "Project Deep Blue"
  - typos: "victorius", and "merge" should probably be "emerge"
  - `font-size: .5rem` on the credits is very small (8px)
- `Asciibg.tsx`: the `if` without braces, plus inconsistent indentation (the ESLint `curly` rule, Prettier).
- `App.tsx`:
  - a trailing comma in `Route, }`
  - `import{` with no space
  - `path ="..."` has a stray space
  - it imports `App.css`; check whether that file is empty or still needed
- `App.tsx` uses a **named** import (`{ UnderwaterReef }`) from a file named `Asciibg`. The name and file don't match, and the export style differs from the other components, which use default exports.
- `asciibg.css`: `opacity: 50%` on the background also dims the reef colours. Fine as a design choice, but worth explaining.
- `index.css`: `--text-color` doesn't follow the `--color-*` naming pattern.
- `index.html` links to `/favicon.svg`, but `public/` is empty, so it 404s.

---

## Example questions (for calibration)
1. *Find the bug:* `rules.map((rule) => { <li key={rule.step}>{rule.desc}</li> })`. Nothing renders. Why? (The arrow function uses braces with no `return`.)
2. *Write it:* a type `Enemy` with a `name` string, an `hp` number, and an optional boolean `boss`. Then a typed array with two enemies.
3. *Predict:* `.card { margin-inline: auto; margin: 0; max-width: 20rem; }`. Is the card centred?
4. *Why:* why does `grid-area: 1 / 1` on an `img` do nothing when `display: grid` is on the grandparent?
5. *What does it do:* `repeat(auto-fit, minmax(min(24rem, 100%), 1fr))`. What changes between a 1280px screen and a 390px screen?
6. *Find the bug:* `<Route path="/story" element={Story} />`.
7. *Write it:* a `useEffect` that starts a `setInterval` and cleans it up on unmount.
