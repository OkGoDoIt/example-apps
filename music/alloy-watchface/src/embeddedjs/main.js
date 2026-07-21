import Poco from "commodetto/Poco";
import MusicService from "pebble/music";

const render = new Poco(screen);
const timeFont = new render.Font("Bitham-Bold", 42);
const labelFont = new render.Font("Gothic-Bold", 14);
const titleFont = new render.Font("Gothic-Bold", 24);
const detailFont = new render.Font("Gothic-Regular", 18);

const black = render.makeColor(0, 0, 0);
const white = render.makeColor(255, 255, 255);
const blue = render.makeColor(0, 170, 255);
const darkGray = render.makeColor(85, 85, 85);

let progressTimer;
let inFocus = true;

function fitText(text, font, maximumWidth) {
    if (render.getTextWidth(text, font) <= maximumWidth)
        return text;

    while (text.length && render.getTextWidth(text + "...", font) > maximumWidth)
        text = text.slice(0, -1);
    return text + "...";
}

function centeredText(text, font, color, y, maximumWidth) {
    text = fitText(text, font, maximumWidth);
    const width = render.getTextWidth(text, font);
    render.drawText(text, font, color, (render.width - width) / 2, y);
}

function playbackIsActive(playback) {
    return playback.state === "playing" || playback.state === "forwarding" ||
        playback.state === "rewinding";
}

function updateProgressTimer(active) {
    if (active && inFocus && progressTimer === undefined) {
        progressTimer = setInterval(() => draw(new Date()), 5000);
    } else if (!active && progressTimer !== undefined) {
        clearInterval(progressTimer);
        progressTimer = undefined;
    }
}

function draw(date) {
    const inset = render.width === render.height ? 20 : 8;
    const contentWidth = render.width - (2 * inset);
    const nowPlaying = music.nowPlaying();
    const playback = music.playbackInfo();
    const stateAvailable = playback.capabilities.playbackState;
    const showMedia = nowPlaying && (!stateAvailable || playbackIsActive(playback));

    render.begin();
    render.fillRectangle(black, 0, 0, render.width, render.height);

    let hours = date.getHours();
    if (watch.hour12) {
        hours %= 12;
        if (hours === 0)
            hours = 12;
    }
    const time = String(hours).padStart(2, "0") + ":" +
        String(date.getMinutes()).padStart(2, "0");
    centeredText(time, timeFont, white, 4, contentWidth);

    if (showMedia) {
        centeredText("NOW PLAYING", labelFont, blue, 62, contentWidth);
        centeredText(nowPlaying.title, titleFont, white, 78, contentWidth);
        centeredText(nowPlaying.artist, detailFont, white, 108, contentWidth);
        centeredText(nowPlaying.album, detailFont, white, 130, contentWidth);

        const progressY = render.height - 10;
        render.fillRectangle(darkGray, inset, progressY, contentWidth, 4);
        if (playback.capabilities.progress && playback.duration > 0) {
            const progress = Math.min(playback.position / playback.duration, 1);
            render.fillRectangle(blue, inset, progressY,
                Math.floor(contentWidth * progress), 4);
        }
    }

    render.end();
    updateProgressTimer(showMedia && playback.capabilities.progress &&
        playbackIsActive(playback));
}

const music = new MusicService({
    onChange() {
        if (inFocus)
            draw(new Date());
    }
});

watch.addEventListener("minutechange", event => {
    if (inFocus)
        draw(event.date);
});

watch.addEventListener("didFocus", focused => {
    inFocus = focused;
    if (focused) {
        draw(new Date());
    } else {
        updateProgressTimer(false);
    }
});

draw(new Date());
