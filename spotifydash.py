import asyncio
import socket
import time
import unicodedata
import threading
import os
from pathlib import Path
from http.server import HTTPServer, SimpleHTTPRequestHandler

from winsdk.windows.media.control import (
    GlobalSystemMediaTransportControlsSessionManager as MediaManager
)

# ================= CONFIG =================

ESP32_IP = "192.168.1.156"
ESP32_PORT = 4210

PC_IP = "192.168.1.160"     # IP-ul PC-ului tău
HTTP_PORT = 8080

UPDATE_INTERVAL = 0.25
SEEK_SNAP_SEC = 0.75

BASE_DIR = Path(__file__).resolve().parent
ART_DIR = BASE_DIR / "art"
ART_FILE = "cover.jpg"

# =========================================


def clamp(x, a, b):
    return max(a, min(b, x))


def strip_diacritics(text: str) -> str:
    if not text:
        return ""
    normalized = unicodedata.normalize("NFKD", text)
    return "".join(c for c in normalized if not unicodedata.combining(c))


def normalize_artists(artist_str: str) -> str:
    if not artist_str:
        return ""
    for sep in [";", ",", " & ", " / "]:
        if sep in artist_str:
            parts = [a.strip() for a in artist_str.split(sep) if a.strip()]
            return ", ".join(parts)
    return artist_str.strip()


# ---------- HTTP SERVER ----------
from functools import partial
from http.server import HTTPServer, SimpleHTTPRequestHandler

class NoChunkHTTPRequestHandler(SimpleHTTPRequestHandler):
    protocol_version = "HTTP/1.0"   # ⬅️ CRITIC: dezactivează chunked

def start_http_server():
    handler = partial(
        NoChunkHTTPRequestHandler,
        directory=str(ART_DIR)
    )
    server = HTTPServer(("0.0.0.0", HTTP_PORT), handler)
    print(f"[HTTP] Server running on port {HTTP_PORT}")
    server.serve_forever()


# ---------- THUMBNAIL SAVE ----------
from winsdk.windows.storage.streams import DataReader

async def save_thumbnail(info):
    if not info.thumbnail:
        return False

    try:
        stream = await info.thumbnail.open_read_async()
        size = stream.size

        reader = DataReader(stream)
        await reader.load_async(size)

        buffer = bytearray(size)
        reader.read_bytes(buffer)

        with open(ART_DIR / ART_FILE, "wb") as f:
            f.write(buffer)

        reader.close()
        stream.close()

        return True

    except Exception as e:
        print("[ART] Error saving thumbnail:", e)
        return False


# ---------- MAIN LOOP ----------

async def main():
    ART_DIR.mkdir(exist_ok=True)

    threading.Thread(
        target=start_http_server,
        daemon=True
    ).start()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    manager = await MediaManager.request_async()

    last_t = time.perf_counter()
    last_track_sig = None
    est_pos = 0.0

    art_url = f"http://{PC_IP}:{HTTP_PORT}/{ART_FILE}"

    print("[UDP] Sending to ESP32:", ESP32_IP, ESP32_PORT)

    while True:
        session = manager.get_current_session()
        if not session:
            await asyncio.sleep(0.5)
            continue

        info = await session.try_get_media_properties_async()
        timeline = session.get_timeline_properties()
        playback = session.get_playback_info()

        # ---- METADATA ----
        title = strip_diacritics(info.title or "")
        artist = strip_diacritics(
            normalize_artists(info.artist or "")
        )

        # ---- TIME ----
        real_pos = timeline.position.total_seconds()
        dur = max(timeline.end_time.total_seconds(), 0.0)

        # ---- TRACK CHANGE ----
        track_sig = f"{title}|{artist}|{int(dur)}"
        if track_sig != last_track_sig:
            ok = await save_thumbnail(info)
            if ok:
                last_track_sig = track_sig
                est_pos = real_pos
                print("[ART] cover.jpg updated")
            else:
                print("[ART] thumbnail missing")


        now = time.perf_counter()
        dt = now - last_t
        last_t = now

        is_playing = (playback.playback_status == 4)

        if is_playing:
            est_pos += dt

        est_pos = clamp(est_pos, 0.0, dur)

        if abs(real_pos - est_pos) > SEEK_SNAP_SEC:
            est_pos = real_pos

        pos_i = int(clamp(est_pos, 0.0, dur))
        dur_i = int(dur)
        art_path = ART_DIR / ART_FILE
        art_url_to_send = art_url if art_path.exists() else ""

        # ---- SEND UDP ----
        msg = (
            f"{title}|"
            f"{artist}|"
            f"{pos_i}|"
            f"{dur_i}|"
            f"{art_url_to_send}|"
            f"{1 if is_playing else 0}"
        )

        sock.sendto(msg.encode("utf-8"), (ESP32_IP, ESP32_PORT))

        await asyncio.sleep(UPDATE_INTERVAL)


if __name__ == "__main__":
    asyncio.run(main())
