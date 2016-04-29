function ldoom.load()
    snd = ldoom.audio.loadSound("snd.ogg")
    print(snd)
end

function ldoom.tick()
    print(snd)
    snd:play()
end
