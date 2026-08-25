import Poco from "commodetto/Poco";
import Button from "pebble/button";
import PeekWidget from "pebble/peek";
import Vibes from "pebble/vibes";
import WakeUp from "pebble/wakeup";

// Tea steeping timer built around the Quick View widget API.
//
// Start a steep and leave the app: the timer publishes itself as a Quick View widget at the
// bottom of the watchface, a wakeup relaunches the app when the tea is ready, and the ready
// state stays peeked until the user acts on it. Pressing Down on the watchface (depending on
// the user's Settings > Quick View > Open With Down choice) launches this app with the
// widget's launch code in watch.launch.arguments.

const STEEP_MINUTES_MIN = 1;
const STEEP_MINUTES_MAX = 5;
const STEEP_MINUTES_DEFAULT = 3;

// watch.launch.reason value for a Quick View widget launch (APP_LAUNCH_PEEK_WIDGET)
const LAUNCH_REASON_PEEK_WIDGET = 8;

// Widget launch codes: watch.launch.arguments tells us which widget the user tapped through
const LAUNCH_CODE_STEEPING = 1;
const LAUNCH_CODE_READY = 2;

const render = new Poco(screen);
const titleFont = new render.Font("Gothic-Bold", 28);
const statusFont = new render.Font("Gothic-Regular", 24);
const hintFont = new render.Font("Gothic-Regular", 18);
const black = render.makeColor(0, 0, 0);
const white = render.makeColor(255, 255, 255);

let state = "idle"; // "idle" | "steeping" | "ready"
let steepMinutes = STEEP_MINUTES_DEFAULT;
let countdownTimer;

function centeredText(text, font, y) {
    const width = render.getTextWidth(text, font);
    render.drawText(text, font, black, (render.width - width) / 2, y);
}

function steepEnd() {
    return Number(localStorage.getItem("steepEnd") ?? 0);
}

function draw() {
    render.begin();
    render.fillRectangle(white, 0, 0, render.width, render.height);
    centeredText("Tea Timer", titleFont, 18);

    if (state === "idle") {
        centeredText(`Steep for ${steepMinutes} min`, statusFont, 64);
        centeredText("UP/DOWN adjust", hintFont, render.height - 54);
        centeredText("SELECT start", hintFont, render.height - 34);
    } else if (state === "steeping") {
        const remaining = Math.max(0, Math.round(steepEnd() - (Date.now() / 1000)));
        const seconds = String(remaining % 60).padStart(2, "0");
        centeredText(`${Math.floor(remaining / 60)}:${seconds} left`, statusFont, 64);
        centeredText("Leave the app!", hintFont, render.height - 54);
        centeredText("SELECT cancel", hintFont, render.height - 34);
    } else {
        centeredText("Enjoy your tea!", statusFont, 64);
        centeredText("SELECT reset", hintFont, render.height - 34);
    }
    render.end();
}

function setState(next) {
    state = next;
    if (state === "steeping" && countdownTimer === undefined) {
        countdownTimer = setInterval(() => {
            if (Date.now() / 1000 >= steepEnd())
                teaIsReady();
            else
                draw();
        }, 1000);
    } else if (state !== "steeping" && countdownTimer !== undefined) {
        clearInterval(countdownTimer);
        countdownTimer = undefined;
    }
    draw();
}

function formatReadyTime(end) {
    const date = new Date(end * 1000);
    let hours = date.getHours();
    if (watch.hour12) {
        hours %= 12;
        if (hours === 0)
            hours = 12;
    }
    return `Ready at ${hours}:${String(date.getMinutes()).padStart(2, "0")}`;
}

function startSteep() {
    const end = Math.round(Date.now() / 1000) + (steepMinutes * 60);
    const wakeupId = WakeUp.schedule(end * 1000, LAUNCH_CODE_READY, true);
    localStorage.setItem("steepEnd", String(end));
    localStorage.setItem("wakeupId", String(wakeupId));
    PeekWidget.publish({
        title: "Tea is steeping",
        subtitle: formatReadyTime(end),
        launchCode: LAUNCH_CODE_STEEPING,
        // If the wakeup somehow never fires, don't linger forever
        timeout: (steepMinutes * 60) + 60,
    });
    setState("steeping");
}

function teaIsReady() {
    localStorage.removeItem("steepEnd");
    localStorage.removeItem("wakeupId");
    Vibes.doublePulse();
    PeekWidget.publish({
        title: "Tea is ready!",
        subtitle: "Fish out that tea bag",
        launchCode: LAUNCH_CODE_READY,
        timeout: 0, // Stays up until the user acts on it
    });
    setState("ready");
}

function reset() {
    const wakeupId = localStorage.getItem("wakeupId");
    if (wakeupId !== null)
        WakeUp.cancel(Number(wakeupId));
    localStorage.removeItem("steepEnd");
    localStorage.removeItem("wakeupId");
    PeekWidget.withdraw();
    setState("idle");
}

new Button({
    types: ["select", "up", "down"],
    onPush(down, type) {
        if (!down)
            return;
        if (type === "select") {
            if (state === "idle")
                startSteep();
            else
                reset();
        } else if (state === "idle") {
            if (type === "up" && steepMinutes < STEEP_MINUTES_MAX)
                steepMinutes++;
            else if (type === "down" && steepMinutes > STEEP_MINUTES_MIN)
                steepMinutes--;
            draw();
        }
    }
});

// Resume a steep that is still running regardless of how we were launched
if (steepEnd() > Date.now() / 1000)
    state = "steeping";

if (watch.wake) {
    // The steep finished while we were closed
    teaIsReady();
} else if (watch.launch.reason === LAUNCH_REASON_PEEK_WIDGET &&
        watch.launch.arguments === LAUNCH_CODE_READY) {
    // Acting on "Tea is ready!" through the widget's button shortcut clears it
    PeekWidget.withdraw();
    localStorage.removeItem("steepEnd");
    localStorage.removeItem("wakeupId");
    state = "ready";
} else if (state !== "steeping" && steepEnd() !== 0) {
    // A steep ended while we were closed and the wakeup was missed
    teaIsReady();
}

setState(state);
