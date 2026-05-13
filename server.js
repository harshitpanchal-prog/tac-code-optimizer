const express  = require("express");
const { spawn } = require("child_process");

const app = express();

app.use(express.json());
app.use(express.static("public"));

// ── Platform-aware binary path ────────────────────────────────────────────────
// CMake on Windows produces optimizer.exe; on Linux/Mac it produces ./optimizer
const BINARY = process.platform === "win32" ? ".\\optimizer.exe" : "./optimizer";

// ── POST /optimize ────────────────────────────────────────────────────────────
// Accepts { code: "..." } JSON, pipes it to the C++ optimizer via stdin,
// and returns { output: "..." } with the full diagnostic output.
//
// Using spawn + stdin pipe (instead of writeFileSync + exec) means:
//   ✓ No shared input.txt → no race condition under concurrent requests
//   ✓ Works on Windows (BINARY resolved above)
//   ✓ Errors surfaced properly via stderr and exit code
//
app.post("/optimize", (req, res) => {
    const code = req.body.code;

    if (!code || code.trim() === "") {
        return res.json({ output: "Error: no input code provided." });
    }

    const proc = spawn(BINARY);

    let stdout = "";
    let stderr = "";

    proc.stdout.on("data", (chunk) => { stdout += chunk; });
    proc.stderr.on("data", (chunk) => { stderr += chunk; });

    proc.on("close", (exitCode) => {
        if (exitCode !== 0) {
            const msg = stderr.trim() || "optimizer exited with code " + exitCode;
            return res.json({ output: "Error: " + msg });
        }
        res.json({ output: stdout });
    });

    proc.on("error", (err) => {
        // Fires if the binary is not found or not executable
        res.json({ output: "Error: could not start optimizer — " + err.message });
    });

    // Send the TAC code to the binary via stdin, then close the pipe
    proc.stdin.write(code);
    proc.stdin.end();
});

app.listen(3000, () => {
    console.log("Server running at http://localhost:3000");
    console.log("Using binary:", BINARY);
});