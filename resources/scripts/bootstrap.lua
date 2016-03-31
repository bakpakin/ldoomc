ldoom = {}

local function receive_event(event, ...)
    if ldoom[event] then
        ldoom.event(...)
    end
end
