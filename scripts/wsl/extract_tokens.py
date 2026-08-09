import sqlite3, json, base64, time
from pathlib import Path

p = Path(r"C:\Users\odinl\AppData\Roaming\Cursor\User\globalStorage\state.vscdb")
con = sqlite3.connect(f"file:{p}?mode=ro", uri=True)

def get_val(key: str) -> str:
    row = con.execute("SELECT value FROM ItemTable WHERE key=?", (key,)).fetchone()
    if not row:
        return ""
    v = row[0]
    return v.decode("utf-8") if isinstance(v, bytes) else str(v)

access = get_val("cursorAuth/accessToken")
refresh = get_val("cursorAuth/refreshToken")

def jwt_valid(tok: str) -> bool:
    if not tok.startswith("eyJ"):
        return False
    parts = tok.split(".")
    pad = parts[1] + "=" * (-len(parts[1]) % 4)
    payload = json.loads(base64.urlsafe_b64decode(pad))
    exp = payload.get("exp")
    return bool(exp and exp > time.time())

if not access or not jwt_valid(access):
    raise SystemExit("no valid access token in Cursor state.vscdb")

wsl_auth = Path(r"C:\Users\odinl\AppData\Local\Temp\wsl_auth.json")
wsl_auth.write_text(json.dumps({"accessToken": access, "refreshToken": refresh}))
