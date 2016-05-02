function levent.load()
    myfont = ldoom.text.loadFont("hud.txt")
    mytext = myfont:newText[[When you use Meteor you ARE using Node.js.
    Biggie Smalls was here.
    $@F05Yo Hombre.]]
    mytext:setWidth(1800)
    fpstext = myfont:newText("fps: 0")
    fpstext:setPosition(400,500)
end

function levent.keyboard(key, action)
    if key == "escape" then
        ldoom.quit()
    end
    ldoom.console.logc(action)
end

function levent.tick()
    fpstext:setText(string.format("fps: %d", ldoom.getFPS()))
end

local t = 0

function levent.update(dt)
    t = t + dt
    local s = math.sin(0.5 * t)
    mytext:setThreshold(0.5 + 0.1 * s)
end

function levent.draw()
    fpstext:draw()
    mytext:draw()
end
