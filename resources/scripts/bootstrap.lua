function levent.load()
    myfont = ldoom.text.loadFont("hud.txt")
    mytext = myfont:newText("Hello, hello?\nHowdy?\n\n\nhi")
    mytext:setPosition(300, 300)
end

function levent.keyboard(key, action)
    if key == "escape" then
        ldoom.quit()
    end
    ldoom.console.logc(action)
end

function levent.draw()
    mytext:draw()
end
