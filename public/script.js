async function optimize() {
    const input = document.getElementById("input").value;

    const res = await fetch("/optimize", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify({ code: input })
    });

    const data = await res.json();

    document.getElementById("output").value = data.output;
}