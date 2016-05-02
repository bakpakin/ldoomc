function levent.load()
    myfont = ldoom.text.loadFont("hud.txt")
    mytext = myfont:newText[[When you use Meteor you ARE using Node.js.\nkajsncka"
    akjbckasbckjasc
    aksbckjsac
    ajsca
$@F0 sahcsas
    cashjvcas
    caschjvsacascasjvcsac
    kasckjsabckjsac
    hjvsachjcsa]]
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
