// ── Sample TAC code ─────────────────────────────────────────────────────────
const SAMPLE = `t1 = 5 + 3
t2 = t1 * 1
t3 = a + b
t4 = a + b
x = t2 * 0
y = t3 + 0`;

// ── Pass IDs in pipeline order ───────────────────────────────────────────────
const PASS_IDS = ['p-cf', 'p-as', 'p-cp', 'p-cse', 'p-dce'];

// ── DOM refs ─────────────────────────────────────────────────────────────────
const inputEl   = document.getElementById('input');
const lineNums  = document.getElementById('line-nums');
const lineCount = document.getElementById('line-count');
const optBtn    = document.getElementById('opt-btn');
const outBody   = document.getElementById('out-body');
const emptyState= document.getElementById('empty-state');
const statsTag  = document.getElementById('stats-tag');
const copyBtn   = document.getElementById('btn-copy');

// ── Line numbers ─────────────────────────────────────────────────────────────
function syncLines() {
  const lines = inputEl.value.split('\n');
  lineNums.textContent = lines.map((_, i) => i + 1).join('\n');
  const filled = lines.filter(l => l.trim()).length;
  lineCount.textContent = filled + (filled === 1 ? ' line' : ' lines');
}
inputEl.addEventListener('input', syncLines);
inputEl.addEventListener('scroll', () => { lineNums.scrollTop = inputEl.scrollTop; });
syncLines();

// ── Keyboard shortcut ────────────────────────────────────────────────────────
inputEl.addEventListener('keydown', e => {
  if (e.ctrlKey && e.key === 'Enter') { e.preventDefault(); optimize(); }
});

// ── Sample / Clear ───────────────────────────────────────────────────────────
function loadSample() { inputEl.value = SAMPLE; syncLines(); inputEl.focus(); }
function clearInput()  { inputEl.value = '';      syncLines(); inputEl.focus(); }

// ── Pass animation ───────────────────────────────────────────────────────────
function resetPasses() {
  PASS_IDS.forEach(id => {
    const el = document.getElementById(id);
    el.classList.remove('active', 'done');
  });
}

async function animatePasses(done) {
  for (const id of PASS_IDS) {
    if (done) { document.getElementById(id).classList.add('done'); continue; }
    const el = document.getElementById(id);
    el.classList.add('active');
    await sleep(260);
    el.classList.remove('active');
    el.classList.add('done');
  }
}

function sleep(ms) { return new Promise(r => setTimeout(r, ms)); }

// ── Syntax highlight (basic) ─────────────────────────────────────────────────
function esc(s) {
  return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

function highlight(text) {
  return text.split('\n').map(line => {
    const e = esc(line);
    if (/^-{3,}/.test(line))       return `<span class="hl-section">${e}</span>`;
    if (/^Block \d+:/.test(line))  return `<span class="hl-block">${e}</span>`;
    if (/^\w+:$/.test(line.trim()))return `<span class="hl-label">${e}</span>`;
    if (/--\[/.test(line))         return `<span class="hl-arrow">${e}</span>`;
    if (/goto/.test(line))         return e.replace(/(goto)/g, '<span class="hl-goto">$1</span>');
    // Colour simple assignment lines: result = op1 [op op2]
    return e.replace(
      /^(\s*)(\S+)(\s*=\s*)(\S+)(\s*)([-+*/]?)(\s*)(\S*)/,
      (_, sp, res, eq, op1, s2, op, s3, op2) => {
        const colorOp1 = /^\d/.test(op1) ? 'hl-num' : 'hl-var';
        const colorOp2 = /^\d/.test(op2) ? 'hl-num' : 'hl-var';
        return `${sp}<span class="hl-var">${res}</span>${eq}<span class="${colorOp1}">${op1}</span>`
          + (op  ? `${s2}<span class="hl-op">${op}</span>${s3}<span class="${colorOp2}">${op2}</span>` : '');
      }
    );
  }).join('\n');
}

// ── Parse raw output into named sections ─────────────────────────────────────
const SECTIONS = [
  { key: 'orig',  marker: 'ORIGINAL CODE',    label: 'Original Code',  tagClass: 'tag-orig', emoji: '📄' },
  { key: 'bb',    marker: 'BASIC BLOCKS',     label: 'Basic Blocks',   tagClass: 'tag-bb',   emoji: '🗂️' },
  { key: 'cfg',   marker: 'CFG',              label: 'Control Flow',   tagClass: 'tag-cfg',  emoji: '🔀' },
  { key: 'opt',   marker: 'OPTIMIZED BLOCKS', label: 'Optimized',      tagClass: 'tag-opt',  emoji: '✅' },
];

function parseOutput(raw) {
  const result = {};
  const lines  = raw.split('\n');
  let current  = null;
  let buf      = [];

  for (const line of lines) {
    const match = SECTIONS.find(s => line.includes(s.marker));
    if (match) {
      if (current) result[current.key] = buf.join('\n').trim();
      current = match; buf = [];
    } else if (current) {
      buf.push(line);
    }
  }
  if (current) result[current.key] = buf.join('\n').trim();
  return result;
}

// ── Build section card ───────────────────────────────────────────────────────
let cardIndex = 0;
function makeCard(section, content, delay) {
  const id = 'card-' + cardIndex++;
  const card = document.createElement('div');
  card.className = 'out-card';
  card.style.animationDelay = delay + 'ms';

  card.innerHTML = `
    <div class="out-card-head" onclick="toggleCard('${id}')">
      <div class="out-card-title">
        ${section.emoji}
        ${esc(section.label)}
        <span class="out-tag ${section.tagClass}">${section.key.toUpperCase()}</span>
      </div>
      <svg class="chevron" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="6 9 12 15 18 9"/></svg>
    </div>
    <div class="out-card-body" id="${id}">
      <pre class="out-pre">${highlight(content || '(empty)')}</pre>
    </div>`;
  return card;
}

function toggleCard(bodyId) {
  const body = document.getElementById(bodyId);
  const card = body.closest('.out-card');
  card.classList.toggle('collapsed');
}

// ── Count instructions ───────────────────────────────────────────────────────
function countInstructions(text) {
  return text.split('\n').filter(l => l.trim() && !l.includes('Block') && !l.includes('-----')).length;
}

// ── Copy output ──────────────────────────────────────────────────────────────
let lastRawOutput = '';
async function copyOutput() {
  if (!lastRawOutput) return;
  await navigator.clipboard.writeText(lastRawOutput);
  copyBtn.title = 'Copied!';
  setTimeout(() => { copyBtn.title = 'Copy output'; }, 1800);
}

// ── Main optimize function ───────────────────────────────────────────────────
async function optimize() {
  const code = inputEl.value.trim();
  if (!code) { inputEl.focus(); return; }

  // ── Loading state
  optBtn.classList.remove('done');
  optBtn.classList.add('loading');
  resetPasses();
  outBody.innerHTML = '';
  emptyState.style.display = 'none';
  cardIndex = 0;
  statsTag.hidden = true;
  copyBtn.hidden  = true;

  // Start pass animation concurrently
  const passAnim = animatePasses(false);

  try {
    const res  = await fetch('/optimize', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ code }),
    });
    const data = await res.json();

    await passAnim; // wait for pass dots to finish

    optBtn.classList.remove('loading');

    if (!data.output || data.output.startsWith('Error')) {
      optBtn.classList.remove('done');
      outBody.innerHTML = `<div class="out-error">⚠ ${esc(data.output || 'Unknown error')}</div>`;
      return;
    }

    // ── Success
    optBtn.classList.add('done');
    lastRawOutput = data.output;

    const sections = parseOutput(data.output);

    let delay = 0;
    for (const sec of SECTIONS) {
      if (sections[sec.key] !== undefined) {
        outBody.appendChild(makeCard(sec, sections[sec.key], delay));
        delay += 80;
        // Collapse original/bb/cfg by default — keep optimized open
        if (sec.key !== 'opt') {
          await sleep(10);
          const card = outBody.lastChild;
          card.classList.add('collapsed');
        }
      }
    }

    // ── Stats
    const origCount = countInstructions(sections['orig'] || '');
    const optCount  = countInstructions(sections['opt']  || '');
    const removed   = origCount - optCount;
    if (removed > 0) {
      statsTag.textContent = `−${removed} instruction${removed !== 1 ? 's' : ''}`;
      statsTag.hidden = false;
    }
    copyBtn.hidden = false;

  } catch (err) {
    await passAnim;
    optBtn.classList.remove('loading');
    outBody.innerHTML = `<div class="out-error">⚠ Network error: ${esc(err.message)}</div>`;
  }
}