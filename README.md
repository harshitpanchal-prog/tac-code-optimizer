# TAC Code Optimizer

> A compiler backend tool that parses **Three-Address Code (TAC)** intermediate representation, partitions it into basic blocks, builds a Control Flow Graph, and applies classical compiler optimizations — all accessible through a browser UI backed by a Node.js + C++ pipeline.

---

## What Is It?

**TAC Code Optimizer** is an academic/systems-programming project that implements the core optimization pipeline found inside real compilers (like GCC or LLVM), but focused specifically on the **intermediate representation (IR)** stage.

Three-Address Code is a common IR format where every instruction has at most one operator and at most three operands:

```
result = op1 operator op2
```

For example:
```
t1 = 5 + 3
t2 = t1 * 1
t3 = a + b
t4 = a + b
x  = t2 * 0
y  = t3 + 0
```

The project is built as a **hybrid system**:
- A **C++ core engine** that performs all parsing and optimization logic
- A **Node.js / Express web server** that bridges the browser to the C++ binary
- A **minimal browser frontend** with an input textarea, an output textarea, and an Optimize button

---

## What Does It Do?

The optimizer accepts TAC code and applies **five classical optimization passes** sequentially on each basic block:

### 1. Constant Folding
Evaluates constant expressions at compile time, eliminating runtime computation.

```
t1 = 5 + 3   →   t1 = 8
```

### 2. Algebraic Simplification
Applies mathematical identity rules to simplify expressions.

| Rule | Before | After |
|---|---|---|
| `x + 0 = x` | `y = t1 + 0` | `y = t1` |
| `0 + x = x` | `y = 0 + t1` | `y = t1` |
| `x * 1 = x` | `t2 = t1 * 1` | `t2 = t1` |
| `1 * x = x` | `t2 = 1 * t1` | `t2 = t1` |
| `x * 0 = 0` | `x = t2 * 0` | `x = 0` |

### 3. Constant Propagation
Tracks variables that hold known constant values and substitutes them directly into subsequent instructions, enabling further folding.

```
t1 = 8        →  known: t1 = 8
t2 = t1 * 1   →  t2 = 8 * 1   →  t2 = 8
```

### 4. Common Subexpression Elimination (CSE)
Detects expressions that are computed more than once and replaces redundant computations with a reference to the already-computed result.

```
t3 = a + b    →  computed, stored
t4 = a + b    →  t4 = t3       (reuse)
```

### 5. Dead Code Elimination (DCE)
Scans all instructions to determine which temporary variables are ever used downstream. Instructions whose results are never referenced are removed.

```
t1 = 8        →  removed if t1 never used again
```

**Example — Full Pass:**

| Input | After All Passes |
|---|---|
| `t1 = 5 + 3` | *(eliminated — t1 constant-folded and propagated into t2, then t2 eliminated)* |
| `t2 = t1 * 1` | *(eliminated — t2 simplified, propagated into x, then x = 0)* |
| `t3 = a + b` | `t3 = a + b` |
| `t4 = a + b` | `t4 = t3` *(CSE)* |
| `x = t2 * 0` | `x = 0` *(algebraic + propagation)* |
| `y = t3 + 0` | `y = t3` *(algebraic simplification)* |

---

## How It Works — Pipeline

### End-to-End Flow

```
Browser UI
    │
    │  POST /optimize  (TAC text as JSON)
    ▼
Node.js Express Server (server.js)
    │
    └── spawn(optimizer)  ← TAC piped via stdin (no shared file)
            │
            ├── Stage 1: Parse
            │       Reads from stdin line by line
            │       Each line → Instruction { result, op1, op, op2 }
            │
            ├── Stage 2: Basic Block Formation
            │       Leader algorithm identifies block entry points:
            │         - First instruction is always a leader
            │         - Instruction after a goto is a leader
            │         - Any labeled instruction is a leader
            │       Instructions are grouped into BasicBlock structs
            │
            ├── Stage 3: Control Flow Graph (CFG)
            │       Builds label → block ID map
            │       Adds jump edges (goto targets) + fallthrough edges
            │       Distinguishes conditional vs unconditional gotos
            │
            └── Stage 4: Optimization Passes  (per block)
                    1. Constant Folding       (div-by-zero guarded)
                    2. Algebraic Simplification
                    3. Constant Propagation   (tracks negatives too)
                    4. CSE                    (pipe-delimited keys)
                    5. Dead Code Elimination  (iterative backward liveness)
                    │
                    ├── Prints diagnostics to stdout → server captures
                    └── Writes final result  → output.txt
                            │
                ◄───────────┘
Node.js captures stdout → returns as JSON
    │
    ▼
Browser displays optimized output
```

### Project Structure

```
tac-code-optimizer/
├── src/
│   ├── main.cpp          # Orchestrates all pipeline stages
│   ├── parser.cpp        # Tokenizes TAC lines → Instruction structs
│   ├── basicblock.cpp    # Leader algorithm → BasicBlock partitioning
│   ├── cfg.cpp           # Builds Control Flow Graph
│   ├── optimizer.cpp     # All 5 optimization passes
│   └── utils.cpp         # isNumber() + isTemporary() helpers
├── include/
│   ├── parser.h          # Instruction struct definition
│   ├── basicblock.h      # BasicBlock struct definition
│   ├── cfg.h             # CFG function declarations
│   ├── optimizer.h       # Optimizer function declarations
│   └── utils.h           # Utility declarations
├── public/
│   ├── index.html        # Browser UI
│   ├── script.js         # Fetch API call to /optimize
│   └── style.css         # Styling
├── server.js             # Node.js + Express web server
├── Makefile              # Build: make / make clean / make rebuild
├── CMakeLists.txt        # Cross-platform CMake build
├── input.txt             # Sample TAC input (CLI: ./optimizer input.txt)
├── output.txt            # Optimized output written by binary
└── package.json          # Node.js dependencies
```

### Building and Running

**Step 1: Compile the C++ optimizer**

*Option A — Direct `g++` (works on Windows with MinGW, Linux, and Mac — no extra tools needed):*
```bash
g++ -std=c++11 -Wall -Wextra -I include \
    src/main.cpp src/parser.cpp src/basicblock.cpp \
    src/cfg.cpp src/optimizer.cpp src/utils.cpp \
    -o optimizer.exe
```
> On Linux / Mac, change `-o optimizer.exe` to `-o optimizer`.

*Option B — Makefile (Linux / Mac / Git Bash on Windows):*
```bash
make
```

*Option C — CMake (requires CMake installed):*
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The compiled binary (`optimizer` or `optimizer.exe`) is placed in the project root automatically.

**Step 2: Install Node.js dependencies**
```bash
npm install
```

**Step 3: Start the web server**
```bash
node server.js
```

You should see:
```
Server running at http://localhost:3000
Using binary: .\optimizer.exe
```

**Step 4: Open the browser**
```
http://localhost:3000
```

Paste TAC code in the left panel and click **Optimize** (or press `Ctrl + Enter`).
Use the **Load Sample** button to try a built-in example instantly.

---

## Scalability

The current implementation is designed as a single-pass, single-file optimizer suitable for academic study. Below is how it can scale toward production-grade use:

### What Scales Well
- **Pass modularity** — Each optimization is a self-contained function operating on a `vector<Instruction>`. New passes (e.g., loop unrolling, strength reduction) can be added without touching existing passes.
- **Basic block model** — The `BasicBlock` + `CFG` architecture directly maps to how LLVM and GCC structure their IR, making it straightforward to extend.
- **Stateless passes** — Each pass has no shared global state, making them easy to parallelize across blocks.

### Current Limitations & Upgrade Path

| Limitation | Status | Scalable Approach |
|---|---|---|
| **Inter-block optimization** | Passes run per-block only | Implement dataflow analysis (reaching definitions, live variable analysis) across the full CFG |
| **CFG edges** | ✅ Jump + fallthrough edges resolved | Add support for `if/else` conditional branches and indirect jumps |
| **Dead code analysis** | ✅ Iterative backward liveness | Extend to inter-block liveness over the full CFG |
| **Input format** | Space-delimited text file | Extend parser to handle conditionals (`if x goto L`), function calls |
| **Code size** | Fits in memory as a flat vector | For large programs, switch to a linked IR (e.g., doubly-linked instruction list per block) |
| **Build system** | ✅ Makefile + CMakeLists.txt | Add CI/CD pipeline (e.g., GitHub Actions) with correctness test suite |
| **Web server** | ✅ stdin/stdout piping, no shared file | Add request timeout, rate limiting, Monaco Editor UI |
| **Frontend** | Plain HTML textarea | Build a Monaco Editor-based UI with syntax highlighting for TAC |

### Extending the Pass Pipeline

To add a new optimization pass:

1. Declare the function in `include/optimizer.h`:
   ```cpp
   void strengthReduction(vector<Instruction>& code);
   ```

2. Implement it in `src/optimizer.cpp`.

3. Call it in `main.cpp` inside the optimization loop:
   ```cpp
   strengthReduction(blocks[i].instructions);
   ```

No other files need to change.

---

## Changelog

### v1.1.0 — 2026-05-14

#### 🔴 Critical Fixes

| Fix | File | Detail |
|---|---|---|
| Guard `.back()` on empty string | `basicblock.cpp` | Added `!result.empty()` check before `.back() == ':'` — prevents UB crash on blank input lines |
| Division by zero | `optimizer.cpp` | Constant folding skips the instruction unchanged when divisor is `0`; also skips on unknown operators instead of silently folding to `0` |
| Unchecked `ifstream` open | `main.cpp` | Program now exits with code `1` and a descriptive `stderr` message if the input file cannot be opened |

#### 🟠 Correctness Fixes

| Fix | File | Detail |
|---|---|---|
| CSE key collision | `optimizer.cpp` | Replaced raw `op1 + op + op2` concatenation with `op1 + "\|" + op + "\|" + op2` — prevents false subexpression matches when operand/operator strings share characters |
| `isNumber` rejects negatives | `utils.cpp` | Accepts an optional leading `-` so constant folding results like `-2` can be tracked by propagation and re-folded in subsequent passes |

#### 🟡 Platform Fixes

| Fix | File | Detail |
|---|---|---|
| Windows binary name | `server.js` | Resolves binary as `optimizer.exe` on Windows, `./optimizer` on Linux/Mac via `process.platform` |
| Race condition on `input.txt` | `server.js` + `main.cpp` | Replaced `writeFileSync` + `exec` with `spawn` + stdin pipe — each request is isolated with no shared file; concurrent requests can no longer corrupt each other |

#### 🔵 Warning Fixes

| Fix | File | Detail |
|---|---|---|
| Signed/unsigned comparisons | `basicblock.cpp` | All `.size()` comparisons cast to `(int)` — eliminates `-Wsign-compare` warnings |
| Signed/unsigned comparisons | `main.cpp` | Remaining uncast `.size()` loop in the basic blocks print section fixed |

#### ✨ Improvements

| Improvement | File | Detail |
|---|---|---|
| Build system | `Makefile`, `CMakeLists.txt` | `make` / `make clean` / CMake cross-platform build added; binary placed in project root automatically |
| CFG jump resolution | `cfg.cpp` | Full rewrite: label→block ID map, jump + fallthrough edges, conditional vs unconditional goto distinction, `[jump]`/`[fall]` edge labels in output |
| Dead Code Elimination | `optimizer.cpp` | Replaced hardcoded `x`/`y` liveness with iterative backward liveness analysis that converges to a fixed point |
| `output.txt` written | `main.cpp` | Optimized blocks persisted to `output.txt` in addition to stdout |
| `isTemporary()` added | `utils.cpp`, `utils.h` | Detects compiler-generated temporaries (`t1`, `t2`, …) by pattern `t\d+`; used by DCE liveness seeding |
| `printCode` stream overload | `parser.cpp`, `parser.h` | Added `printCode(code, ostream&)` so output routes to any stream (file, stdout, stringstream) |
| Flexible input mode | `main.cpp` | Binary reads from stdin by default (web/pipe mode) or from a file path given as `argv[1]` (CLI mode: `./optimizer input.txt`) |
| Better error surfacing | `server.js` | `stderr` from binary forwarded to browser; binary-not-found caught by `.on("error")`; empty input rejected before spawning |

### v1.2.0 — 2026-05-14

#### 🎨 Frontend Redesign — `public/`

Complete rewrite of all three frontend files. The old interface (two plain textareas + one unstyled button) has been replaced with a premium dark-mode UI.

**Visual & Layout**

| Change | Detail |
|---|---|
| Dark mode | Deep navy (`#0d1117`) background with animated CSS grid overlay and ambient glow orbs |
| Split panel | Full-height side-by-side panels with proper chrome — headers, footers, icon buttons |
| Navbar | Fixed top bar with brand icon, version badge, and animated `● Live` status chip |
| Typography | Google Fonts — **Inter** for UI text, **JetBrains Mono** for all code surfaces |
| Background | Floating radial gradient orbs (blue + green) with smooth CSS drift animation |

**Input Panel — `index.html` + `style.css`**

| Change | Detail |
|---|---|
| Code editor feel | Line numbers column synced to scroll position; monospace font; accent-coloured caret |
| Toolbar | Line count display, **Load Sample** button, **Clear** button |
| Keyboard shortcut | `Ctrl + Enter` triggers optimization; hint shown in panel footer |

**Optimize Button — `style.css` + `script.js`**

| Change | Detail |
|---|---|
| Circular animated button | 68 px circle with gradient fill and glow ring on hover |
| Loading state | Icon fades out → CSS spinner rotates |
| Success state | Spinner replaced by ✓ checkmark; button recolours to green |
| Pass indicators | 5 labelled dots animate left-to-right: grey → blue (active) → green (done) |

**Output Panel — `script.js`**

| Change | Detail |
|---|---|
| Section cards | Raw output parsed into 4 collapsible cards: **Original Code**, **Basic Blocks**, **Control Flow**, **Optimized** |
| Default state | Original / BB / CFG cards collapsed by default; Optimized card open — focus is on the result |
| Syntax highlighting | Variables, operators, numeric literals, block headers, labels, goto arrows each coloured separately |
| Stats badge | `−N instructions` badge in the output header when DCE removes code |
| Copy button | One-click clipboard copy of the full raw output |
| Empty / error states | Illustrated empty state with guidance text; red error card with forwarded `stderr` message |

---

## License

```
MIT License

Copyright (c) 2026

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```
