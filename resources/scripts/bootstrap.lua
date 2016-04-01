ldoom = {}

local stuff = 0

function ldoom.tick()
    print(stuff)
end

function ldoom.update(dt)
    stuff = stuff + dt
end
