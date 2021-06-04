-- this is bad, insecure code. - react
local modem = peripheral.wrap("right")
local channel = 1234
local key = 1234
modem.open(channel)
local function send(str)
    modem.transmit(channel,key,str)
end
local function recv()
    while true do
        local e,s,c,r,m = os.pullEvent("modem_message")
        if c == channel and r == key then
            return m
        end
    end
end

local id = 0
os.loadAPI("/apis/sorter.lua")
sorter.unmanageChest(id)

while true do
    local cmd = recv()
    if cmd == "inventory" then
        send(sorter.inventory())
    end
    if cmd == "totals" then
        send(sorter.totals())
    end
    if type(cmd) == "table" then
        if cmd.umc then
            sorter.unmanageChest(cmd.id)
            print("unmanage",cmd.id)
        elseif cmd.mc then
            sorter.manageChest(cmd.id)
            print("manage",cmd.id)
        elseif cmd.cc then
            sorter.clearChest(cmd.id)
            print("clear",cmd.id)
        elseif cmd.fc then
            sorter.fillChest(cmd.id,cmd.item,cmd.count)
            print("fill",cmd.id,cmd.item,cmd.count)
        end
    end
end
