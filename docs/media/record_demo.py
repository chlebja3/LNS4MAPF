"""Record the README media (GIFs + screenshots) of the LNS4MAPF visualizer.

The solver GUI is driven on a virtual X display, so the figures can be regenerated on a
headless machine and without touching the developer's desktop:

    Xvfb  -> off-screen X server (1600x900)
    XTEST -> synthetic mouse clicks / scroll events that drive the ImGui widgets
    ffmpeg x11grab -> screen capture, converted to a looping GIF with a generated palette

Requirements: Xvfb, ffmpeg, ImageMagick, python-xlib, and a built ./build/simple_MAPF_solver
(see my_code/my_solver/scripts/compile.sh).

Usage:
    python3 docs/media/record_demo.py [clip ...]     # default: every clip
"""

import os
import shlex
import signal
import subprocess
import sys
import time

from Xlib import X, display
from Xlib.ext import xtest

DISPLAY = ":99"
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SOLVER_DIR = os.path.join(ROOT, "my_code", "my_solver")
BENCH = os.path.join(ROOT, "MAPF-benchmark")
OUT_DIR = os.path.join(ROOT, "docs", "media")
TMP_DIR = "/tmp/lns4mapf-media"

WIN_W, WIN_H = 1600, 900

MAP = os.path.join(BENCH, "mapf-map", "random-32-32-20.map")
SCEN = os.path.join(BENCH, "mapf-scen-random", "scen-random", "random-32-32-20-random-1.scen")
# The visualizer only receives LNS iterations when PSIPP is disabled (see LNS.cpp), so the
# demo instance is solved with plain SIPP + prioritized planning.
DEMO_ARGS = f"-m {MAP} -a {SCEN} -k 60 -s 1 -i 500 -t 10 --use_psipp false"
DEMO_ZOOM_OUT = 4  # scroll notches needed to fit a 32x32 map into the central dock node

BIG_MAP = os.path.join(BENCH, "mapf-map", "den312d.map")
BIG_SCEN = os.path.join(BENCH, "mapf-scen-random", "scen-random", "den312d-random-1.scen")
BIG_ARGS = (f"-m {BIG_MAP} -a {BIG_SCEN} -k 250 -s 0 -i 500 -t 10 --use_psipp false "
            f"--destroy_operator BLOCKED --sipp_implementation SIPP_suboptimal -w 1.2")
BIG_ZOOM_OUT = 6
BIG_PAN_DOWN = 45

# Widget positions in the 1600x900 window (the ImGui docking layout is deterministic).
# The side panels default to 20% of the window, which truncates the longer labels, so both
# clips first drag the dock splitter to widen the panel they show to 440 px.  The splitter is
# grabbed at the very edge of the SFML viewport (x = 320 / 1280) - one pixel further inside and
# the press would also start a map pan.
MAP_CENTER = (800, 380)
LEFT_SPLITTER = (318, 500)
RIGHT_SPLITTER = (1280, 500)
PANEL_WIDE = 440

SOL_PLAY = (1333, 139)
SOL_PAUSE = (1442, 139)
LNS_SLIDER_END = (1585, 194)
LNS_PLAY = (1333, 223)
LNS_STEP_FWD = (1478, 223)
HIGHLIGHT_DESTROYED = (1250, 399)
SIPP_PLAY = (174, 259)
SIPP_STEP_FWD = (318, 259)

# map + wide right panel, without the menu bar and the console
MAP_AND_RIGHT = "1160:690:440:30"
# wide left panel + map
LEFT_AND_MAP = "1180:690:0:30"
# just the 32x32 grid, zoomed out by DEMO_ZOOM_OUT notches
MAP_ONLY = "660:660:485:25"

# badges drawn onto the annotated overview screenshot: left panel, map, right panel, console
OVERVIEW_LABELS = [(410, 700), (1250, 70), (1560, 480), (1540, 790)]


# --------------------------------------------------------------------------- infrastructure
def start_xvfb() -> subprocess.Popen:
    subprocess.run(["pkill", "-f", f"Xvfb {DISPLAY}"], check=False)
    time.sleep(0.5)
    proc = subprocess.Popen(
        ["Xvfb", DISPLAY, "-screen", "0", f"{WIN_W}x{WIN_H}x24", "-nolisten", "tcp"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    time.sleep(1.5)
    return proc


def start_solver(args: str) -> subprocess.Popen:
    """Launch the GUI solver on the virtual display with software OpenGL."""
    env = dict(os.environ, DISPLAY=DISPLAY, LIBGL_ALWAYS_SOFTWARE="1")
    return subprocess.Popen(
        ["./build/simple_MAPF_solver"] + shlex.split(args),
        cwd=SOLVER_DIR,
        env=env,
        stdout=open(f"{TMP_DIR}/solver.log", "w"),
        stderr=subprocess.STDOUT,
    )


def wait_until_solved(timeout: float = 90.0) -> None:
    """Block until the computation thread reports the final cost."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        with open(f"{TMP_DIR}/solver.log") as fh:
            if "Final solution has sum of costs" in fh.read():
                time.sleep(1.0)
                return
        time.sleep(0.5)
    raise RuntimeError("solver did not finish in time")


def screenshot(path: str) -> None:
    subprocess.run(["import", "-window", "root", path],
                   env=dict(os.environ, DISPLAY=DISPLAY), check=True)


def cleanup(*procs) -> None:
    for proc in procs:
        if proc is None:
            continue
        proc.send_signal(signal.SIGTERM)
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()


# --------------------------------------------------------------------------- input & geometry
class Input:
    """Synthetic mouse input via the XTEST extension."""

    def __init__(self):
        self.display = display.Display(DISPLAY)
        self.root = self.display.screen().root

    def move(self, x: int, y: int) -> None:
        self.root.warp_pointer(x, y)
        self.display.sync()
        time.sleep(0.1)

    def click(self, x: int, y: int, hold: float = 0.08, after: float = 0.15) -> None:
        self.move(x, y)
        xtest.fake_input(self.display, X.ButtonPress, 1)
        self.display.sync()
        time.sleep(hold)
        xtest.fake_input(self.display, X.ButtonRelease, 1)
        self.display.sync()
        time.sleep(after)

    def drag(self, x0: int, y0: int, x1: int, y1: int, steps: int = 12) -> None:
        self.move(x0, y0)
        xtest.fake_input(self.display, X.ButtonPress, 1)
        self.display.sync()
        time.sleep(0.15)
        for i in range(1, steps + 1):
            self.root.warp_pointer(round(x0 + (x1 - x0) * i / steps), round(y0 + (y1 - y0) * i / steps))
            self.display.sync()
            time.sleep(0.03)
        time.sleep(0.15)
        xtest.fake_input(self.display, X.ButtonRelease, 1)
        self.display.sync()
        time.sleep(0.3)

    def scroll(self, x: int, y: int, notches: int) -> None:
        """Positive notches zoom in (button 4), negative zoom out (button 5)."""
        button = 4 if notches > 0 else 5
        self.move(x, y)
        for _ in range(abs(notches)):
            xtest.fake_input(self.display, X.ButtonPress, button)
            xtest.fake_input(self.display, X.ButtonRelease, button)
            self.display.sync()
            time.sleep(0.05)
        time.sleep(0.2)


class MapView:
    """Converts grid cells to screen pixels, mirroring Visualizer::adjust_sfml_part_viewport.

    The view spans the whole map (CELL_SIZE world units per cell) and is fitted into the
    central dock node so that one axis always overflows; the scroll wheel scales it by 1.1.
    """

    CELL = 20
    LEFT_PANEL = 0.2
    RIGHT_PANEL = 0.2
    BOTTOM_PANEL = 0.2

    def __init__(self, map_w: int, map_h: int, zoom_out: int = 0):
        sfml_w = WIN_W * (1 - self.LEFT_PANEL - self.RIGHT_PANEL)
        sfml_h = WIN_H * (1 - self.BOTTOM_PANEL)
        aspect = map_w / map_h
        if aspect < sfml_w / sfml_h:
            view_w = sfml_w
            view_h = view_w / aspect
        else:
            view_h = sfml_h
            view_w = view_h * aspect
        x0 = (self.LEFT_PANEL + (1 - view_w / sfml_w) / 2) * WIN_W
        y0 = (1 - view_h / sfml_h) / 2 * WIN_H
        self.scale = view_w / (map_w * self.CELL) / (1.1 ** zoom_out)
        self.center = (x0 + view_w / 2, y0 + view_h / 2)
        self.world_center = (map_w * self.CELL / 2, map_h * self.CELL / 2)

    def px(self, col: int, row: int) -> tuple:
        world_x = col * self.CELL + self.CELL / 2
        world_y = row * self.CELL + self.CELL / 2
        return (round(self.center[0] + (world_x - self.world_center[0]) * self.scale),
                round(self.center[1] + (world_y - self.world_center[1]) * self.scale))


def read_map_size(path: str) -> tuple:
    width = height = 0
    with open(path) as fh:
        for line in fh:
            if line.startswith("width"):
                width = int(line.split()[1])
            elif line.startswith("height"):
                height = int(line.split()[1])
            elif line.startswith("map"):
                break
    return width, height


def read_starts(path: str) -> list:
    starts = []
    with open(path) as fh:
        next(fh)  # version header
        for line in fh:
            parts = line.split()
            if len(parts) >= 8:
                starts.append((int(parts[4]), int(parts[5])))
    return starts


# --------------------------------------------------------------------------- capture
def record(out: str, duration: float, fps: int = 20) -> subprocess.Popen:
    cmd = [
        "ffmpeg", "-y", "-loglevel", "error",
        "-f", "x11grab", "-draw_mouse", "0",
        "-framerate", str(fps), "-video_size", f"{WIN_W}x{WIN_H}",
        "-i", DISPLAY, "-t", str(duration),
        "-c:v", "libx264", "-preset", "ultrafast", "-crf", "18", "-pix_fmt", "yuv420p",
        out,
    ]
    return subprocess.Popen(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)


def to_gif(src: str, out: str, crop: str = "", scale: int = 900, fps: int = 10,
           slow: float = 1.0, start: float = 0.0, duration: float = 0.0) -> None:
    """Convert a captured clip into a looping GIF with a per-clip 128 colour palette."""
    filters = []
    if crop:
        filters.append(f"crop={crop}")
    if slow != 1.0:
        filters.append(f"setpts={slow}*PTS")
    filters.append(f"fps={fps}")
    filters.append(f"scale={scale}:-1:flags=lanczos")
    chain = ",".join(filters)
    trim = ["-ss", str(start)] + (["-t", str(duration)] if duration else [])
    palette = f"{TMP_DIR}/palette.png"
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error"] + trim +
                   ["-i", src, "-vf", f"{chain},palettegen=max_colors=128", palette], check=True)
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error"] + trim +
                   ["-i", src, "-i", palette, "-lavfi",
                    f"{chain} [x]; [x][1:v] paletteuse=dither=bayer:bayer_scale=3", out], check=True)
    size = os.path.getsize(out) / 1e6
    print(f"  wrote {os.path.relpath(out, ROOT)} ({size:.1f} MB)", flush=True)


def annotate(path: str, labels: list) -> None:
    """Draw numbered badges onto a screenshot, in place."""
    args = ["convert", path, "-font", "DejaVu-Sans-Bold", "-pointsize", "24", "-strokewidth", "3"]
    for number, (x, y) in enumerate(labels, start=1):
        args += ["-fill", "#e8452c", "-stroke", "white", "-draw", f"circle {x},{y} {x + 19},{y}",
                 "-fill", "white", "-stroke", "none",
                 "-annotate", f"+{x - 7}+{y + 9}", str(number)]
    subprocess.run(args + [path], check=True)


# --------------------------------------------------------------------------- clips
def prepare(inp: Input, wide_panel: str) -> None:
    """Zoom out until the whole map is visible and widen the panel of interest."""
    inp.scroll(*MAP_CENTER, -DEMO_ZOOM_OUT)
    if wide_panel == "left":
        inp.drag(*LEFT_SPLITTER, LEFT_SPLITTER[0] + (PANEL_WIDE - 320), LEFT_SPLITTER[1])
    else:
        inp.drag(*RIGHT_SPLITTER, RIGHT_SPLITTER[0] - (PANEL_WIDE - 320), RIGHT_SPLITTER[1])


def clip_solution() -> None:
    """Playback of a solution: 60 agents crossing random-32-32-20 without colliding."""
    app = start_solver(DEMO_ARGS)
    wait_until_solved()
    inp = Input()
    # the panel is widened even though it is cropped away: the playback buttons are centred in
    # the panel, so their coordinates only match the measured ones at PANEL_WIDE
    prepare(inp, "right")
    inp.click(*LNS_SLIDER_END)  # jump to the last LNS iteration
    raw = f"{TMP_DIR}/solution.mp4"
    rec = record(raw, 4.0)
    time.sleep(0.8)  # let ffmpeg spin up before the animation starts
    inp.click(*SOL_PLAY)
    rec.wait()
    cleanup(app)
    # the grid alone: one GIF frame per simulated timestep
    to_gif(raw, f"{OUT_DIR}/solution.gif", crop=MAP_ONLY, scale=660, fps=10, slow=2.0)


def clip_sipp() -> None:
    """The low level search: SIPP expanding safe intervals for a single agent."""
    app = start_solver(DEMO_ARGS)
    wait_until_solved()
    view = MapView(*read_map_size(MAP), DEMO_ZOOM_OUT)
    starts = read_starts(SCEN)
    inp = Input()
    prepare(inp, "left")
    inp.click(*view.px(*starts[0]))  # select agent 0 by clicking its circle
    raw = f"{TMP_DIR}/sipp.mp4"
    rec = record(raw, 5.5)
    time.sleep(0.8)
    inp.click(*SIPP_PLAY)
    rec.wait()
    cleanup(app)
    to_gif(raw, f"{OUT_DIR}/sipp.gif", crop=LEFT_AND_MAP, scale=900, fps=10, slow=2.0)


def clip_lns() -> None:
    """The LNS loop: destroyed neighbourhoods highlighted, iteration by iteration."""
    app = start_solver(DEMO_ARGS)
    wait_until_solved()
    inp = Input()
    prepare(inp, "right")
    inp.click(*HIGHLIGHT_DESTROYED)  # before touching the slider: the list is still empty
    raw = f"{TMP_DIR}/lns.mp4"
    rec = record(raw, 9.5)
    time.sleep(0.8)
    for _ in range(12):
        inp.click(*LNS_STEP_FWD, hold=0.03, after=0.65)
    rec.wait()
    cleanup(app)
    to_gif(raw, f"{OUT_DIR}/lns.gif", crop=MAP_AND_RIGHT, scale=900, fps=5)


def shot_overview() -> None:
    """Full window with an agent selected, its SIPP search stepped a few nodes in."""
    app = start_solver(DEMO_ARGS)
    wait_until_solved()
    view = MapView(*read_map_size(MAP), DEMO_ZOOM_OUT)
    starts = read_starts(SCEN)
    inp = Input()
    prepare(inp, "left")
    inp.click(*view.px(*starts[0]))
    for _ in range(45):
        inp.click(*SIPP_STEP_FWD, hold=0.02, after=0.03)
    screenshot(f"{OUT_DIR}/gui_overview.png")
    cleanup(app)
    annotate(f"{OUT_DIR}/gui_overview.png", OVERVIEW_LABELS)
    print("  wrote docs/media/gui_overview.png", flush=True)


def shot_large_instance() -> None:
    """250 agents on den312d, solved with the bounded suboptimal search."""
    app = start_solver(BIG_ARGS)
    wait_until_solved()
    view = MapView(*read_map_size(BIG_MAP), BIG_ZOOM_OUT)
    starts = read_starts(BIG_SCEN)
    inp = Input()
    inp.scroll(*MAP_CENTER, -BIG_ZOOM_OUT)  # den312d is tall and overflows the viewport
    inp.drag(*RIGHT_SPLITTER, RIGHT_SPLITTER[0] - (PANEL_WIDE - 320), RIGHT_SPLITTER[1])
    inp.click(*view.px(*starts[0]))
    # the view is centred on the whole viewport, part of which hides behind the menu bar:
    # drag the map down (starting in the empty margin, so no agent is selected) to centre it
    inp.drag(400, 620, 400, 620 + BIG_PAN_DOWN)
    inp.click(*LNS_SLIDER_END)
    inp.click(*SOL_PLAY)
    time.sleep(3.5)
    inp.click(*SOL_PAUSE)
    screenshot(f"{OUT_DIR}/large_instance.png")
    cleanup(app)
    print("  wrote docs/media/large_instance.png", flush=True)


CLIPS = {
    "solution": clip_solution,
    "sipp": clip_sipp,
    "lns": clip_lns,
    "overview": shot_overview,
    "large": shot_large_instance,
}


def main(names: list) -> None:
    os.makedirs(TMP_DIR, exist_ok=True)
    os.makedirs(OUT_DIR, exist_ok=True)
    xvfb = start_xvfb()
    for name in names:
        print(f"recording {name} ...", flush=True)
        CLIPS[name]()
        time.sleep(1)
    cleanup(xvfb)


if __name__ == "__main__":
    main(sys.argv[1:] or list(CLIPS))
