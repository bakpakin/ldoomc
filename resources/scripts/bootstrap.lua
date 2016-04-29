function ldoom.load()
    snd = ldoom.audio.loadOgg("snd.ogg")
    ldoom.console.log "$@F0FHi"
    ldoom.console.writec "Hi"
end

function ldoom.tick()
    snd:play()
end
