#!/usr/bin/env python3
"""Tiny HTTP + SSE server that wraps the native actuator simulator.

Run:  ./serve.py [--port 8765]
Then open http://localhost:8765/ in a browser.

Endpoints
---------
GET  /         -> index.html
GET  /events   -> Server-Sent Events stream of state snapshots (one per tick)
POST /input    -> JSON body, e.g. {"start": true} / {"stop": true} / {"reset": true}

The simulator binary (./sim_bin) is started as a subprocess at server boot;
its stdout JSON lines become SSE events and POSTs are written to its stdin.
Stderr is forwarded to the server's stderr so build/serial output is visible.

Only Python's stdlib is used. Designed for a single local user; threads
fan out events to whoever's listening.
"""

import argparse
import json
import os
import queue
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HERE = Path(__file__).resolve().parent
SIM_BIN = HERE / "sim_bin"
INDEX_HTML = HERE / "index.html"


class SimRunner:
    """Spawns sim_bin and fans its stdout to any SSE subscribers."""

    def __init__(self, binary):
        self.proc = subprocess.Popen(
            [str(binary)],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=sys.stderr,
            bufsize=1,
            text=True,
        )
        self.latest = None
        self.subscribers = []
        self._lock = threading.Lock()
        t = threading.Thread(target=self._reader, daemon=True)
        t.start()

    def _reader(self):
        for line in self.proc.stdout:
            line = line.strip()
            if not line:
                continue
            self.latest = line
            with self._lock:
                subs = list(self.subscribers)
            for q in subs:
                try:
                    q.put_nowait(line)
                except queue.Full:
                    pass

    def subscribe(self):
        q = queue.Queue(maxsize=256)
        with self._lock:
            self.subscribers.append(q)
        # Seed the new subscriber with the most recent snapshot so the page
        # shows current state immediately instead of waiting for the next tick.
        if self.latest:
            try:
                q.put_nowait(self.latest)
            except queue.Full:
                pass
        return q

    def unsubscribe(self, q):
        with self._lock:
            try:
                self.subscribers.remove(q)
            except ValueError:
                pass

    def send_command(self, cmd):
        line = json.dumps(cmd) + "\n"
        try:
            self.proc.stdin.write(line)
            self.proc.stdin.flush()
        except (BrokenPipeError, OSError):
            pass


def make_handler(runner):
    class Handler(BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def log_message(self, fmt, *args):  # quieter access log
            sys.stderr.write("[serve] " + (fmt % args) + "\n")

        def do_GET(self):
            if self.path in ("/", "/index.html"):
                self._serve_file(INDEX_HTML, "text/html; charset=utf-8")
                return
            if self.path == "/events":
                self._serve_events()
                return
            self.send_error(404, "Not found")

        def do_POST(self):
            if self.path != "/input":
                self.send_error(404, "Not found")
                return
            length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(length) if length else b""
            try:
                cmd = json.loads(body or b"{}")
            except json.JSONDecodeError:
                self.send_error(400, "Bad JSON")
                return
            runner.send_command(cmd)
            self.send_response(204)
            self.send_header("Content-Length", "0")
            self.end_headers()

        def _serve_file(self, path, content_type):
            try:
                data = path.read_bytes()
            except FileNotFoundError:
                self.send_error(404, f"missing {path.name}")
                return
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(data)))
            self.send_header("Cache-Control", "no-store")
            self.end_headers()
            self.wfile.write(data)

        def _serve_events(self):
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Connection", "keep-alive")
            self.send_header("X-Accel-Buffering", "no")
            self.end_headers()
            q = runner.subscribe()
            try:
                while True:
                    line = q.get()
                    try:
                        self.wfile.write(b"data: " + line.encode() + b"\n\n")
                        self.wfile.flush()
                    except (BrokenPipeError, ConnectionResetError):
                        return
            finally:
                runner.unsubscribe(q)

    return Handler


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--host", default="127.0.0.1")
    args = parser.parse_args()

    if not SIM_BIN.exists():
        sys.stderr.write(f"sim_bin not found at {SIM_BIN}. Run `make` first.\n")
        sys.exit(1)
    if not INDEX_HTML.exists():
        sys.stderr.write(f"index.html not found at {INDEX_HTML}.\n")
        sys.exit(1)

    runner = SimRunner(SIM_BIN)
    httpd = ThreadingHTTPServer((args.host, args.port), make_handler(runner))
    print(f"Actuator simulator running. Open http://{args.host}:{args.port}/",
          file=sys.stderr)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        try:
            runner.proc.terminate()
        except Exception:
            pass


if __name__ == "__main__":
    main()
