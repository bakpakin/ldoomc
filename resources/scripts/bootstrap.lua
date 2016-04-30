function levent.load()
    snd = ldoom.audio.loadOgg("snd.ogg")
    ldoom.console.logc "$@F0FHi"
end

function levent.keyboard(key, action)
    if key == "q" then
        ldoom.quit()
    end
    ldoom.console.logc(string.byte(key), key, action)
    snd:play("hello.")
end
