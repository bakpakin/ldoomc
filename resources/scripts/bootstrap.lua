ldoom = {}

function ldoom.load()
    snd = makesound("snd.ogg")
end

function ldoom.unload()
    deletesound(snd)
end

function ldoom.tick()
    print "hi."
    playsound(snd)
end
