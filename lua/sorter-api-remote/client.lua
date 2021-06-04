-- this is also bad insecure code -react
local modem = peripheral.find("modem")
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

function inventory()
    send("inventory")
    return recv()
end
function totals()
    send("totals")
    return recv()
end
function unmanageChest(id)
    send({umc = true, id=id})
end
function manageChest(id)
    send({mc=true, id=id})
end
function clearChest(id)
    send({cc=true, id=id})
end
function fillChest(id,item,count)
    send({fc=true, id=id,item=item,count=count})
end
